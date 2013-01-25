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

    Contact: leo.yan.cn@gmail.com
*/

#include "Arduino.h"
#include "SwarmRobot.h"
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
    MotorPin[MOTOR_INDEX_R].DriverA = MOTOR_R_DIRPINA;
    MotorPin[MOTOR_INDEX_R].DriverB = MOTOR_R_DIRPINB;

    Stop();
    //pin initialization

    pinMode(MotorPin[MOTOR_INDEX_L].DriverA,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_L].DriverB,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_R].DriverA,OUTPUT);
    pinMode(MotorPin[MOTOR_INDEX_R].DriverB,OUTPUT);

    LastMoveDirection = MT_FORWARD;
    LastMovePara = 0;

    StepRepeatState = false;
    RotateRepeatState = false;
    RotatePingPangState = false;

}


/* MT_STOP is also consider as moving in order to balance time. */
bool BiMotor::isMoving()
{
    return ( (MT_SPEED_ZERO == SpeedVal)? false : true  );
}

bool BiMotor::isStepRepeat()
{
    return StepRepeatState;
}

bool BiMotor::isRotateRepeat()
{
    return RotateRepeatState;
}

bool BiMotor::isRotatePingPang()
{
    return RotatePingPangState;
}


void BiMotor::GetMoveAciton( MTMovDir_enum &Direction, uint8_t &Para )
{
    Direction = LastMoveDirection;
    Para = LastMovePara;
}



void BiMotor::Move( MTMovDir_enum Direction, uint8_t Para, MTSpeed_enum Speed )
{

    if ( 0 == Para )
    {
        ASSERT_T(0);
        return;
    }

    switch ( Direction )
    {
    case MT_FORWARD:
    case MT_BACKWARD:
    case MT_STOP:
        Walk( Direction, Para );
        break;

    case MT_CLOCKWISE:
    case MT_ANTICLOCK:
        Rotate( Direction, Para );
        break;
        
    case MT_TURNLEFT:
    case MT_TURNRIGHT:
        //todo
        break;

    default:
        ASSERT_T(0);
    }

    /** RunCnt must be set before SpeedVal**/
    SetSpeed(Speed);

    Run();

    Statistic( Direction, Para);

}

void BiMotor::Statistic( MTMovDir_enum Direction, uint8_t Para )
{
    static uint8_t RepeatCnt = 0;
    static uint8_t RotatePingPangCnt = 0;

    /**judge repeated single action**/
    if ( LastMoveDirection != Direction )
    {
        RepeatCnt = 0;
        StepRepeatState = false;
        RotateRepeatState = false;
    }
    else
    {
        if ( RepeatCnt < 255 )
        {
            RepeatCnt++;
        }
    }

    if ( MT_FORWARD == Direction )
    {
        StepRepeatState = (RepeatCnt >= REPEATED_STEP_THRESHOLD) ? true : false;
    }
    else if ( (MT_CLOCKWISE == Direction) || (MT_CLOCKWISE == Direction) )
    {
        RotateRepeatState = (RepeatCnt >= REPEATED_ROTATE_THRESHOLD) ? true : false;
    }
    else
    {
        //nothing
    }

    /**judge rotate PingPong state**/

    if ( (MT_CLOCKWISE != Direction) && (MT_ANTICLOCK != Direction) )
    {
        RotatePingPangCnt = 0;
    }
    else
    {
        if ( (LastMoveDirection != Direction)
                && (RotatePingPangCnt < 255) )
        {   //the fist is maybe incorrect, but it isn't a matter if PINGPANG_ROTATE_THRESHOLD > 2.

            RotatePingPangCnt++;
        }
    }

    RotatePingPangState = (RotatePingPangCnt >= PINGPANG_ROTATE_THRESHOLD) ? true : false;

    /**store info**/
    LastMoveDirection = Direction;
    LastMovePara = Para;
}


/* Walk == Go straight */

inline void BiMotor::Walk(
        MTMovDir_enum Direction,
        uint8_t Distance         //the unit is 1 cm
        )
{
    MTSingleAct_enum Action;
    uint8_t i;


    /*set direction*/
    switch(Direction)
    {
    case MT_FORWARD:
        Action = MT_ACTION_FORWORD;
        break;

    case MT_BACKWARD:
        Action = MT_ACTION_BACKWORD;
        break;

    case MT_STOP:
        Action = MT_ACTION_STOP;
        break;

    default:
        return;

    }

    for (i = 0; i < MOTOR_NUM; ++i )
    {
        SetSingleMotorAction( i, Action);
    }

    RunCnt = (uint32_t)Distance * MT_STEP_ONE;      // todo: to add measure program later
}


/*
 *
 */

inline void BiMotor::Rotate(
        MTMovDir_enum Direction,
        uint8_t Angle              // the unit is 10 degree
        )
{

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

    RunCnt = (uint32_t)Angle * MT_ROTATE_ONE;  // todo: to add measure program later
}

inline void BiMotor::Stop( )
{
    uint8_t i;

    RunCnt = 0;
    SpeedVal = MT_SPEED_ZERO;

    for (i = 0; i < MOTOR_NUM; ++i )
    {
        SetSingleMotorAction( i, MT_ACTION_STOP );
    }
}

/*private*/
inline void BiMotor::Run( )
{
    /*if the driver chip has Enable pin, add code here*/
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


