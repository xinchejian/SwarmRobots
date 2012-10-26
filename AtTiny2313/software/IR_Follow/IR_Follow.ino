
// *** This code MUST have AtTiny2313 running at 8MHz so change from factory default of 4MHz!! ***
// this can be done in Arduino programmign GUI by Tools menu - BurnBootloader.


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

#define M_LF 2               // Motor Left Front
#define M_LB 3               // Motor Left Back
#define M_RF 4               // Motor Right Front
#define M_RB 5               // Motor Right Back

#define M_REn 7              // Motor Right Enable
#define M_LEn 11             // Motor Left Enable

#define T_L 16               // Left  touch sensor
#define T_F 15               // Front touch sensor
#define T_R 14               // Right touch sensor

#define SPD_FAST 255         // setting for PWM motor control Fast
#define SPD_NORMAL 200       // setting for PWM motor control Normal
#define SPD_SLOW 100         // setting for PWM motor control Slow

#define LedRx 0
#define LedTx 1

#define LF 0
#define RT 1

int curDir = LF;

void setup() {

  // Set the Attiny pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);

  pinMode(T_L, INPUT);     // Front Center IR LED input Attiny pin
  pinMode(T_F, INPUT);     // Front Left IR LED input Attiny pin
  pinMode(T_R, INPUT);     // Front Right IR LED input Attiny pin

  pinMode(LedRx, OUTPUT);
  pinMode(LedTx, OUTPUT);

}



void loop() {

  digitalWrite(LedRx, HIGH);
  digitalWrite(LedTx, LOW);
  delay(100);
  digitalWrite(LedRx, LOW);
  digitalWrite(LedTx, HIGH);
  delay(100);

  // Read all the IR LED outputs
  int l = digitalRead(T_L);
  int f = digitalRead(T_F);
  int r = digitalRead(T_R);

  if(l == HIGH && r == LOW){
    softRight();
    curDir = LF;
  }else if(r == HIGH && l == LOW){
    softLeft();
    curDir = RT;
  }else if(f == HIGH){
    backward(SPD_FAST);
    delay(500);
    if(curDir == LF){
      turnRight(SPD_FAST);
      delay(300);
    }else {
      turnLeft(SPD_FAST);
      delay(300);
    }
  }else if(curDir == LF){
    softLeft();
  }else {
    softRight();
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

void softLeft(){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);
  analogWrite(M_REn, SPD_FAST);    // PWM pin
  analogWrite(M_LEn, SPD_NORMAL);    // PWM pin
}

void softRight(){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);
  analogWrite(M_REn, SPD_NORMAL);    // PWM pin
  analogWrite(M_LEn, SPD_FAST);    // PWM pin
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

