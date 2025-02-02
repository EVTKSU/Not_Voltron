// system libraries
#include <Arduino.h>
#include <Metro.h>
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

#include <cstdint>

// #include <microros_task.h>
// #include <superloop_task.h>
// #include <TeensyThreads.h>

// evt libraries
#include <HAL.h>
#include <ODriveTeensyCAN.h>
#include <SBUS.h>
#include <shared_data.h>
#include <util.h>
#include <vesc_can.h>

#include "StateMachine.h"
#include "evt_time.hpp"
#include "subsystems/Controller.h"
#include "subsystems/Drivetrain.h"
#include "subsystems/EStop.h"

/*
Status:
Vesc RPM
Odrive Revolutions
Odrive Torque
Timestamps

Control:
Vesc RPM
Odrive Revolutions
Odrive Torque
*/
// structs have been changed, still need to change shared messages

struct vcu_status { // struct for vcu status msg type
  int state = 0;
  float commanded_current = 0.0f;
  float commanded_drive_rpm = 0.0f;
  float commanded_steer = 0.0f;
  float reported_current = 0.0f;
  float reported_steer = 0.0f;
  float reported_rpm = 0.0f;
  float drive_batt_voltage = 0.0f; //
  float steer_batt_voltage = 0.0f; //
  evt_time_t timestamp;
};

struct vcu_control {
  float drive_current = 0.0f;
  float drive_rpm = 0.0f;
  float steering_revs = 0.0f;
};

// NativeEthernet init
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
const IPAddress vcu_ip(10, 0, 0, 3);
const unsigned int vcu_port = 8888;
const unsigned int vcu_time_port = 8889;  // port to be used for NTP message
const IPAddress pc_ip(10, 0, 0, 2);
const unsigned int pc_port = 7777;
const unsigned int pc_time_port = 123;

EthernetUDP udp;
EthernetUDP time_udp;
// char receive_buffer[64];

// state_machine init (previously in superloop_task)
SBUS x8r(Serial8);
ODriveTeensyCAN odrive;

Controller rc(&x8r);
Drivetrain drivetrain(&odrive);
EStop estop;

StateMachine state_machine(&drivetrain, &rc, &estop);

bool NTP_packet_received = false;
const int NTP_PACKET_SIZE = 48;
char ntp_buf[NTP_PACKET_SIZE];

Metro status_metro = Metro(10);
vcu_control control;
vcu_status status;

