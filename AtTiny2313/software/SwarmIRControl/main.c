
//todo - also use these SAME #def's in the AtTiny receiver code!
// in the switch statement!!!!!
#define STOPCODE1 1        // button 1
#define STOPCODE2 0x36     // button stop

#define FWDCODE1  2        // button 2
#define FWDCODE2  0x35     // button play

#define REVCODE1  3        // button 3
#define REVCODE2  0x29     // button pause
#define REVCODE3  0x37     // button record

#define LFTCODE1  4        // button 4
#define LFTCODE2  0x32     // button fast forward

#define RGTCODE1  5        // button 5
#define RGTCODE2  0x34     // button fast rewind

#define SLTCODE1  0x3A     // button red
#define SRTCODE1  0x1C     // button blue

/*
            switch (Command)
            {

                case  STOPCODE1:
                case  STOPCODE2:
                {
                  break;
                }
                case  FWDCODE1:
                case  FWDCODE2:
                {
                  break;
                }
                case  REVCODE1:
                case  REVCODE2:
                case  REVCODE3:
                {
                  break;
                }
                case  LFTCODE1:
                case  LFTCODE2:
                {
                  break;
                }
                case  RGTCODE1:
                case  RGTCODE2:
                {
                  break;
                }
                case  SLTCODE1:
                case  SRTCODE1:
                {
                }
                default:
                {
                  break;
                }

*/




// todo - finish adding alternate Rx LED to read on - for physiacal layout - better reception - alternate wchich LED used

// after inactivy - spin a bit - in case can't receive IR at current position

// demo code timer/timeout not really workign!

//TODO - investigate if can decode MORE bits here - then can have more commands & groups (easy to add more in Arduino code)
//TODO - extend switch to handle all the cmd combinations



// *** This code MUST have AtTiny2313 running at **4MHz** so leave at factory default, or reset the fuse. ***
// code can probably be updated for 8MHz ......

#include "main.h"

/*
RC5 Address/ID/sys is 5 bits = 32, cmd is 6 bits = 64
    future:RC6 adds one more bit - but that also requires updated the IR decoding code.
Robot actions = #cmds (stop, f/b, 2xturn f/b, 2xspin f/b)=11 + allow for "more" actions - charge, follow me, .....

So start with:
    RC5
        - Address/ID/sys is 5 bits = 32 = 0x1F,     => SwarmRobot Group - each bit = 1 group - so a SwarmRobot can be in 0, 1 or many groups.
            0x0     not member of any group - ACT ON every COMMAND
            0x1     group 1 ACT on group 1 commands
            0x2     group 2
            0x4     group 3
            0x8     group 4
            0x10    group 5
            0x12    groups 5 & 2 ACT on group 2 & 5 commands
            ......

        - cmd is 6 bits = 64 = 0x3F                 => 0x0-0x1F = rc5Command
                                                    keep last bit spare for now - can use either for commands or for another group

    Note if need more than 32 SwarmRobots in a group, then just combine one or more groups - in the code.

above works well with custom Arduino transmitter

BUT - a commercial remote sends decimal numbers as command codes, not as per above!!!!
and more limited ability to set/choose group#


So implement group 0 + one #group, and specified cmd# in switch (cater for 2+ sets of arrow & play buttons <=> movement!!!!)



*/



//Ultimate aim is to put a groups for each robot in EEPROM, then use that to control group & individual behaviour
// This way code can be updated without overwriting group membership data!
// for now just KISS with a #define
#define MY_GROUP 2  // group 0 - execute ALL cmds!

#define MOTORS          // for debugging - comment out to turn off motors

// >>> Use of the serial works BUT
//      - speed is way off without crystal adn most terminals canto read
//      - using it INTERFERS WITH IO BIT BANGING SO THAT ir DECODING only works 1ST TIME, & MOTORS ONLY RUN FOR ~1/4 SECOND!
//#define SERIAL
#ifdef SERIAL
    #include "serial.h"
