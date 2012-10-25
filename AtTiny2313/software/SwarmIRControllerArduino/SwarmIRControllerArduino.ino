//TODO - add buttons for group number. currently group hard coded.


/*
 * read Switches/Joystick Group & command then send as RC5 IR
 *  
 * Based on:- IRremote: IRsendDemo http://arcfn.com
 *                 IR library can send/recieve MANY protocols!
 http://www.arduino.cc/en/Tutorial/Debounce
 */

#include <IRremote.h>

const int firstButton = 4; //First=Left, then Right, Up, Down, Center, ????
const int numButtons = 2;

IRsend irsend;               // initialise IR library
// IR LED pin 3 (defined in Library)

unsigned int IRData = 0;     // data to Transmit via IR LED
int Group = 8;               // Group of SwarmRobot

int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup()
{
  // set al the swtich pins as input
  for (int i=firstButton; i<(firstButton + numButtons); i++){
    pinMode(i, INPUT);
  }

  // start serial port at 9600 bps:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}



void loop() {
  //Read all the control/command switches
  int Command = 0;             // Command for SwarmRobot
  for (int i=firstButton; i<(firstButton + numButtons); i++){
    Command |= getButtton(i) << (i-firstButton);
  }

  // only send if a button is pushed!
  if (Command > 0){
    // send IR at least twice - else may not get received!
    for (int i=0;i<4;i++){ 
      // RC5 IR must start with leading '1', ie 0x800
      // sys(0-1F    cmd 0-1F {reserving 6th MSB for now}
      IRData = 0x800 | (Group << 6) | Command;  //0x10;
      Serial.println(IRData, HEX);

      // Remember IR library can send and recieve from MANY protocols!
      //      irsend.sendSony(0xa90, 12); // Sony TV power code
      //      irsend.sendRC5(0xc90, 12);  // c90 => sys=18, cmd=16
      //        2nd parameter = #bits being sent.
      irsend.sendRC5(IRData, 12);  
      delay(100);
    }
  }
}


int getButtton(int pinNumber){
  int lastState;
  int reading;
  lastState = digitalRead(pinNumber);
  do  {
    reading = digitalRead(pinNumber);
    if (reading != lastState) {
      lastDebounceTime = millis();
      lastState = reading;
    } 
  } 
  while ((millis() - lastDebounceTime) < debounceDelay);
  //  lastDebounceTime = millis();
  return reading;
}