uint32_t last_ntp_request_time = 0;
void send_ntp_request() {
  char buf[NTP_PACKET_SIZE];

  // set all bytes in the buffer to 0
  memset(buf, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  buf[0] = 0b11100011;  // LI, Version, Mode
  buf[1] = 0;           // Stratum, or type of clock
  buf[2] = 6;           // Polling Interval
  buf[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  buf[12] = 49;
  buf[13] = 0x4E;
  buf[14] = 49;
  buf[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  time_udp.beginPacket(pc_ip, pc_time_port);  // NTP requests are to port 123
  time_udp.write(buf, NTP_PACKET_SIZE);
  time_udp.endPacket();
  Serial.println("NTP Packet Sent");
}

uint32_t last_on_pps_millis = 0;
constexpr uint32_t PPS_LED = 3;
constexpr uint32_t TIME_SYNCED_LED = 4;
void on_pps() {
  digitalWrite(PPS_LED, HIGH);
  last_on_pps_millis = millis();

  evt_pps_round();
}

bool pps_inited = false;
void init_pps(int32_t secs, uint32_t microsecs) {
  pinMode(PPS_LED, OUTPUT);
  digitalWrite(PPS_LED, LOW);

  evt_time_t t;
  t.seconds = secs;
  t.microseconds = microsecs;
  evt_set_time(t);
  pinMode(14, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(14), on_pps, RISING);
  pps_inited = true;
}

void setup() {
  Serial.begin(9600);
  delay(3000);
  //while (!Serial) {
  //}

  Serial.println("Serial Begun");

  

  // ethernet init
  Ethernet.begin(mac, vcu_ip);
  

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println(
        "Ethernet shield was not found.  Sorry, can't run without hardware. "
        ":(");
  } else {
    
    Serial.println("Ethernet Hardware Found");
    
  }

  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  } else {
    
    Serial.println(Ethernet.localIP());
    
  }

  udp.begin(vcu_port);
  time_udp.begin(vcu_time_port);

  // end ethernet init
  // end pps init

  // trying to deprecate threads
  //  threads.addThread(microros_task, NULL, 10000);
  //  threads.addThread(superloop_task, NULL, 10000);

  // state_machine init
  state_machine.init();
}

void loop() {
  uint32_t start_loop_micros = micros();

  // delay(1000);
  // evt_time_t test_timestamp = evt_time_now();
  // Serial.printf("Time: %d ; %d\n", test_timestamp.seconds,
  // test_timestamp.microseconds);

  if (!NTP_packet_received) {
    if (micros() - last_ntp_request_time > 10'000'000) {
      ;
      send_ntp_request();
      last_ntp_request_time = micros();
    }

    size_t time_packet_len = time_udp.parsePacket();

    if (time_packet_len == NTP_PACKET_SIZE) {
      time_udp.read((unsigned char*)&ntp_buf, NTP_PACKET_SIZE);

      uint32_t secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (uint32_t)ntp_buf[40] << 24;
      secsSince1900 |= (uint32_t)ntp_buf[41] << 16;
      secsSince1900 |= (uint32_t)ntp_buf[42] << 8;
      secsSince1900 |= (uint32_t)ntp_buf[43];

      const unsigned long seventyYears = 2208988800UL;
      int32_t unix_secs = (int32_t)(secsSince1900 - seventyYears);

      uint32_t fractional_secs;
      fractional_secs = (uint32_t)ntp_buf[44] << 24;
      fractional_secs |= (uint32_t)ntp_buf[45] << 16;
      fractional_secs |= (uint32_t)ntp_buf[46] << 8;
      fractional_secs |= (uint32_t)ntp_buf[47];
      float microsf =
          ((float)fractional_secs / (float)UINT32_MAX) * 1'000'000.0;
      uint32_t microsecs =
          (uint32_t)microsf + (micros() - last_ntp_request_time);

      NTP_packet_received = true;

      Serial.printf("NTP Packet received\n");
      // Serial.printf("secsSince1900: %lu\n", secsSince1900);
      // Serial.printf("unix_secs: %d\n", unix_secs);
      // Serial.printf("microsf: %f\n", microsf);
      // Serial.printf("microsecs: %lu\n", microsecs);

      init_pps(unix_secs, microsecs);
    }
  }

  size_t packet_len = udp.parsePacket();
  if (packet_len == sizeof(vcu_control)) {
    udp.read((unsigned char*)&control, packet_len);
    shared_last_msg_recv_time = millis();
  }

  shared_input_rpm = control.drive_rpm;
  shared_input_steer = control.steering_revs;

  state_machine.loop();

  if (status_metro.check() && pps_inited) {
    status.commanded_current = shared_data_commanded_current;
    status.commanded_drive_rpm = shared_data_commanded_rpm;
    status.commanded_steer = shared_data_commanded_steer;
    status.reported_current = shared_data_reported_current;
    status.reported_steer = shared_data_reported_steer;
    status.reported_rpm = shared_data_reported_rpm;
    status.steer_batt_voltage = shared_data_steer_batt_voltage; //
    status.drive_batt_voltage = shared_data_drive_batt_voltage; //
    status.timestamp = evt_time_now();

    //int begin_err = udp.beginPacket(pc_ip, pc_port);
    //size_t len_written = udp.write((uint8_t*)&status, sizeof(status));
    //int end_err = udp.endPacket();
    udp.beginPacket(pc_ip, pc_port);
    udp.write((uint8_t*)&status, sizeof(status));
    udp.endPacket();
    // Serial.println("Packet Sent");
  }

  if (millis() - last_on_pps_millis > 100) {
    digitalWrite(PPS_LED, LOW);
  }

  shared_last_loop_micros = micros() - start_loop_micros;
}
