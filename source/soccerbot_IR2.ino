#define MLF 11
#define MLB 12
#define MRF 7
#define MRB 13
#define LED 0
#define SPD_FAST 255
#define SPD_NORMAL 200
#define SPD_SLOW 100


#define IR_RECEIVER_BUFFER_SIZE 10
#define IR_RECEIVER_SIGNAL_MAX_LENGTH 6000
/*

In IR2, the comment out section, it is a piece of work in progress code for decoding of simple PWM signal, 
i decided to code my own as i feel all the current TV control protocols are having too low a bandwidth.

class IRreceiver {
public:
  IRreceiver(int pin){
    this->pin = pin;
    this->status=HIGH;
    this->t = micros();
  };
  
  void refresh(){
    boolean reading= digitalRead(this->pin);
    if(this->status != reading){
      int now = micros();
      int len = now - this->t;
      this->t = now;
      
      if(status == HIGH) {
        tHigh = len;
      }else {
        if(len < IR_RECEIVER_SIGNAL_MAX_LENGTH){
          buf[buf_pt++] = times(tHigh, len);
        }
      }
      status = reading;
    }
  };
  boolean hasByte(){};
  byte read(){};

private:
  int pin, unit, tHigh, t, buf_pt;
  boolean status;
  byte buf[IR_RECEIVER_BUFFER_SIZE];
  
  int times(int small, int big){
    if(small > big) return times(big, small);
    int margin = (int)(small * 0.1);
    int ratio = (small + margin) * 10) / big;
  }
};

IRreceiver ir1(4);
*/
void setup() {
  boolean i0 = digitalRead(7);
  boolean i1 = digitalRead(6);
  boolean i2 = digitalRead(5);
  boolean i3 = digitalRead(4);
  
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

void loop() {
  
  boolean i0 = digitalRead(0);
  boolean i1 = digitalRead(1);
  boolean i2 = digitalRead(2);
  boolean i3 = digitalRead(3);
  
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


