/*
  Xinchejian Hackerspace Shanghai SwarmRobot http://wiki.xinchejian.com/wiki/Swarm_robots/

  Implements core motor driver functionality and simple InfraRed (IR) chasing.

  Licence CC by SA

  Sofware version              IR1.01  Date:- 2012-08-30

  Uses Hardware:-
    - Motor driver PCB version     Date:- 2012-08
    - IR shield version            Date:- 2012-08
*/

/*
  Release notes for this version:

  - Added Notes_Tips_ToDO files so the main code looks simpler and is not overwhelming!
  - Lots of comments added and things shuffled about to make things more readable
  - Added #define SPIN to allow SwarmBot to turn on the spot (one wheel goes forward, other backwards),
    else pivot on one wheel while other drives forwards
  - Discarded changes in previous version as motor drive logic was not working

*/


// *** This code MUST have AtTiny2313 running at 8MHz so change from factory default of 4MHz!! ***
// this can be done in Arduino programmign GUI by Tools menu - BurnBootloader.



//********************************************************************************************************
//Global IO Pin and variable definitions

// These pins flash to show serial data is being received (Rx) or transmitted (Tx)
// Also can be used to show IR signals received from left, right, front (both LED). No indication for reverse.
#define RxPin 0
#define TxPin 1
#define R_LED RxPin          // using less confusing name for IR signal flashing
#define L_LED TxPin          // using less confusing name for IR signal flashing

// These pins receive signals from the IR LEDs
#define IR_FC 6              // IR LED Front Center
#define IR_FL 8              // IR LED Front Left
#define IR_R 9               // IR LED Rear
#define IR_FR 10             // IR LED Front Right

//These pins are connected to the L293D motor controller
#define M_LF 2               // Motor Left Front
#define M_LB 3               // Motor Left Back
#define M_RF 4               // Motor Right Front
#define M_RB 5               // Motor Right Back
#define M_REn 7              // Motor Right ENABLE
#define M_LEn 11             // Motor Left ENABLE

#define PWM                  // use PWM motor speed control, else just full power!
#define SPD_FAST 255         // setting for PWM motor control Fast
#define SPD_NORMAL 200       // setting for PWM motor control Normal
#define SPD_SLOW 100         // setting for PWM motor control Slow

#define SPIN true            // Turn on the spot (one wheel goes forward, other backwards),
                             // else pivot on one wheel while other drives forwards

#define FLASH_ON_IR          // if this is defined, then flash the Rx &/or Tx LEDs to indicate direction of received IR.
                             // This way you can skip the four LEDs on the IR shield!
                             // no flashing for rear signal!
//********************************************************************************************************


void setup() {

// Set the Attiny IO pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);

  #ifndef PWM
  // just testing no PWM, L293 always enabled!
  pinMode(M_REn, OUTPUT);
  pinMode(M_LEn, OUTPUT);
  digitalWrite(M_REn, HIGH);    // Low is disabled
  digitalWrite(M_LEn, HIGH);    // Low is disabled
  #endif

  pinMode(IR_FC, INPUT);        // Front Center IR LED input Attiny pin
  pinMode(IR_FL, INPUT);        // Front Left IR LED input Attiny pin
  pinMode(IR_FR, INPUT);        // Front Right IR LED input Attiny pin
  pinMode(IR_R,  INPUT);        // Rear IR LED input Attiny pin

  pinMode(RxPin, OUTPUT);
  pinMode(TxPin, OUTPUT);

  stopNow();                    //make sure all pins initialised & both motors stopped

  //Moving forward for short time to show this SwarmBot is working OK!
  forward(SPD_FAST);
  delay(300);
}


void loop() {

  // Read all the IR LED outputs
  boolean IR_front = digitalRead(IR_FC);
  boolean IR_left = digitalRead(IR_FL);
  boolean IR_right = digitalRead(IR_FR);
  boolean IR_rear = digitalRead(IR_R);

  // Move the SwarmBot in direction of the 'best' IR signal
  if( (IR_front == LOW) && ( 0 == (IR_left ^ IR_right)) ) {
    forward(SPD_FAST);
    delay(100);
  }else if(IR_left == LOW) {
    turnLeft(SPD_FAST, SPIN);
    delay(100);
  }else if(IR_right == LOW) {
    turnRight(SPD_FAST, SPIN);
    delay(100);
  }else if(IR_rear == LOW) {
    backward(SPD_FAST);
    delay(100);
  }else {
    stopNow();
  }

  #ifdef FLASH_ON_IR
  //Turn both LEDs OFF - so they flash a bit - just for showing off!
  digitalWrite(R_LED, LOW);
  digitalWrite(L_LED, LOW);
  #endif
}
// **************** END of main loop program ****************



// *** Below here are all the motor control functions ***
void stopNow(){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, 0);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, 0);

  #ifdef PWM
  analogWrite(M_REn, 0);            // PWM pin
  analogWrite(M_LEn, 0);            // PWM pin
  #endif
}



void forward(int speed){
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);

  #ifdef PWM
  analogWrite(M_REn, speed);            // PWM pin
  analogWrite(M_LEn, speed);            // PWM pin
  #endif

  #ifdef FLASH_ON_IR
  //turn both LEDs on
  digitalWrite(R_LED, HIGH);
  digitalWrite(L_LED, HIGH);
  #endif
}



void backward(int speed){
  digitalWrite(M_LF, 0);
  digitalWrite(M_LB, HIGH);
  digitalWrite(M_RF, 0);
  digitalWrite(M_RB, HIGH);

  #ifdef PWM
  analogWrite(M_REn, speed);            // PWM pin
  analogWrite(M_LEn, speed);            // PWM pin
  #endif

  //NO LEDs on!!
}



void turnLeft(int speed, bool spin){
  if (spin){
    // left wheel in reverse
    digitalWrite(M_LF, 0);
    digitalWrite(M_LB, HIGH);
  }
  // Right wheel forwards
  digitalWrite(M_RB, 0);
  digitalWrite(M_RF, HIGH);

  #ifdef PWM
  analogWrite(M_REn, speed);            // PWM pin
  analogWrite(M_LEn, speed);            // PWM pin
  #endif

  #ifdef FLASH_ON_IR
  digitalWrite(L_LED, HIGH);            //turn Left LED on
  #endif
}



void turnRight(int speed, bool spin){
  // Left wheel forwards
  digitalWrite(M_LB, 0);
  digitalWrite(M_LF, HIGH);
  if (spin){
    // Right wheel in reverse
    digitalWrite(M_RF, 0);
    digitalWrite(M_RB, HIGH);
  }
  #ifdef PWM
  analogWrite(M_REn, speed);          // PWM pin
  analogWrite(M_LEn, speed);          // PWM pin
  #endif

  #ifdef FLASH_ON_IR
  digitalWrite(R_LED, HIGH);        //turn right LED on
  #endif
}