#endif

// These pins receive signals from the IR LEDs
#define IR_FC PD4              // IR LED Front Center
#define IR_R  PB0              // IR LED Rear
#define IR_FL PD6              // IR LED Front Left
#define IR_FR PB1              // IR LED Front Right



int irPIN[] = {6, 3};
#define IR_PIN PIND
int irPINIndex = 0;

//#define IR_IN  PD6             // Pin used for IR RC5 decode - using seperate (duplicate) definition to above - so can easily change!
//#define IR_PIN PIND            // port of pin above - used in IR decode function

/* All AtTiny2313 pins used for IR Rx

    *** take great care in the code!!!!- 1st two on portD, 2nd 2 on PortB

D6	8	PD4	P2 3	IR_FC
D8	11	PD6	P2 6	IR_FL

D9	12	PB0	P2 5	IR_R
D10	13	PB1	P2 4	IR_FR
*/



#define FLASHLED
#ifdef FLASHLED
    #define RX   PD0
    #define TX   PD1
#endif


// temp debug - do not use this AND FLASHLED!!!!!!
//#define CMD_FLASHLED
#ifdef CMD_FLASHLED
    #define RX   PD0
    #define TX   PD1
#endif




//Motor pin definitions
#define M_LF PA1             // Motor Left Front
#define M_LB PA0             // Motor Left Back
#define M_RF PD2             // Motor Right Front
#define M_RB PD3             // Motor Right Back

#define M_REn PD5            // Motor Right Enable
#define M_LEn PB2            // Motor Left Enable

#define ROBOT_CMD_MASK  0x3F
#define ROBOT_SYS_MASK  0x1F


volatile uint8_t inttemp;
volatile uint8_t timerL;
volatile uint8_t timerH;

uint16_t detect(void);
void main_init(void);
void actionLogic(int command);


#ifdef MOTORS
void stopNow(void);
void forward(void);
void backward(void);
void turnLeft(void);
void turnRight(void);
void spinLeft(void);
void spinRight(void);
#endif

//********************************************************************************
//********************************************************************************
int demoCode = 0;       // use to trigger demo mode.
#define DEMO 180        // just some magic number that means run demo mode
#define DEMO_DELAY_COUNT 2      // how long each action runs for
#define DEMO_ACTION_COUNT 13
int demoAction[] = {0x3A, 0x3A, 1, 0x1C, 0x1C, 1, 0x3A, 1, 0x1C, 1, 4, 1, 5, 1};
int demoActionIndex = 0;
int demoDelayCount = 0;
//********************************************************************************
//********************************************************************************

