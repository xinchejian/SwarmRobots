/*
                          IR-Swarmbot Software

    Copyright (C) <2012>  <Leo Yan>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contract: leo.yan.cn@gmail.com
*/

/*assumption:
 * now it is based on Arduino UNO (ATMega 328)
 *
 */

#include "Arduino.h"
#include <avr/interrupt.h>
#include "PWM.h"
#include "SwarmRobot.h"
#include "Sensor.h"
#include "Messager.h"
#include "Action.h"

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif



IR_Sensor IRSensor( ROBOT_ID_SELF );
BiMotor Wheels;

Messager_cls Messager( IRSensor );
Action_cls Action( IRSensor, Wheels, Messager );


void setup() {
  
     /*initialization of PWM */

    Initialize_16();          //if init and set frequency in the IRSensor construct, it will not working normally.
    SetFrequency_16(38000);   //IR_CARRIER_FRENQUENCY
    _SFR_MEM16(OCR1B_MEM) = 0; //((uint32_t)90*ICR1)/255;
    _SFR_MEM16(OCR1A_MEM) = 0;

    TimerInit();

    pinMode(RUNING_LED_PIN, OUTPUT);

    delay(5000);   // the noise of motor is loud.  so you can update software before running

#if _DEBUG
    Serial.begin(9600);
#endif

}

void loop()
{

    
#if _DEBUG_MAIN
    DebugProcess();
#else
    Messager.MessageProc();
    Action.ActionProc();
#endif

}


#if _DEBUG_MAIN
void DebugProcess()
{
#if _DEBUG_SENSOR
    uint8_t Index,SenderID, MessageID, Para;
    uint8_t MessageState = false;

    IRSensor.SendMessage( 0xEE, 0x88, 0xab );
    IRSensor.SendData();
    Index = IR_POSITION_FL;
    IRSensor.ReceiveData(Index);

    (void)IRSensor.GetMessage( Index, &SenderID, &MessageID, &Para, &MessageState );
    delay(100);

#endif


#if _DEBUG_MT
    uint8_t i;

    for (i = 0; i <= MT_CLOCKRANDOM; ++i) {
        Serial.print("Move to: ");
        Serial.println(i);

        Wheels.Move((MTMovDir_enum) i, 1);

        delay(8000);
    }
#endif

#if _DEBUG_MS
    uint8_t i;
    uint8_t Index,SenderID, MessageID, Para;
    uint8_t State;
#if !_DEBUG_ACTION        
    IRSensor.SendMessage( 0xEE, 0x88, 0xa0 );

    Messager.MessageProc();


    for (i = 1; i < MS_CONDITIONTBL_NUM; ++i)
    {
        Messager.GetExteriorInfo( (ExteriorInfoID_enum)i, &State );
        Serial.print("ExInfo ");
        Serial.print(i);
        Serial.print(": S=");
        Serial.println(State);
    }

    delay(1000);
#endif
#endif

#if _DEBUG_ACTION

    Messager.MessageProc();
    Action.ActionProc();

    delay(500);

#endif


}
#endif

/* Thread of Timer
 *
 *
 */

void TimerInterruptHandler()
{
#if !_DEBUG_SENSOR  //can not use serial.print in interrupt
    IRSensor.TimerProc();
#endif
    Wheels.TimerProc();

#if _DEBUG_MT
 {
    static uint16_t Cnt = 0;
    static uint8_t Flag = LOW;
    
    if ( 2000 < Cnt )
    {
      Flag = !Flag;
      digitalWrite(RUNING_LED_PIN, Flag);
      Cnt = 0;
    }
    Cnt ++; 
 }
#endif
    
}

ISR(TIMER0_COMPA_vect)
{
    TimerInterruptHandler();
}

void TimerInit()
{
    //Initialize  frenquency Timer0
    uint32_t Val;

    Val = IRSensor.GetTimerFrequency();
    SetFrequency_8(0, Val);
    //set interrupt vector


    sbi(TIMSK0, OCIE0A);
    sbi(TIFR0,OCF0A);
}

