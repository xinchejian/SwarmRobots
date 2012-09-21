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

int curDir = LF;

void setup() {

  // Set the Attiny pin modes, Note PWM pins do not require initialisation.
  pinMode(M_LF, OUTPUT);
  pinMode(M_LB, OUTPUT);
  pinMode(M_RF, OUTPUT);
  pinMode(M_RB, OUTPUT);

  pinMode(IRR_L, INPUT);     // Front Center IR LED input Attiny pin
  pinMode(IRR_F, INPUT);     // Front Left   IR LED input Attiny pin
  pinMode(IRR_R, INPUT);     // Front Right  IR LED input Attiny pin
  pinMode(IRR_B, INPUT);     // Back  Center IR LED input Attiny pin

  pinMode(LedRx, OUTPUT);
  pinMode(LedTx, OUTPUT);

  pinMode(IrLED, OUTPUT);


  // SETUP pin modes for te PWM pin
  DDRB = DDRB | (1 << PB4);
  
}

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

uint8_t pulse(uint16_t ontime, uint16_t offtime){
  
  TCNT1 = 0; // Clear timer counter
  TIFR = 0;  // Clear timer flags
  OCR1A = 105;

  TCCR1A = (1 << COM1A0) | (1 << COM1B0);   // Toggle OC1A at TOP
  TCCR1B = (1 << WGM12) | (1 << CS10);      // CTC mode 4, Prescaler = 1

  delay_us(ontime);

  uint8_t reading = (PINB & ( _BV(PB0) | _BV(PB1))) | (PIND & (_BV(PD4) | _BV(PD6)));// & PIND;  
//  uint8_t reading = PINB & _BV(PB0);
//  uint8_t reading = PINB & _BV(PB0);
  
  TCCR1A = 0;
  TCCR1B = 0;
  
  PORTB &= ~_BV(PB4);
  
  delay_us(ontime);
  return reading;
}


void loop() {
  uint8_t l = 0, r = 0, f = 0, b = 0;
  for(uint8_t i = 0; i < 15; i ++){
    uint8_t reading = pulse(90, 90);
    if(!(reading & _BV(PB0))) l++;
    if(!(reading & _BV(PB1))) r++;
    if(!(reading & _BV(PD4))) f++;
    if(!(reading & _BV(PD6))) b++;
  }
  // Show a short light for left and right
  if(l > 5) digitalWrite(LedRx, HIGH); else digitalWrite(LedRx, LOW);
  if(r > 5) digitalWrite(LedTx, HIGH); else digitalWrite(LedTx, LOW);
  delay_us(1000);
  // Show a long light for front and back
  if(f > 5) digitalWrite(LedRx, HIGH); else digitalWrite(LedRx, LOW);
  if(b > 5) digitalWrite(LedTx, HIGH); else digitalWrite(LedTx, LOW);
  delay_us(8000);
  stopNow();
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