int main(void)
{
#ifdef SERIAL
  char sys[3];
  char cmd[3];
#endif
  uint16_t rc5Frame;
  uint8_t rc5Toggle;
  uint8_t rc5Id;
  uint8_t rc5Command;

    // these two depend upon the time in detect() attempting to read start of an IR code.
    // this is a variable time that is PROBABLY between 3.5 & 131mS
  int timoutCounter = 0;
  int timeout = 100;            // how many counts to get before timeout is triggered.

  main_init();
    #ifdef SERIAL
  serial_init(9600);
    #endif
  sei();
    #ifdef SERIAL
  serial_puts("RC5 IR Decoder\r\n");
    #endif
  while(1) {
    rc5Frame = detect();
    if (rc5Frame != 0xFFFF) {
        timoutCounter = 0;                  // reset timeOut counter - because command has been received.
        rc5Id = (rc5Frame >> 6) & ROBOT_SYS_MASK;
        rc5Command = rc5Frame & ROBOT_CMD_MASK;
        rc5Toggle = (rc5Frame >> 11) & 0x01;

        // if received ID matches any group this robot belongs to, do the action specified!
        if ((rc5Id == MY_GROUP) || (rc5Id == 0)) {
            actionLogic(rc5Command);
        }

    #ifdef FLASHLED
    //  bit/LED off &=0, on |=1
    PORTD |= (1 <<RX);          // turn LED ON
    #endif

    #ifdef SERIAL
      utoa(rc5Id, sys, 10);
      utoa(rc5Command, cmd, 10);

      serial_puts("sys: ");
      serial_puts(sys);
      serial_puts(" cmd: ");
      serial_puts(cmd);
      serial_puts(" rc5Toggle: ");
      serial_putc(rc5Toggle + '0');
      serial_puts("\r\n");
    #endif
    }
    else {
        //count how many times "tried" to decode & failed
        if (timoutCounter++ > timeout) {
            #ifdef MOTORS
            stopNow();
            #endif
            demoCode = 0;
            timoutCounter = 0;
        }
        #ifdef FLASHLED
        PORTD &= ~(1 <<RX);          // turn LED off
        #endif

        #ifdef CMD_FLASHLED
        PORTD &= ~(1 <<RX);          // turn LED off
        #endif

        if (demoCode > DEMO){
            // so run the demo action at current index
            if (demoAction[demoActionIndex] == 1)
                stopNow();  // because using actionLogic(1)  - exits demo mode!
            else
                actionLogic(demoAction[demoActionIndex]);
            //now update counter/index as required
            if (++demoDelayCount > DEMO_DELAY_COUNT) {
                //when delay times out - reset delay counter & increment for next action
                demoDelayCount =0;
                if (++demoActionIndex > DEMO_ACTION_COUNT) demoActionIndex = 0;
            }
        }
    }
  }

  //stopNow();
  return 0;
}


void actionLogic(int command){
            switch (command)
            {

                case 1:
                case 0x36:
                {
#ifdef MOTORS
                    stopNow();
#endif
                    demoCode =0;    // exit demo mode!
                    demoDelayCount = 0;
                    demoActionIndex = 0;
                    break;
                }
                case 2:
                case 0x35:
                {
#ifdef MOTORS
                    forward();
#endif
                    break;
                }
                case 3:
                case 0x29:
                case 0x37:
                {
#ifdef MOTORS
                    backward();
#endif
                    break;
                }
                case 4:
                case 0x32:
                {
#ifdef MOTORS
                    turnLeft();
#endif
                    break;
                }
                case 5:
                case 0x34:
                {
#ifdef MOTORS
                    turnRight();
#endif
                    break;
                }
                case 0x3A:
                {
#ifdef MOTORS
                    spinLeft();
#endif
                    break;
                }
                case 0x1C:
                {
#ifdef MOTORS
                    spinRight();
#endif
                    break;
                }


// here is code to trigger demo mode.
                case 9:
                {
                     demoCode +=9;
                   break;
                }
                default:
                {
#ifdef MOTORS
                    //stopNow();
#endif
                    break;
                }
            }
}


void main_init(void)
{
  // clocked at CK @ 4MHZ
  TCCR0B = _BV(CS00);
  // enable over flow interrupt
  TIMSK = _BV(TOIE0);

    #ifdef MOTORS
    // initialise all the motor control pins
    DDRA |= (1 <<M_LF);         //Set as output
    DDRA |= (1 <<M_LB);         //Set as output
    DDRD |= (1 <<M_RF);         //Set as output
    DDRD |= (1 <<M_RB);         //Set as output

    DDRD |= (1 <<M_REn);        //Set as output
    DDRB |= (1 <<M_LEn);        //Set as output

    stopNow();                  //// all motor controller bits = 0 - ie off/disabled
    #endif

    #ifdef FLASHLED
    DDRD |= (1 <<RX);         //Set as output
    PORTD &= ~(1 <<RX);        // LED OFF
    #endif

    #ifdef CMD_FLASHLED
    DDRD |= (1 <<RX);         //Set as output
    PORTD &= ~(1 <<RX);        // LED OFF
    #endif
}



