//#define MOTORS           // comment out this define for testing without motors!

#define RECEIVED_IR_THRESHOLD 5     // Transmitter sends 15 pulses, this count used to decide
                                    // how many of these pulses received = object found!

#define NOP __asm__ __volatile__ ("nop")
#define DELAY_CNT 11

#define M_LF 2               // Motor Left Front
#define M_LB 3               // Motor Left Back
#define M_RF 4               // Motor Right Front
#define M_RB 5               // Motor Right Back

#define M_REn 7              // Motor Right Enable
#define M_LEn 11             // Motor Left Enable

#define IRR_L 9  //CON P2-5  PB0  Left  touch sensor
#define IRR_F 6  //CON P2-3  PD4  Front touch sensor
#define IRR_R 10 //CON P2-4  PB1  Right touch sensor
#define IRR_B 8  //CON P2-6  PD6  Right touch sensor

#define SPD_FAST 255         // setting for PWM motor control Fast
#define SPD_NORMAL 200       // setting for PWM motor control Normal
#define SPD_SLOW 100         // setting for PWM motor control Slow

#define LedRx 0
#define LedTx 1

#define IrLED 13

#define LF 0
#define RT 1

//int curDir = LF;

void setup() {

  DDRB = DDRB | (1 << PB4);     // Set IR LED transmit pin as output Arduino D13, AtTiny chip pin16.

/*
//trying to initialise IR Tx LED off
  pinMode(PB4, OUTPUT);
  digitalWrite(PB4, LOW);
*/

  // Set the Attiny pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);


/*
//testing to try and set as input, then read to get rid of startup issue - did not make any difference
  pinMode(IRR_L, INPUT);     // Front Center IR LED input Attiny pin
  pinMode(IRR_F, INPUT);     // Front Left   IR LED input Attiny pin
  pinMode(IRR_R, INPUT);     // Frontd Right  IR LED input Attiny pin
  pinMode(IRR_B, INPUT);     // Back  Center IR LED input Attiny pin

  digitalRead(IRR_L);
  digitalRead(IRR_F);
  digitalRead(IRR_R);
  digitalRead(IRR_B);

  digitalWrite(IRR_L, HIGH);    //Enable internal AtTiny Pull-Up resistor
  digitalWrite(IRR_F, HIGH);    //Enable internal AtTiny Pull-Up resistor
  digitalWrite(IRR_R, HIGH);    //Enable internal AtTiny Pull-Up resistor
  digitalWrite(IRR_B, HIGH);    //Enable internal AtTiny Pull-Up resistor
*/

  pinMode(LedRx, OUTPUT);
  pinMode(LedTx, OUTPUT);



//this code does not seem to make a difference to issues- but leave here for the  moment!
// it actually gives very NICE indication of startup - and has already found unexpected reboots!!!!
  for (byte i = 0;i <4; i++){
    // testing a power on delay to see if helps with start up issues
    digitalWrite(LedRx, HIGH);
    digitalWrite(LedTx, LOW);
    delay_us(30000);          // ~0.3 seconds
    digitalWrite(LedRx, LOW);
    digitalWrite(LedTx, HIGH);
    delay_us(30000);          // ~0.3 seconds
  }
  digitalWrite(LedRx, HIGH);    // turn off Rx LED, now both are off


/*
// read & display **actual** pin value with OLS or even DVM!
// looking to see if external pin value is low, or it is something internal/in code!
// Outcome was that code & logic analyser both agree
//      - IR receiver LEDs DO output signal - even when NO IR received!

// REMEMBER - is normaly high, goes low on signal.
// so here LOW = LED off, but later code reverses this so LED off no signal, on signal received!
// test - read values & display - just do TWO, then delay 4 seconds, then signal again
    uint8_t l = 0, r = 0, f = 0, b = 0;
    uint8_t reading = (PINB & ( _BV(PB0) | _BV(PB1))) | (PIND & (_BV(PD4) | _BV(PD6)));// & PIND;
//    if(!(reading & _BV(PB0))) digitalWrite(LedTx, LOW);
//    if(!(reading & _BV(PB1))) digitalWrite(LedRx, LOW);
    if(!(reading & _BV(PD4))) digitalWrite(LedTx, LOW);
    if(!(reading & _BV(PD6))) digitalWrite(LedRx, LOW);

    for (byte i=0; i<20; i++){
        delay_us(50000);          // ~0.5 seconds
    }

*/


}

