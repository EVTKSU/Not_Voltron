#include "microros_task.h"

#include <Arduino.h>
#include <TeensyThreads.h>
#include <geometry_msgs/msg/point32.h>
#include <geometry_msgs/msg/quaternion.h>
#include <micro_ros_platformio.h>
#include <rcl/error_handling.h>
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rclc_parameter/rclc_parameter.h>
#include <rmw_microros/custom_transport.h>
#include <rmw_microros/rmw_microros.h>
#include <rmw_microros/timing.h>
#include <shared_data.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <util.h>
#include <voltron_msgs/msg/voltron_state.h>

#include "geometry_msgs/msg/detail/point32__struct.h"
#include "rcl/subscription.h"
#include "rclc/init.h"

#define RCCHECK(fn)                                                    \
  {                                                                    \
    rcl_ret_t temp_rc = fn;                                            \
    if ((temp_rc != RCL_RET_OK)) {                                     \
      Serial.printf("micro-ROS error: %s returned %d at %s:%d\n", #fn, \
                    (int)temp_rc, __FILE__, __LINE__);                 \
      return false;                                                    \
    }                                                                  \
  }
#define EXECUTE_EVERY_N_MS(MS, X)      \
  do {                                 \
    static volatile int64_t init = -1; \
    if (init == -1) {                  \
      init = uxr_millis();             \
    }                                  \
    if (uxr_millis() - init > MS) {    \
      X;                               \
      init = uxr_millis();             \
    }                                  \
  } while (0)

// Custom Transport Functions
bool usb2_transport_open(struct uxrCustomTransport* /*transport*/) {
  SerialUSB1.begin(115200);
  return true;
}

bool usb2_transport_close(struct uxrCustomTransport* /*transport*/) {
  SerialUSB1.end();
  return true;
}

size_t usb2_transport_write(struct uxrCustomTransport* /*transport*/,
                            const uint8_t* buf, size_t len,
                            uint8_t* /*errcode*/) {
  size_t sent = SerialUSB1.write(buf, len);
  return sent;
}

size_t usb2_transport_read(struct uxrCustomTransport* /*transport*/,
                           uint8_t* buf, size_t len, int timeout,
                           uint8_t* /*errcode*/) {
  SerialUSB1.setTimeout(timeout);
  return SerialUSB1.readBytes((char*)buf, len);
}

rclc_support_t support;
rcl_node_t node;
rcl_timer_t pub_timer;
rclc_executor_t executor;
rcl_allocator_t allocator;
rcl_subscription_t control_subscription;
rcl_subscription_t tuning_subscription;
rcl_publisher_t status_publisher;

const char* kp_param_name = "Kp";
const char* rolling_decel_param_name = "rolling_deceleration";

// voltron_vcu_interfaces__msg__VcuControlInput control_msg;
// voltron_vcu_interfaces__msg__VcuStatus status_msg;
geometry_msgs__msg__Point32 control_msg;
geometry_msgs__msg__Point32 tuning_msg;
voltron_msgs__msg__VoltronState state_msg;

enum states {
  WAITING_AGENT,
  AGENT_AVAILABLE,
  AGENT_CONNECTED,
  AGENT_DISCONNECTED
} state;

void control_sub_callback(const void* msg_in) {
  const geometry_msgs__msg__Point32* msg =
      (const geometry_msgs__msg__Point32*)msg_in;
  shared_input_speed = msg->x;
  shared_input_steer = msg->y;
  uint32_t cur_time = millis();
  shared_last_msg_recv_time = cur_time;
}

void tuning_sub_callback(const void* msg_in) {
  const geometry_msgs__msg__Point32* msg =
      (const geometry_msgs__msg__Point32*)msg_in;
  shared_input_kp = msg->x;
  shared_input_rolling_decel = msg->y;
}

void pub_timer_callback(rcl_timer_t* timer, int64_t /*last_call_time*/) {
  if (timer != NULL) {
    // status_msg.state.data = (char*)shared_data_state;
    // status_msg.state.size = strlen((const char*)shared_data_state);
    // status_msg.measured_speed = shared_data_speed;
    // status_msg.measured_accel_estimate = shared_data_measured_accel_estimate;
    // status_msg.commanded_accel_estimate =
    // shared_data_commanded_accel_estimate;
    state_msg.header.stamp.sec = rmw_uros_epoch_millis() / 1000;
    state_msg.header.stamp.nanosec = rmw_uros_epoch_nanos();

    state_msg.steer = shared_current_steer;
    state_msg.throttle = shared_current_rpm;
    state_msg.brake_torque = shared_data_brake_torque;
    (void)rcl_publish(&status_publisher, &state_msg, NULL);
  }
}

// Functions create_entities and destroy_entities can take several seconds.
// In order to reduce this rebuild the library with
// - RMW_UXRCE_ENTITY_CREATION_DESTROY_TIMEOUT=0
// - UCLIENT_MAX_SESSION_CONNECTION_ATTEMPTS=3

