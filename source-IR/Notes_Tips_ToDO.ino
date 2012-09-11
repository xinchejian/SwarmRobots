/*
Placing some notes, tips and ToDo in this file, so the main code looks simpler and is not overwhelming!



see xcj wiki.....
Especially for construction, how to setup the Attiny library adn how to program the Attiny.


??? DOXY GEN - to get html of code doc.....



If SwarmBot moves for a longer time than expected, check if the Attiny has had the fuses set correctly. 
See the section "Configuring the ATtiny to run at 8 MHz (for SoftwareSerial support)" at http://hlt.media.mit.edu/?p=1695 


//could save some memory by just reading in the if statements, 
//  or by using one byte to store all values - which could then be easier to use a switch to evaluate

// could add motor saving - eg stop before change f/reverse etc.

//do some tests on CURRENT required (for XCJ MOTOR!) and voltage range
// and how long battery lasts..


  NOTE:- Attiny cannot use all (IO hardware) Arduino commands!
  See http://hlt.media.mit.edu/?p=1695 
     download from http://code.google.com/p/arduino-tiny
  July 31 2012 release:- SoftwareSerial is now supported.  ===>>>>Neither the ATtiny2313 nor the ATtiny4313 are included.  
    Definitions are missing from the header files and the code is too big.
    So keep using older version (?????????????? DATE OF # ?????????????????????) or try to remove other AtTiny and add Attiny2313 back!


*/