void loop() {
// Simulate RC5 protocol - See www.vishay.com/docs/80071/dataform.pdf
//      Step 1. transmit 38kHz carrier for {900uS on, 900uS off} - 15 times
//      Step 2. delay 90ms
//      Step 3. repeat


  // Step 1. transmit 38kHz carrier for {900uS on, 900uS off} - 15 times
    uint8_t l = 0, r = 0, f = 0, b = 0;
  for(uint8_t i = 0; i < 15; i ++){
    uint8_t reading = pulse(90, 90);        // Send IR carrier frequency and TRY to receive after bouncing off objects.
    // increment counter for each IR receiver LED that detected signal
    if(!(reading & _BV(PB0))) l++;
    if(!(reading & _BV(PB1))) r++;
    if(!(reading & _BV(PD4))) f++;
    if(!(reading & _BV(PD6))) b++;
  }


#ifdef MOTORS
  // Simple logic. If object in front - GoForwards, if no object left ....
  if(f < RECEIVED_IR_THRESHOLD) {
    forward(SPD_FAST);
  }else if(l < RECEIVED_IR_THRESHOLD) {
    turnLeft(SPD_FAST);
  }else if(r < RECEIVED_IR_THRESHOLD) {
    turnRight(SPD_FAST);
  }else if(b < RECEIVED_IR_THRESHOLD) {
    backward(SPD_FAST);
  }else {
    stopNow();
  }
#endif

  // Step2. *** The delays below are also critical for IR receivers to work! (1000 + 8000) = 9,000 x 10 = 90,000 uS = 90mS!***
  // Show a short light for left and right
  if(l > RECEIVED_IR_THRESHOLD) digitalWrite(LedRx, HIGH); else digitalWrite(LedRx, LOW);
  if(r > RECEIVED_IR_THRESHOLD) digitalWrite(LedTx, HIGH); else digitalWrite(LedTx, LOW);
  delay_us(1000);
  // Show a long light for front and back
  if(f > RECEIVED_IR_THRESHOLD) digitalWrite(LedRx, HIGH); else digitalWrite(LedRx, LOW);
  if(b > RECEIVED_IR_THRESHOLD) digitalWrite(LedTx, HIGH); else digitalWrite(LedTx, LOW);
  delay_us(8000);
  stopNow();

}



//delay: {10 microSeconds x us}
void delay_us(uint16_t us) {
  uint8_t timer;
  while (us != 0) {
    // for 8MHz we want to delay 80 cycles per 10 microseconds
    // this code is tweaked to give about that amount.
    for (timer=0; timer <= DELAY_CNT; timer++) {
      NOP;
      NOP;
    }
    NOP;
    us--;
  }
}

// Pulse function turns on IR transmitter LED 38kHz carrier signal for specified time
// reads the decoded IR singal received by the IR LEDs
// Turns off IR transmitter LED 38kHz carrier signal AND delays for the specified off time
// returns the decoded IR singal received by the IR LEDs
uint8_t pulse(uint16_t ontime, uint16_t offtime){

  // Initialise & turn on the 38kHz carrier signal
  TCNT1 = 0; // Clear timer counter
  TIFR = 0;  // Clear timer flags
  OCR1A = 105;

  TCCR1A = (1 << COM1A0) | (1 << COM1B0);   // Toggle OC1A at TOP
  TCCR1B = (1 << WGM12) | (1 << CS10);      // CTC mode 4, Prescaler = 1

  // leave 38kHz carrier signal ON for this time period
  delay_us(ontime);

  // Now read the DECODED IR signal from all the IR receivers
  uint8_t reading = (PINB & ( _BV(PB0) | _BV(PB1))) | (PIND & (_BV(PD4) | _BV(PD6)));// & PIND;

  //??? Turn off the 38kHz IR carrier signal
  TCCR1A = 0;
  TCCR1B = 0;

  PORTB &= ~_BV(PB4);
  // keep carrier off for this time to match the specifications!
  delay_us(offtime);

  return reading;
}
/*
It seems there are a number of different types of IR receiver suited for different purposes. And the ones I've got seems to be only for TV remote control protocols.

www.vishay.com/docs/80071/dataform.pdf

Which means it has to have a way lower signal rate, so I simuated a RC-5 protocol signal, and all worked like charm.

http://en.wikipedia.org/wiki/RC-5#Protocol_Details

15  900us burst followed by a 90ms pause.


15  900us burst  (13.5ms)
  for(uint8_t i = 0; i < 15; i ++){
    uint8_t reading = pulse(90, 90);

90ms pause = delays in the LED flashes!!!!!

(1000 + 8000) = 9,000 x 10 = 90,000 uS = 90mS!


Vishay data formats.pdf
RC 5 standard uses a bi-phase coding (see figure 4) the carrier frequency fixed at 36 kHz <<<<<<<!!!!!!!!!!!!!!!!

The “Bi Phase Coding” has one rising or falling edge in the centre of each time slot (figure 1)
  ..rising = 1, falling edge = 0





Tx - 38kHz pulse for ON period, then 38kHz off period,
  no protocol/data/bits added to carrier at the moment
  but the Rx LED imposes RC5 timing restriction on the carrier on/off
    15  900us burst followed by a 90ms pause.
    38kHz = 26.316×10⁻⁶ period, so in 900uS, 38.5 cycles.

Rx LEDs detect the 38kHz carrier and output 1/0 for decoded bits found
*/



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