bool create_entities() {
  Serial.println("(microROS) Creating...");
  allocator = rcl_get_default_allocator();

  // create support
  rcl_init_options_t init_options = rcl_get_zero_initialized_init_options();
  RCCHECK(rcl_init_options_init(&init_options, allocator));
  RCCHECK(rcl_init_options_set_domain_id(&init_options, 18));
  RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options,
                                         &allocator));
  RCCHECK(rcl_init_options_fini(&init_options));

  // create node
  Serial.println("(microROS) Creating node...");
  RCCHECK(rclc_node_init_default(&node, "vcu_node", "", &support));

  // create subscriber
  Serial.println("(microROS) Creating subscriber...");
  RCCHECK(rclc_subscription_init_best_effort(
      &control_subscription, &node,
      // ROSIDL_GET_MSG_TYPE_SUPPORT(voltron_vcu_interfaces, msg,
      // VcuControlInput),
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Point32),
      "/voltron_vcu/control_in"));

  RCCHECK(rclc_subscription_init_best_effort(
      &tuning_subscription, &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Point32),
      "/voltron_vcu/tuning_in"));

  // create publisher
  Serial.println("(microROS) Creating publisher...");
  RCCHECK(rclc_publisher_init_best_effort(
      &status_publisher, &node,
      // ROSIDL_GET_MSG_TYPE_SUPPORT(voltron_vcu_interfaces, msg,
      // VcuControlInput),
      ROSIDL_GET_MSG_TYPE_SUPPORT(voltron_msgs, msg, VoltronState),
      "/voltron_vcu/status"));

  // create timer,
  Serial.println("(microROS) Creating timer...");
  const unsigned int pub_timer_timeout = 1;
  RCCHECK(rclc_timer_init_default(&pub_timer, &support,
                                  RCL_MS_TO_NS(pub_timer_timeout),
                                  pub_timer_callback));

  // create param server
  Serial.println("(microROS) Creating param server...");

  // create executor
  Serial.println("(microROS) Creating executor...");
  executor = rclc_executor_get_zero_initialized_executor();
  GH;
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));
  GH;
  RCCHECK(rclc_executor_add_subscription(&executor, &control_subscription,
                                         (void*)&control_msg,
                                         control_sub_callback, ON_NEW_DATA));
  GH;
  RCCHECK(rclc_executor_add_subscription(&executor, &tuning_subscription,
                                         (void*)&tuning_msg,
                                         tuning_sub_callback, ON_NEW_DATA));
  GH;
  RCCHECK(rclc_executor_add_timer(&executor, &pub_timer));
  GH;
  // RCCHECK(rclc_executor_add_parameter_server(&executor, &param_server,
  // param_change_callback));
  //  GH;

  RCCHECK(rmw_uros_sync_session(100));
  GH;

  return true;
}

void destroy_entities() {
  Serial.println("(microROS) Destroying...");
  GH;
  rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
  GH;
  rmw_ret_t rc =
      rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);
  Serial.printf("AAAAA: %d\n", rc);
  GH;

  (void)rcl_subscription_fini(&control_subscription, &node);
  GH;
  (void)rcl_subscription_fini(&tuning_subscription, &node);
  GH;
  (void)rcl_publisher_fini(&status_publisher, &node);
  GH;
  (void)rcl_timer_fini(&pub_timer);
  GH;

  GH;
  (void)rclc_executor_fini(&executor);
  GH;
  (void)rcl_node_fini(&node);
  GH;
  (void)rclc_support_fini(&support);
  GH;
}

void microros_task(void* /*pvParameters*/) {
  rmw_uros_set_custom_transport(true, NULL, usb2_transport_open,
                                usb2_transport_close, usb2_transport_write,
                                usb2_transport_read);

  state = WAITING_AGENT;

  state_msg.header.frame_id.capacity = 100;
  state_msg.header.frame_id.data =
      (char*)malloc(state_msg.header.frame_id.capacity * sizeof(char));
  state_msg.header.frame_id.size = 0;

  // Assigning value to the frame_id char sequence
  strcpy(state_msg.header.frame_id.data, "voltron");
  state_msg.header.frame_id.size = strlen(state_msg.header.frame_id.data);

  while (true) {
    threads.delay(10);

    // Serial.printf("(microROS) State: %d\n", state);

    switch (state) {
      case WAITING_AGENT:
        EXECUTE_EVERY_N_MS(500,
                           state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1))
                                       ? AGENT_AVAILABLE
                                       : WAITING_AGENT;);
        break;
      case AGENT_AVAILABLE:
        state = (true == create_entities()) ? AGENT_CONNECTED : WAITING_AGENT;
        if (state == WAITING_AGENT) {
          destroy_entities();
        };
        break;
      case AGENT_CONNECTED:
        EXECUTE_EVERY_N_MS(200,
                           state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1))
                                       ? AGENT_CONNECTED
                                       : AGENT_DISCONNECTED;);
        if (state == AGENT_CONNECTED) {
          rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
        }
        break;
      case AGENT_DISCONNECTED:
        destroy_entities();
        state = WAITING_AGENT;
        break;
      default:
        break;
    }
  }
}
