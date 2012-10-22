#include "main.h"


// >>> Use of the serial works BUT
//      - speed is way off without crystal adn most terminals canto read
//      - using it INTERFERS WITH IO BIT BANGING SO THAT ir DECODING only works 1ST TIME, & MOTORS ONLY RUN FOR ~1/4 SECOND!
//#define SERIAL
#ifdef SERIAL
#include "serial.h"
#endif

#define IR_PIN PIND
#define IR_IN  PD6


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
  uint16_t frame;
  uint8_t toggle;
  uint8_t id;
  uint8_t command;

  main_init();
#ifdef SERIAL
  serial_init(9600);
#endif
  sei();
#ifdef SERIAL
  serial_puts("RC5 IR Decoder\r\n");
#endif
  while(1) {
    frame = detect();
    if (frame != 0xFFFF) {
    id = (frame >> 6) & 0x1F;
    command = frame & 0x3F;
    toggle = (frame >> 11) & 0x01;

    if (command == 16) {
        stopNow();
    }
    else if (command == 17) {
        forward();
    }
    else if (command == 18) {
        backward();
    }
    else if (command == 19) {
        turnLeft();
    }
    else if (command == 20) {
        turnRight();
    }
    //else stopNow();

#ifdef FLASHLED
//  bit/LED off &=0, on |=1
    PORTD |= (1 <<RX);          // turn LED ON
#endif

#ifdef SERIAL
      utoa(id, sys, 10);
      utoa(command, cmd, 10);

      serial_puts("sys: ");
      serial_puts(sys);
      serial_puts(" cmd: ");
      serial_puts(cmd);
      serial_puts(" toggle: ");
      serial_putc(toggle + '0');
      serial_puts("\r\n");
#endif
    }
#ifdef FLASHLED
    else {
        PORTD &= ~(1 <<RX);          // turn LED off
    }
#endif
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
  uint16_t frame;
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
    frame = 0;

    // sample the 12 data bits
    for(i=0;i<12;i++)
    {
      timerL = 0;

      while (timerL < ref1)
        ;

      frame <<= 1;
      if (bit_is_set(IR_PIN, IR_IN)) {
        // store a 1
        frame |= 0x01;

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
    // frame == 12 bits stored.
    return frame;
  fault:
    // return fault code
    return 0xFFFF;
}
