// digitalwrite
// constants won't change. They're used here to set pin numbers:


const int thIn = 5;    // throttle in pin
const int thOut = 6;    // throttle out pin
const int stIn = 7;    // steering in pin
const int stOut = 8;    // steering out pin
const int special = 9;    // special function 

// variables will change:
int specialState = 0;  // variable for reading the pushbutton status

void setup() {
  // initialize the pins as an output or input:
  pinMode(thIn, INPUT);
  pinMode(thOut, OUTPUT);
  pinMode(stIn, INPUT);
  pinMode(stOut, OUTPUT);
  pinMode(special, INPUT);
}

void loop() {

  // read the state of the special value:
  specialState = digitalRead(special);


  // check if the switch is pressed. If it is, the specialstate is HIGH:
  if (specialState == HIGH) {
    // figure 8 
    delay(1000);
    
  } else {
    // forwarding RC data
    digitalWrite(stOut, digitalRead(stIn));
    digitalWrite(thOut, digitalRead(thIn));
  }
}
