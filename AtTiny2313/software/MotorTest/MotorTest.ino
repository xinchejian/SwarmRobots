// SwarmRobot moves forward for about 10 seconds, stops, then backwards for 2 seconds
// this why it is easy to check if both motors work and go in the correct direction when building a SwarmRobot
// Tx LED is on for Forward direction, Rx LED on for reverse.
// For AtTiny2313 SwarmRobot - no shield required!
// by spanner888 usabledevices.com 2012-10 Licence:CCBYSA

#define MOTORS                      // comment out this define for testing without motors!

#define NOP __asm__ __volatile__ ("nop")
#define DELAY_CNT 11
#define DELAY_AVOID 65534   // uint16_t = 16-bit unsigned type, so 0-65535

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

#define IrLED 13

void setup() {

  // Set the Attiny pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);

  pinMode(LedRx, OUTPUT);
  pinMode(LedTx, OUTPUT);
}

void loop() {

  digitalWrite(LedRx, LOW);
  digitalWrite(LedTx, HIGH);

  forward(SPD_FAST);
  delay(10000);


  digitalWrite(LedTx, LOW);
  stopNow();
  delay(1000);


  digitalWrite(LedRx, HIGH);
  backward(SPD_FAST);
  delay(2000);

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