// Below here are all the motor control functions
#ifdef MOTORS
void stopNow(){
  PORTA &= ~(1 <<M_LB);
  PORTA &= ~(1 <<M_LF);
  PORTD &= ~(1 <<M_RB);
  PORTD &= ~(1 <<M_RF);

  PORTD &= ~(1 <<M_REn);
  PORTB &= ~(1 <<M_LEn);
}

void forward(){
  PORTA &= ~(1 <<M_LB);
  PORTA |= (1 <<M_LF);
  PORTD &= ~(1 <<M_RB);
  PORTD |= (1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}

void backward(void){
  PORTA |= (1 <<M_LB);
  PORTA &= ~(1 <<M_LF);
  PORTD |= (1 <<M_RB);
  PORTD &= ~(1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}

void turnLeft(void){
  PORTA &= ~(1 <<M_LB);
  PORTA |= (1 <<M_LF);
  PORTD &= ~(1 <<M_RB);
  PORTD &= ~(1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}

void turnRight(void){
  PORTA &= ~(1 <<M_LB);
  PORTA &= ~(1 <<M_LF);
  PORTD &= ~(1 <<M_RB);
  PORTD |= (1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}

void spinLeft(void){
  PORTA &= ~(1 <<M_LB);
  PORTA |= (1 <<M_LF);
  PORTD |= (1 <<M_RB);
  PORTD &= ~(1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}

void spinRight(void){
  PORTA |= (1 <<M_LB);
  PORTA &= ~(1 <<M_LF);
  PORTD &= ~(1 <<M_RB);
  PORTD |= (1 <<M_RF);

  PORTD |= (1 <<M_REn);
  PORTB |= (1 <<M_LEn);
}


#endif



// Below here = IR decode
ISR(TIMER0_OVF_vect)
{
  timerL++;
  inttemp++;
  if (inttemp == 0) {
    timerH++;
  }
}

uint16_t detect(void)
{
  uint16_t rc5Frame;
  uint8_t i, temp, ref1, ref2;
  inttemp = 0;
  timerH = 0;

  detect1:
    timerL = 0;

    while(timerL < 55) { // idle high for 3.52ms
      if (timerH > 8) { // no success in 131.072ms, time to give up
        goto fault;
      }

      if (bit_is_clear(IR_PIN, irPIN[irPINIndex])) {
        goto detect1;
      }
    }

    while(bit_is_set(IR_PIN, irPIN[irPINIndex])) { // start1:
      if (timerH >= 8) { // no success in 131.072ms, time to give up
        goto fault;
      }
    }

    timerL = 0;
    while(bit_is_clear(IR_PIN, irPIN[irPINIndex])) { // start2:
      if (timerL >= 17) { // high for excess of 1088us
        goto fault;
      }
    }

    temp = timerL;
    timerL = 0;

    ref1 = temp;  // half bit time
    ref1 >>= 1;   // quarter bit time
    ref2 = ref1;
    ref1 += temp; // 3/4 bit time
    temp <<= 2;   // 1 bit time
    ref2 += temp; // 5/4 bit time

    while(bit_is_set(IR_PIN, irPIN[irPINIndex])) { // start3:
      if (timerL >= ref1) {
        goto fault;
      }
    }

    //timerL = 0;
    rc5Frame = 0;

    // sample the 12 data bits
    for(i=0;i<12;i++)
    {
      timerL = 0;

      while (timerL < ref1)
        ;

      rc5Frame <<= 1;
      if (bit_is_set(IR_PIN, irPIN[irPINIndex])) {
        // store a 1
        rc5Frame |= 0x01;

        while(bit_is_set(IR_PIN, irPIN[irPINIndex])) {
          if (timerL > ref2) {
            goto fault;
          }
        }
      } else {
        // no action required to store a 0
        while(bit_is_clear(IR_PIN, irPIN[irPINIndex])) {
           if (timerL > ref2) {
             goto fault;
           }
         }
      }
    }
    // rc5Frame == 12 bits stored.
    return rc5Frame;
  fault:
    // return fault code
    return 0xFFFF;
}
