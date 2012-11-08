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

#if ( ROBOT_GROUP >= (0B1 << ROBOT_GROUP_BITS) ) || (ROBOT_INNERID >= ( 0B1 << ROBOT_INNERID_BITS))
#error
#endif

#if (ROBOT_ID_SELF == ROBOT_ID_OTHER) || (ROBOT_ID_SELF == ROBOT_ID_ANY)
#error
#endif

uint32_t g_BaseTimeFrequency;

IR_Sensor IRSensor( ROBOT_ID_SELF );
BiMotor Wheels;

Messager_cls Messager( IRSensor );
Action_cls Action( IRSensor, Wheels, Messager );


void setup() {

#if _DEBUG
    uint8_t j;
    Serial.begin(9600);

    Serial.print("ID=");
    Serial.println( ROBOT_ID_SELF, HEX );

     Serial.print("IR Pin: FL=");
     Serial.print(IR_R_FL_PIN);
     Serial.print(" FR=");
     Serial.print(IR_R_FR_PIN);
     Serial.print(" BL=");
     Serial.print(IR_R_BL_PIN);
     Serial.print(" BR=");
     Serial.println(IR_R_BR_PIN);

     Serial.print("Motor Pin: EN=");
     Serial.println(MOTOR_L_EN_PIN);

#endif
  
     /*initialization of PWM */

    Initialize_16();          //if init and set frequency in the IRSensor construct, it will not working normally.
    SetFrequency_16(38400);   //IR_CARRIER_FRENQUENCY
    _SFR_MEM16(OCR1B_MEM) = 0;



    pinMode(LED_RUNING_PIN, OUTPUT);

    uint8_t i;
    uint8_t LedPosition[]={LED_FL_PIN, LED_FR_PIN, LED_BL_PIN, LED_BR_PIN};

    Serial.println("HVER_PROMINI");

    for (i = 0; i < 4; ++i )
    {
        pinMode( LedPosition[i], OUTPUT );
        digitalWrite( LedPosition[i] , LOW );
        delay(200);
    }

    for (i = 0; i < 4; ++i )
    {
        digitalWrite( LedPosition[i] , HIGH );
        delay(200);
    }


#if !_DEBUG_SENSOR_NOTIMER
    TimerInit();
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
    static uint8_t Cnt = 0;

#if _DEBUG_SENSOR
    uint8_t Index;
    uint8_t Ret;
    IRPosition_enum IRLoc;
    IRMsgOutput_stru IRMsg; 
    
    

    IRSensor.SendMessage( 0xFF, 0xAA, 0xAA );

#if _DEBUG_SENSOR_NOTIMER
    IRSensor.TimerProc();
#endif

    IRSensor.PrintMsgQue();

    Ret = IRSensor.GetMessage( &IRLoc, &IRMsg );
    if ( SUCCESS == Ret )
    {
        Serial.print("Receive-");
        Serial.print(IRLoc);
        
        if ( IRLoc < IR_RECIEVER_NUM )
        {
            Serial.print(":S=0X");
            Serial.print(IRMsg.SenderID, HEX);
            Serial.print(" M=");
            Serial.print(IRMsg.MessageID);
            Serial.print(" P=");
            Serial.print(IRMsg.Para);
            Serial.print(" V=");
            Serial.print(IRMsg.Check);
        }
        
        Serial.println("");

    }

    delay(200);

#endif



#if _DEBUG_MT
    uint8_t i;

    for (i = 1; i < MT_CLOCKRANDOM; ++i) {
        Serial.print("Move to: ");
        Serial.println(i);
        delay(500);
        
        Wheels.Move((MTMovDir_enum) i, 1);

        delay(1000);
    }
#endif



#if _DEBUG_MS

#if !_DEBUG_ACTION
    uint8_t Info;
    IRSensor.SendMessage( 0xFF, 0xaa, 0x55 );

    Messager.MessageProc();

    if ( true == Messager.IsEnvInfoValid() )
    {
        Messager.GetEnvObstacleInfo( &Info );
        Serial.print("Obstacle: ");
        Serial.println(Info, HEX);

        Messager.ShowEnvRobotInfo();
        Messager.ClrEnvInfo();

    }

    delay(10);
#endif
#endif



#if _DEBUG_ACTION

    Messager.MessageProc();
    Action.ActionProc();

    //delay(30);

#endif

#if _DEBUG_TIMER
    Serial.print("STimer: MaxT=");
    Serial.print(gIRTimerTest.Period);
    Serial.print("Cnt=");
    Serial.print(gIRTimerTest.Cnt);
    Serial.print("Who=");
    Serial.println(gIRTimerTest.Who, BIN);
    gIRTimerTest.Period = 0;
#endif

}
#endif

/* Thread of Timer
 *
 *
 */

void TimerInterruptHandler()
{
    IRSensor.TimerProc();

    Wheels.TimerProc();

    Action.TimerProc();
    
    

#if _DEBUG
 {
    static uint16_t Cnt = 0;
    static uint8_t Flag = LOW;
    
    if ( 2000 < Cnt )
    {
      Flag = !Flag;
      digitalWrite(LED_RUNING_PIN, Flag);
      Cnt = 0;
    }
    Cnt ++; 
 }
#endif
    
}


ISR(TIMER2_OVF_vect)
{
    TimerInterruptHandler();
}


void TimerInit()
{
    //Initialize  frenquency Timer0
    Initialize_8( TIMER2_OFFSET );
    g_BaseTimeFrequency = IRSensor.GetTimerFrequency();
    SetFrequency_8(TIMER2_OFFSET, g_BaseTimeFrequency);

    //set interrupt vector

    bitSet(TIMSK2, TOIE2);
    bitSet(TIFR2,TOV2);
}

#if 0

ISR(TIMER0_COMPA_vect)
{
    TimerInterruptHandler();
}

void TimerInit()
{
    //Initialize  frenquency Timer0
    uint32_t Val;

    g_BaseTimeFrequency = IRSensor.GetTimerFrequency();
    SetFrequency_8(0, g_BaseTimeFrequency);
    //set interrupt vector

    bitSet(TIMSK0, OCIE0A);
    bitSet(TIFR0,OCF0A);
}
#endif

