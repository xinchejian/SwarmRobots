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

#include "Arduino.h"
#include "Action.h"

/* (InfoType+InfoID), must be lined up in sequnce.
 * it is default to stop when (InfoType+InfoID) be executed.
 */
RuleRec_stru Action_cls::RuleTbl[] =
{
    {EXTERIORINFO, OBSTACLE_FRONT_LEFT, COND_TRUE, ACTION_MOVE, MT_CLOCKWISE, 3 },
    {EXTERIORINFO, OBSTACLE_FRONT_LEFT, COND_FALSE, ACTION_JUMP, EXTERIORINFO, OBSTACLE_FRONT_RIGHT },
    //default stopping
    {EXTERIORINFO, OBSTACLE_FRONT_RIGHT, COND_TRUE, ACTION_MOVE, MT_ANTICLOCK, 3 },
    {EXTERIORINFO, OBSTACLE_FRONT_RIGHT, COND_FALSE, ACTION_JUMP, INTERIORINFO, MOVE_FORWARD_BIGSTEP },

    {INTERIORINFO, MOVE_FORWARD_BIGSTEP, true, ACTION_MOVE,  MT_CLOCKRANDOM, ANGLE_RANDOM }, 
    {INTERIORINFO, MOVE_FORWARD_BIGSTEP, false, ACTION_MOVE,  MT_FORWARD, 1}   // IF NOT JUMP, QUIT
};

#define RULE_NUM  (sizeof(RuleTbl)/sizeof(RuleRec_stru))

#define RULE_INVALID_INDEX RULE_NUM


Action_cls::Action_cls(IR_Sensor &Sensor, BiMotor &Wheels, Messager_cls &Messager): Sensor(Sensor), Wheels(Wheels), Messager(Messager)
{
    uint8_t i;

    for ( i = 0; i < ACTION_INERIORINFO_NUM; ++i )
    {
        IneriorInfoTbl[i].State = false;
    }
}


void Action_cls::ActionProc()
{
    uint8_t i;
    uint8_t StartIndex, ActionID, ParaA, ParaB;

    ActionID = ACTION_NONE;

#if _DEBUG_ACTION
Serial.print("ActionProc-Wheel=");
Serial.print(Wheels.isMoving());
Serial.print(" ExInfo=");
Serial.println(Messager.IsExteriorInfoValid());
#endif

    if ( (false == Wheels.isMoving() ) && (true == Messager.IsExteriorInfoValid()) )
    {
#if _DEBUG_ACTION
        Serial.println("*Action rule begin");
#endif
        i = 0;
        StartIndex = 0;

        do
        {
            AnalyzeRule( StartIndex, &ActionID, &ParaA, &ParaB );
            if ( ACTION_JUMP == ActionID )
            {
                LocateRuleIndex( ParaA, ParaB, &StartIndex );
                if ( RULE_INVALID_INDEX == StartIndex )
                {
                    break;
                }
            }
            else
            {
                break;
            }

            i++;
        }while( i <= RULE_NUM );

        Messager.ClrExteriorInfo( );
    }

    if ( (false == Wheels.isMoving()) && (ACTION_SENDMSG != ActionID) )
    {
        Sensor.SendMessage( 0xEE, 0xFF, 0xFb ); //near detecting   todo
    }

#if _DEBUG_ACTION
Serial.print("Action: ");
Serial.print(ActionID);
Serial.print(" ParaA=");
Serial.println(ParaA);
#endif

    if ( (ACTION_NONE != ActionID) && (ACTION_JUMP != ActionID) )
    {
        ExecuteAction( ActionID, ParaA, ParaB );
    }
}
/*
 * only one
 */
void Action_cls::AnalyzeRule( uint8_t StartIndex, uint8_t *pActionID, uint8_t *pParaA, uint8_t *pParaB)
{
    uint8_t i;
    uint8_t State;

    *pActionID = ACTION_NONE;

    if ( EXTERIORINFO == RuleTbl[StartIndex].InfoType )
    {
        Messager.GetExteriorInfo( (ExteriorInfoID_enum)RuleTbl[StartIndex].InfoID , &State );
    }
    else  //INTERIORINFO
    {
        GetIneriorInfo( RuleTbl[StartIndex].InfoID , &State );
    }

    for ( i = StartIndex; i < RULE_NUM; ++i )
    {
        if ( (RuleTbl[StartIndex].InfoID != RuleTbl[i].InfoID)
                || (RuleTbl[StartIndex].InfoType != RuleTbl[i].InfoType) )
        {
            break;
        }

        if ( State == RuleTbl[i].InfoState )
        {
            *pActionID = RuleTbl[i].ActionID;   //todo
            *pParaA = RuleTbl[i].ParaA;
            *pParaB = RuleTbl[i].ParaB;
        }


    }

}



void Action_cls::LocateRuleIndex( uint8_t InfoType, uint8_t InfoID, uint8_t *pLocatedIndex )
{
    uint8_t i;

    *pLocatedIndex = RULE_INVALID_INDEX;
    for ( i = 0; i < RULE_NUM; ++i )
    {
        if ( (InfoType == RuleTbl[i].InfoType)
                && (InfoID == RuleTbl[i].InfoID) )
        {
            *pLocatedIndex = i;
            break;
        }
    }
}



void Action_cls::ExecuteAction( uint8_t ActionID, uint8_t ParaA, uint8_t ParaB )
{
    static uint8_t ContinueStepCnt = 0;  //only used in move straight to single direction
    static MTMovDir_enum MoveLastDiretion = MT_MOV_NULL;

    switch (ActionID)
    {

    case ACTION_MOVE:

        Wheels.Move( (MTMovDir_enum)ParaA, ParaB );

        if ( ParaA != MoveLastDiretion )   //todo
        {
            MoveLastDiretion = (MTMovDir_enum)ParaA;
            ContinueStepCnt = 1;

            IneriorInfoTbl[MOVE_FORWARD_BIGSTEP].State = false;
            IneriorInfoTbl[MOVE_BACKWARD_BIGSTEP].State = false;
        }
        else
        {
            ContinueStepCnt++;
        }

        if ( ContinueStepCnt >= BIGSTEP_TIMES )
        {
            ContinueStepCnt = 1;

            if ( MT_FORWARD == ParaA )
            {
                SetIneriorInfo( MOVE_FORWARD_BIGSTEP, true);
            }
            else if ( MT_BACKWARD == ParaA )
            {
                SetIneriorInfo( MOVE_BACKWARD_BIGSTEP, true);
            }
            else
            {
                ;  //nothing
            }

        }

        break;

    case ACTION_SENDMSG:

        break;

    }

}

inline void Action_cls::GetIneriorInfo( uint8_t InfoID, uint8_t *pState )
{
    *pState = IneriorInfoTbl[InfoID].State;
}

inline void Action_cls::SetIneriorInfo( uint8_t InfoID, uint8_t State )
{
    IneriorInfoTbl[InfoID].State = State;

#if _DEBUG_ACTION
    Serial.print("SetIneriorInfo: I=");
    Serial.print(InfoID);
    Serial.print("S=");
    Serial.println(State);
#endif
}


