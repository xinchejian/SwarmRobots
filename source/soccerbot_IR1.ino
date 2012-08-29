//Spencer trying out new PCB 2012-08-18

// Todo - connect motors and test
//      - build & connect IR shield and test
//      - NO colour LEDs on IR shield, just use the RxTx LEDs to show left/right
//          and both on to show straight ahead. Ignore rear for now :)

// Don't forget to use Arduino pin numbers 
//    - NOT the physical chip pin numbers!
// See the spreadsheet in git for the full pin mapping

// Delay is about SIX times slower - ie delay(500); gives ~3 seconds instead of 0.5!
// - when assuming clock is 8MHz


#define IR_FC 766666              // IR LED Front Center
#define IR_FL 8              // IR LED Front Left
#define IR_R 9               // IR LED Rear
#define IR_FR 10             // IR LED Front Right

#define M_LF 2               // Motor Left Front
#define M_LB 3               // Motor Left Back
#define M_RF 4               // Motor Right Front
#define M_RB 5               // Motor Right Back

#define M_REn 7              // Motor Right Enable
#define M_LEn 11             // Motor Left Enable

#define SPD_FAST 255         // setting for PWM motor control Fast
#define SPD_NORMAL 200       // setting for PWM motor control Normal
#define SPD_SLOW 100         // setting for PWM motor control Slow

#define LedRx 0
#define LedTx 1

void setup() {

  // Set the Attiny pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);

  pinMode(IR_FC, INPUT);     // Front Center IR LED input Attiny pin
  pinMode(IR_FL, INPUT);     // Front Left IR LED input Attiny pin
  pinMode(IR_FR, INPUT);     // Front Right IR LED input Attiny pin
  pinMode(IR_R, INPUT);      // Rear IR LED input Attiny pin

  pinMode(LedRx, OUTPUT);
  pinMode(LedTx, OUTPUT);

  // Start moving forward
  forward(SPD_FAST);
  delay(1000);
}



void loop() {

  
  digitalWrite(LedTx, LOW);
  digitalWrite(LedRx, HIGH);
  delay(100);
  digitalWrite(LedTx, HIGH);
  digitalWrite(LedRx, LOW);
  delay(100);
  
  // Read all the IR LED outputs
  boolean i0 = digitalRead(IR_FC);
  boolean i1 = digitalRead(IR_FL);
  boolean i2 = digitalRead(IR_FR);
  boolean i3 = digitalRead(IR_R);

  // Now 'drive' towards the IR light source!
  if(i0 == LOW && (
      (i1 || i2 == LOW) ||
      (i1 && i2) == HIGH)) {
    forward(SPD_FAST);
    delay(100);
  }else if(i1 == LOW) {
    turnLeft(SPD_FAST);
    delay(100);
  }else if(i2 == LOW) {
    turnRight(SPD_FAST);
    delay(100);
  }else if(i3 == LOW) {
    backward(SPD_FAST);
    delay(100);
  }else {
    stopNow();
  }
}


// Below here are all the motor control functions
void stopNow(){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, 0);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, 0);
  
  analogWrite(M_REn, 0);    // PWM pin
  analogWrite(M_LEn, 0);    // PWM pin
}

void forward(int speed){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);

  analogWrite(M_REn, speed);    // PWM pin
  analogWrite(M_LEn, speed);    // PWM pin
}

void backward(int speed){
  digitalWrite(M_LF, 0);
  digitalWrite(M_LB, HIGH);
  digitalWrite(M_RF, 0);
  digitalWrite(M_RB, HIGH);

  analogWrite(M_REn, speed);    // PWM pin
  analogWrite(M_LEn, speed);    // PWM pin
}

void turnLeft(int speed){
  digitalWrite(M_LF, 0);
  digitalWrite(M_LB, HIGH);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);

  analogWrite(M_REn, speed);    // PWM pin
  analogWrite(M_LEn, speed);    // PWM pin
}

void turnRight(int speed){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  digitalWrite(M_RF, 0);
  digitalWrite(M_RB, HIGH);

  analogWrite(M_REn, speed);    // PWM pin
  analogWrite(M_LEn, speed);    // PWM pin
}
