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

    Contrat: leo.yan.cn@gmail.com
*/

/*assumption:
 * now based on Arduino UNO (ATMega 328)
 *
 */

#include "Arduino.h"
#include <avr/interrupt.h>
#include "SwarmRobot.h"
#include "Sensor.h"
#include "Messager.h"
#include "Action.h"
#include "Ledlight.h"

#if ( ROBOT_GROUP >= (0B1 << ROBOT_GROUP_BITS) ) || (ROBOT_INNERID >= ( 0B1 << ROBOT_INNERID_BITS))
#error
#endif

#if (ROBOT_ID_SELF > ROBOT_ID_SPECIAL_START)
#error
#endif


uint16_t g_BaseTimeFrequency;  //todo ??
uint16_t g_CntsOf100ms = 0;  //how many counts of frequence = 100ms
uint8_t g_TimeOn100ms = 0;   //time, uint 100ms


Ledlight_cls LedLight;

IR_Sensor IRSensor( ROBOT_ID_SELF );
BiMotor Wheels;

Messager_cls Messager( IRSensor, Wheels );
Action_cls Action( IRSensor, Wheels, Messager );


void setup() {

    
#if _DEBUG
    Serial.begin(9600);
    Serial.print("ID=");
    Serial.println( ROBOT_ID_SELF, HEX );
#endif
    /*To light all the LED*/
    LedLight.SetLed(LED_MODE_START);

    /*initialization of PWM, It is used for IR-LED */
    IRSensor.InitFrequence(40000);  //why? if it is set in the constructor of IRSensor, it will fail
    
#if !_DEBUG_SENSOR_NOTIMER
    TimerInit();
#endif

#if ( _START_TEST && !_DEBUG)
    uint8_t i;
    /**Test motor.  F B L R**/

    Wheels.Move(MT_FORWARD, 9);
    delay(2000);
    Wheels.Move(MT_BACKWARD, 9);
    delay(2000);    
    Wheels.Move(MT_CLOCKWISE, MT_DEGREE(90));
    delay(2000);
    Wheels.Move(MT_ANTICLOCK, MT_DEGREE(90));
    delay(2000);
    
    /**Test IR**/
    digitalWrite( LED_INFO_L , HIGH );
    digitalWrite( LED_INFO_R , HIGH );

    for (i = 0; i < 100; ++i)
    {
        delay(100);
        IRSensor.SendMessage( 0xEE, 0xAA, 0x88 );
    }

    digitalWrite( LED_INFO_L , LOW );
    digitalWrite( LED_INFO_R , LOW );

#endif


#if ( _START_REMOTE_CONTROL && !_DEBUG )
    uint8_t Cnt = 0;

    do
    {

        if ( ( LOW == digitalRead( IR_R_FL_PIN))
                || (LOW == digitalRead( IR_R_FR_PIN))
                || (LOW == digitalRead( IR_R_BL_PIN))
                || (LOW == digitalRead( IR_R_BR_PIN))
        )
        {
            Cnt++;
        }
        else
        {
            Cnt = 0;
        }

        delay(12);

    }while( Cnt < 2 );

#endif


    

}



void loop()
{

#if _DEBUG_MAIN
    DebugProcess();
#else
#if ROBOT_TYPE_ANT_LINEUP == ROBOT_INIT_TYPE
    Messager.MessageProc();
    delay(100);   
#endif

    Messager.MessageProc();
    Action.ActionProc();
#endif

    LedLight.QueryTimeProc(GetTimeOn100ms());

}


#if _DEBUG_MAIN
void DebugProcess()
{
    static uint8_t Cnt = 0;
    uint8_t i;

#if _DEBUG_SENSOR
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


    for (i = 1; i < MT_CLOCKRANDOM; ++i) {
        Serial.print("Move to: ");
        Serial.println(i);
        delay(500);
        
        Wheels.Move((MTMovDir_enum) i, 9);

        delay(1000);
    }
#endif



#if _DEBUG_MS
    uint8_t Info;
    uint8_t NearRobotID;
    
#if !_DEBUG_ACTION

    IRSensor.SendMessage( 0xFF, 0xaa, 0x55 );
    delay(100);
#endif
    Messager.MessageProc();

    if ( true == Messager.IsEnvInfoValid() )
    {
        Messager.GetEnvObstacleInfo( &Info );
        Serial.print("Obstacle: ");
        Serial.println(Info, HEX);

        Serial.print("NearR: ");
        for ( i = 0; i < POSITION_NUM; ++i )
        {
            NearRobotID = Messager.GetEnvNearRobot((TargetPos_enum)i);
            if ( ROBOT_ID_NONE != NearRobotID )
            {
                Serial.print(i);
                Serial.print("=");
                Serial.print(NearRobotID, HEX);
            }

        }
        Serial.println("");

        Messager.ShowEnvRobotInfo();

#if !_DEBUG_ACTION
        Messager.ClrEnvInfoLoc();
#endif
    }


#endif



#if _DEBUG_ACTION

#if !_DEBUG_MS
    Messager.MessageProc();
#endif
  
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
 *1, The frequency is based on the requirements of the IR-Sensor.
 *2, TimerProc can be called at this interrupt.  The TimerProc must be short.
 *3, It produces a pulse at the 200 milliseconds and 1 second. The width of the pulse is 100 ms
 */

void TimerInterruptHandler()
{
    /**Callback**/
    IRSensor.TimerProc();

    Wheels.TimerProc();
    
    /**Produce a pulse**/
    static uint16_t s_100msCnt = 0;

    s_100msCnt++;

    if ( s_100msCnt >= g_CntsOf100ms )
    {
        g_TimeOn100ms++;
        s_100msCnt = 0;
    }
}

//set interrupt vector
ISR(TIMER2_OVF_vect)
{
    TimerInterruptHandler();
}


void TimerInit( )
{
    static const uint16_t pscLst_alt[] = {0, 1, 8, 32, 64, 128, 256, 1024};
    uint32_t TimerFreq;
    //Initialize  frenquency Timer0

    g_TimeOn100ms = 0;
    TimerFreq = IRSensor.GetTimerFrequency();
    g_CntsOf100ms = TimerFreq/10;

    /**Set frequency of timer2**/
    //setting the waveform generation mode
    uint8_t wgm = 5;

    TCCR2A = (TCCR2A & B11111100) | (wgm & 3);
    TCCR2B = (TCCR2B & B11110111) | ((wgm & 12) << 1);

    //
    uint16_t multiplier = (F_CPU / (2 * TimerFreq * 255));

    byte iterate = 0;
    while(multiplier > pscLst_alt[++iterate]);
    multiplier = pscLst_alt[iterate];

    OCR2A = (F_CPU/(2* TimerFreq * multiplier));
    TCCR2B = (TCCR2B & ~7) | (iterate & 7);

    /**enable interrupt**/
    bitSet(TIMSK2, TOIE2);
    bitSet(TIFR2,TOV2);
}


uint8_t GetTimeOn100ms()
{
    return g_TimeOn100ms;
}




/*** Led control subsystem
 * Interface:
 *  - LedID,
 *  - Mode: ON OFF FADE FAST-FLASH  SLOW-FLASH
 *  - duration Time:
 *
 *
 * ***/


