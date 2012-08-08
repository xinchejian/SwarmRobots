#define MLF 11
#define MLB 12
#define MRF 7
#define MRB 13
#define LED 0
#define SPD_FAST 255
#define SPD_NORMAL 200
#define SPD_SLOW 100

void setup() {
  pinMode(MLF, OUTPUT);
  pinMode(MLB, OUTPUT);
  pinMode(MRF, OUTPUT);
  pinMode(MRB, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(4, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  forward(SPD_FAST);
  delay(1000);
}

void loop() {
  
  boolean i0 = digitalRead(4);
  boolean i1 = digitalRead(1);
  boolean i2 = digitalRead(2);
  boolean i3 = digitalRead(3);
  
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

void stopNow(){
  analogWrite(MLB, 0);
  analogWrite(MLF, 0);
  analogWrite(MRB, 0);
  analogWrite(MRF, 0);
}

void forward(int speed){
  analogWrite(MLB, 0);
  analogWrite(MLF, speed);
  analogWrite(MRB, 0);
  analogWrite(MRF, speed);
}

void backward(int speed){
  analogWrite(MLF, 0);
  analogWrite(MLB, speed);
  analogWrite(MRF, 0);
  analogWrite(MRB, speed);
}

void turnLeft(int speed){
  analogWrite(MLF, 0);
  analogWrite(MLB, speed);
  analogWrite(MRB, 0);
  analogWrite(MRF, speed);
}

void turnRight(int speed){
  analogWrite(MLB, 0);
  analogWrite(MLF, speed);
  analogWrite(MRF, 0);
  analogWrite(MRB, speed);
}
