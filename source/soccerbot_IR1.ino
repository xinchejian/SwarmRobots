#define M_LF 11              // Motor Left Front - Attiny Pin 11
#define M_LB 12              // Motor Left Back - Attiny Pin 12
#define M_RF 7               // Motor Right Front - Attiny Pin 7
#define M_RB 13              // Motor Right Back - Attiny Pin 13
#define LED 0
#define SPD_FAST 255         // setting for PWM motor control Fast
#define SPD_NORMAL 200       // setting for PWM motor control Normal
#define SPD_SLOW 100         // setting for PWM motor control Slow

#define IR_FC 4              // IR LED Front Center - Attiny Pin 4
#define IR_FL 1              // IR LED Front Left - Attiny Pin 1
#define IR_R 3               // IR LED Rear - Attiny Pin 3
#define IR_FR 2              // IR LED Front Right - Attiny Pin 2



void setup() {

  // Set the Attiny pin modes
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(IR_FC, INPUT);     // Front Center IR LED input Attiny pin
  pinMode(IR_FL, INPUT);     // Front Left IR LED input Attiny pin
  pinMode(IR_FR, INPUT);     // Front Right IR LED input Attiny pin
  pinMode(IR_R, INPUT);      // Rear IR LED input Attiny pin

  // Start moving forward
  forward(SPD_FAST);
  delay(1000);
}



void loop() {

  // Read all the IR LED outputs
  boolean i0 = digitalRead(IR_FC);
  boolean i1 = digitalRead(IR_FL);
  boolean i2 = digitalRead(IR_FR);
  boolean i3 = digitalRead(IR_R);

  // Now 'drive' towards the IR light source!
  if(i0 == LOW && ((i1 || i2 == LOW) || (i1 && i2) == HIGH)) {
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
  analogWrite(M_LB, 0);
  analogWrite(M_LF, 0);
  analogWrite(M_RB, 0);
  analogWrite(M_RF, 0);
}

void forward(int speed){
  analogWrite(M_LB, 0);
  analogWrite(M_LF, speed);
  analogWrite(M_RB, 0);
  analogWrite(M_RF, speed);
}

void backward(int speed){
  analogWrite(M_LF, 0);
  analogWrite(M_LB, speed);
  analogWrite(M_RF, 0);
  analogWrite(M_RB, speed);
}

void turnLeft(int speed){
  analogWrite(M_LF, 0);
  analogWrite(M_LB, speed);
  analogWrite(M_RB, 0);
  analogWrite(M_RF, speed);
}

void turnRight(int speed){
  analogWrite(M_LB, 0);
  analogWrite(M_LF, speed);
  analogWrite(M_RF, 0);
  analogWrite(M_RB, speed);
}
