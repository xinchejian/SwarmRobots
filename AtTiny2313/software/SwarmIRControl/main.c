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
*/
#define ROBOT_CMD_MASK  0x1F

//Ultimate aim is to put a groups for each robot in EEPROM, then use that to control group & individual behaviour
// This way code can be updated without overwriting group membership data!
// for now just KISS with a #define
#define MY_GROUPS 0x9  // groups 1 and 4.

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

#define IR_IN  PD6             // Pin used for IR RC5 decode - using seperate (duplicate) definition to above - so can easily change!
#define IR_PIN PIND            // port of pin above - used in IR decode function

#define FLASHLED
    #ifdef FLASHLED
    #define RX   PD0
    #define TX   PD1
#endif

#define M_LF PA0             // Motor Left Front
#define M_LB PA1             // Motor Left Back
#define M_RF PD2             // Motor Right Front
#define M_RB PD3             // Motor Right Back

#define M_REn PD5            // Motor Right Enable
#define M_LEn PB2            // Motor Left Enable



volatile uint8_t inttemp;
volatile uint8_t timerL;
volatile uint8_t timerH;


uint16_t detect(void);
void main_init(void);
void stopNow(void);
void forward(void);
void backward(void);
void turnLeft(void);
void turnRight(void);


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

  int timoutCounter = 0;
  int timeout = 50;            // how many counts to get before timeout is triggered.

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
        timoutCounter = 0;                  // reset timeOut counter - becasue comamdn has been recieved.
        rc5Id = (rc5Frame >> 6) & 0x1F;
        rc5Command = rc5Frame & 0x3F;
        rc5Toggle = (rc5Frame >> 11) & 0x01;

        // if received ID matches any group this robot belongs to, do the action specified!
        if ((rc5Id & MY_GROUPS) || (MY_GROUPS == 0)) {
            switch (rc5Command)
            {

     //TODO: use: ROBOT_CMD_MASK + change cmd range 0-0x1F AND update teh Arduino IR sending program!
                case 1:
                {
                    stopNow();
                    break;
                }
                case 2:
                {
                    forward();
                    break;
                }
                case 4:
                {
                    backward();
                    break;
                }
                case 8:
                {
                    turnLeft();
                    break;
                }
                case 16:
                {
                    turnRight();
                    break;
                }
                default:
                {
                    //stopNow();
                    break;
                }
            }
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
            stopNow();
            timoutCounter = 0;
        }
#ifdef FLASHLED
        PORTD &= ~(1 <<RX);          // turn LED off
#endif
    }
  }

  //stopNow();
  return 0;
}

void main_init(void)
{
  // clocked at CK @ 4MHZ
  TCCR0B = _BV(CS00);
  // enable over flow interrupt
  TIMSK = _BV(TOIE0);

    // initialise all the motor control pins
    DDRA |= (1 <<M_LF);         //Set as output
    DDRA |= (1 <<M_LB);         //Set as output
    DDRD |= (1 <<M_RF);         //Set as output
    DDRD |= (1 <<M_RB);         //Set as output

    DDRD |= (1 <<M_REn);        //Set as output
    DDRB |= (1 <<M_LEn);        //Set as output

    stopNow();                  //// all motor controller bits = 0 - ie off/disabled

#ifdef FLASHLED
    DDRD |= (1 <<RX);         //Set as output
    PORTD &= ~(1 <<RX);        // LED OFF
#endif
}

// Below here are all the motor control functions
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

      if (bit_is_clear(IR_PIN, IR_IN)) {
        goto detect1;
      }
    }

    while(bit_is_set(IR_PIN, IR_IN)) { // start1:
      if (timerH >= 8) { // no success in 131.072ms, time to give up
        goto fault;
      }
    }

    timerL = 0;
    while(bit_is_clear(IR_PIN, IR_IN)) { // start2:
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

    while(bit_is_set(IR_PIN, IR_IN)) { // start3:
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
      if (bit_is_set(IR_PIN, IR_IN)) {
        // store a 1
        rc5Frame |= 0x01;

        while(bit_is_set(IR_PIN, IR_IN)) {
          if (timerL > ref2) {
            goto fault;
          }
        }
      } else {
        // no action required to store a 0
        while(bit_is_clear(IR_PIN, IR_IN)) {
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
