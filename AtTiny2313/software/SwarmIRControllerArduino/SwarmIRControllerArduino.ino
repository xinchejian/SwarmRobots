/*
 * read number 0-9 from console & send as RC5 command
 *  
 * Based on:- IRremote: IRsendDemo http://arcfn.com
 * Remember IR library can send and recieve from MANY protocols!
 */

#include <IRremote.h>

IRsend irsend;

int inByte = 0;         // incoming serial byte
unsigned int IRData = 0;

void setup()
{
  // start serial port at 9600 bps:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
}

void loop() {
  // if we get a valid byte, read it
  if (Serial.available() > 0) {
    // get incoming byte:
    inByte = Serial.read();
    if (inByte > 47 && inByte < 58){
      inByte = inByte - 48;
      Serial.print(inByte);
      Serial.print(":- ");

      // send IR at least twice - else may not get received!
      for (int i=0;i<4;i++){ 
         // RC5 IR must start with leading '1', ie 0x800
         // Why inByte + 16 - no reason - cmd can be anything.
                        // sys    cmd
        IRData = 0x800 | 0x280 | (inByte + 16);  //0x10;
        Serial.println(IRData, HEX);
  
                // Remember IR library can send and recieve from MANY protocols!
        //      irsend.sendSony(0xa90, 12); // Sony TV power code
  //      irsend.sendRC5(0xc90, 12);  // c90 => sys=18, cmd=16
        irsend.sendRC5(IRData, 12);  
        delay(100);
      }
    }
  }
}

