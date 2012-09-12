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

#include "PWM.h"
#include "Arduino.h"
#include "Motor.h"

/* Analysis of critical resource:
1.
critical resource:  SpeedVal - (type is uint8_t)

main thread:   SetSpeed()
timer thread:  TimerProc()->if RunCnt==0 ->Stop()

2.
critical resource:  RunCnt - (type is uint16_t)

main thread:   SetMoveDistance()
               SetRotateAngle()

timer thread:  TimerProc()-> if SpeedVal!= 0 ->Stop()

$ conclude:
1. when setting, RunCnt must be set before SpeedVal
2. when clearing, no constraint

 */



/*public*/
BiMotor::BiMotor( )

{
    MotorPin[MOTOR_INDEX_L].DriverA = MOTOR_L_DIRPINA;
    MotorPin[MOTOR_INDEX_L].DriverB = MOTOR_L_DIRPINB;
    MotorPin[MOTOR_INDEX_L].Enable = MOTOR_L_EN_PIN;
    MotorPin[MOTOR_INDEX_R].DriverA = MOTOR_R_DIRPINA;
    MotorPin[MOTOR_INDEX_R].DriverB = MOTOR_R_DIRPINB;
    MotorPin[MOTOR_INDEX_R].Enable = MOTOR_R_EN_PIN;
    //todo, pwm is initilized in SENSOR. it should be to apply the resource of PWM

    Stop();
    //pin initialization

    pinMode(MotorPin[MOTOR_INDEX_L].DriverA,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_L].DriverB,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_R].DriverA,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_R].DriverB,OUTPUT);

    pinMode(MotorPin[MOTOR_INDEX_L].Enable,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_R].Enable,OUTPUT);

}

bool BiMotor::isMoving()
{
    return ( (MT_SPEED_ZERO == SpeedVal)? false : true  );
}

void BiMotor::Move( MTMovDir_enum Direction, uint8_t Para )
{
    switch ( Direction )
    {
    case MT_FORWARD:
    case MT_BACKWARD:
        Walk( Direction, Para );
        break;

    case MT_CLOCKWISE:
    case MT_ANTICLOCK:
    case MT_CLOCKRANDOM:
        Rotate( Direction, Para );
        break;
    }

}

inline void BiMotor::Walk(
        MTMovDir_enum Direction,
        uint8_t Distance,         //the unit is 1 cm
        MTSpeed_enum Speed
        )
{
    MTSingleAct_enum Action;
    uint8_t i;
    //todo:check para
    if ( 0 == Distance )
    {
        return;
    }

    /*set direction*/
    switch(Direction)
    {
    case MT_FORWARD:
        Action = MT_ACTION_FORWORD;
        break;

    case MT_BACKWARD:
        Action = MT_ACTION_BACKWORD;
        break;

    default:
        return;

    }

    for (i = 0; i < MOTOR_NUM; ++i )
    {
        SetSingleMotorAction( i, Action);
    }

    /** RunCnt must be set before SpeedVal**/
    SetMoveDistance(Distance);
    SetSpeed(Speed);

    Run();
}


/*
 *
 */

inline void BiMotor::Rotate(
        MTMovDir_enum Direction,
        uint8_t Angle,            // the unit is 10 degree
        MTSpeed_enum Speed
        )
{
    MTSingleAct_enum Action;
    uint8_t i;
    //check para


    if ( MT_CLOCKRANDOM == Direction )
    {
        randomSeed(millis());
        Direction = (0 == random(2)) ? MT_CLOCKWISE : MT_ANTICLOCK;
    }

    if ( ANGLE_RANDOM == Angle )
    {
        Angle = random(10);
    }

    if ( 0 == Angle )
    {
        return;
    }

    /*set dirction*/
    switch(Direction)
    {
    case MT_CLOCKWISE:
        SetSingleMotorAction( MOTOR_INDEX_L, MT_ACTION_FORWORD);
        SetSingleMotorAction( MOTOR_INDEX_R, MT_ACTION_BACKWORD);
        break;

    case MT_ANTICLOCK:
        SetSingleMotorAction( MOTOR_INDEX_L, MT_ACTION_BACKWORD);
        SetSingleMotorAction( MOTOR_INDEX_R, MT_ACTION_FORWORD);
        break;

    default:
        ; //error

    }

    /** RunCnt must be set before SpeedVal**/
    SetRotateAngle(Angle);
    SetSpeed(Speed);

    Run();
}

inline void BiMotor::Stop( )
{
    RunCnt = 0;
    SpeedVal = MT_SPEED_ZERO;
    _SFR_MEM16(OCR1A_MEM) = (uint16_t)((uint32_t)SpeedVal*ICR1)/255;
}

/*private*/
inline void BiMotor::Run( )
{
    _SFR_MEM16(OCR1A_MEM) = (uint16_t)((uint32_t)SpeedVal*ICR1)/255;
}

/*
 * to control the rotation of single motor
 */

void BiMotor::SetSingleMotorAction( uint8_t Index, MTSingleAct_enum Action )
{

    uint8_t DriverA, DriverB;
    //assert para

    DriverA = MotorPin[Index].DriverA;
    DriverB = MotorPin[Index].DriverB;

    switch (Action)
    {
    case MT_ACTION_FORWORD:
        digitalWrite(DriverA, HIGH);
        digitalWrite(DriverB, LOW);
        break;
    case MT_ACTION_BACKWORD:
        digitalWrite(DriverA, LOW);
        digitalWrite(DriverB, HIGH);
        break;
    case MT_ACTION_STOP:
        digitalWrite(DriverA, LOW);
        digitalWrite(DriverB, LOW);
        break;
    default:
        ; //error
    }

}


inline void BiMotor::SetSpeed( MTSpeed_enum Speed )
{
    SpeedVal = Speed;
}



inline void BiMotor::SetMoveDistance( uint8_t Distance )
{
    RunCnt = (uint32_t)MT_STEP_ONE;      // todo: to add measure program later
}

inline void BiMotor::SetRotateAngle( uint8_t Angle )
{

    RunCnt = (uint32_t)Angle * MT_ROTATE_ONE;  // todo: to add measure program later
}


/*
 * to control the distance by time
 */
void BiMotor::TimerProc()
{
    if ( MT_SPEED_ZERO == SpeedVal )
    {
        return;
    }

    if( 0 == RunCnt )
    {
        Stop();
    }
    else
    {
        RunCnt --;
    }

}


