// SwarmRobot moves in random direction for random time (6 seconds max)
// For AtTiny2313 SwarmRobot - no shield required!
// by spanner888 usabledevices.com 2012-10 Licence:CCBYSA


// *** This code MUST have AtTiny2313 running at 8MHz so change from factory default of 4MHz!! ***
// this can be done in Arduino programmign GUI by Tools menu - BurnBootloader.


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
    uint16_t dly = random(6)*10000;

    digitalWrite(LedRx, LOW);
    digitalWrite(LedTx, HIGH);
    delay_us(dly);
    delay_us(dly);

#ifdef MOTORS

// just stuff about randomly pick an action & do it for random time
  switch (random(5)) {
    case 1:
      backward(SPD_FAST);
      break;
    case 2:
      turnRight(SPD_FAST);
      break;
    case 3:
      turnLeft(SPD_FAST);
      break;
    case 4:
      forward(SPD_FAST);
      break;
    default:
      stopNow();
  }
#endif

    digitalWrite(LedRx, HIGH);
    digitalWrite(LedTx, LOW);
    delay_us(dly);
    delay_us(dly);
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
