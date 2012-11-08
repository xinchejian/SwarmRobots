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

/* Rules are consisted with blocks.  The 'INTERIORINFO, TASK_STAGE' is the first sentence of each block.
 *
 */


#if ( ROBOT_TYPE_ANT == ROBOT_INIT_TYPE  )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0 },
    {INTERIORINFO, POWER, INFO_STATE_FULL,  ACTION_IN_SET, TASK_STAGE, TS_LOOKINGPOWER },

    {INTERIORINFO, TASK_STAGE, TS_LOOKINGPOWER,  0, 0, 0 },
    {ENVINFO, ROBOT_ID_ANY, ROBOT_TYPE_POWERSTATION,  ACTION_IN_SET, TARGET_NEXT, INFO_STATE_SET},
    {ENVINFO, ROBOT_ID_ANY, ROBOT_TYPE_POWERSTATION,  ACTION_IN_SET, TASK_STAGE, TS_FOUNDPOWER},

    {INTERIORINFO, TASK_STAGE, TS_FOUNDPOWER,  0, 0, 0 },
    {INTERIORINFO, TARGET_NEXT, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_LOOKINGPOWER },
    {INTERIORINFO, TARGET_NEXT, INFO_STATE_ARRIVED,  ACTION_EXT_FINISH, 0, 0 },
    {INTERIORINFO, TARGET_NEXT, INFO_STATE_SET,  ACTION_EXT_MOVE, MOVE_TARGET, 0 },
    {ENVINFO, ROBOT_ID_TARGET, MSGID_NOTIFY_IFINDYOU,  ACTION_IN_SET, TARGET_NEXT, INFO_STATE_ARRIVED},

};

RuleRec_stru Action_cls::TimerRuleTbl[] ={};

RuleRec_stru Action_cls::PreRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0 },
    {INTERIORINFO, IS_MOVING, INFO_STATE_TRUE,  ACTION_FLOW_STOP, 0, 0 },
    {INTERIORINFO, IS_ENVINFO, INFO_STATE_FALSE,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_SELFTYPE },
};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0 },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0 },
};


#else
RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,   0, 0, 0 },
    {ENVINFO, ROBOT_ID_ANY, ROBOT_TYPE_ANT, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_NOTIFY_IFINDYOU,},
};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,   0, 0, 0 },
    {INTERIORINFO, TIMER, INFO_STATE_TRUE, ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_SELFTYPE },
    {INTERIORINFO, TIMER, INFO_STATE_TRUE,  ACTION_IN_SET, TIMER, INFO_STATE_FALSE },
};


RuleRec_stru Action_cls::PreRuleTbl[] = {};

RuleRec_stru Action_cls::PostRuleTbl[] ={};

#endif


Action_cls::Action_cls(IR_Sensor &Sensor, BiMotor &Wheels, Messager_cls &Messager): Sensor(Sensor), Wheels(Wheels), Messager(Messager)
{

    InteriorInfoTbl[ROBOT_TYPE].Para = ROBOT_INIT_TYPE;
    InteriorInfoTbl[ROBOT_TYPE].State = INFO_STATE_TRUE;

    InteriorInfoTbl[TASK_STAGE].State = TS_INIT;
    InteriorInfoTbl[IS_ACTION_MOVE].State = INFO_STATE_FALSE;

    InteriorInfoTbl[IS_REPEAT_SWING].State = INFO_STATE_FALSE;
    InteriorInfoTbl[IS_BIGSTEP].State = INFO_STATE_FALSE;
    InteriorInfoTbl[TARGET_NEXT].State = INFO_STATE_EMPTY;

    InteriorInfoTbl[WEIGHT].Para = 0xFF;
    InteriorInfoTbl[WEIGHT].State = INFO_STATE_TRUE;

    InteriorInfoTbl[POWER].State = INFO_STATE_FULL;

    InteriorInfoTbl[TIMER].Para = 0;
    InteriorInfoTbl[TIMER].State = INFO_STATE_FALSE;

    RuleNum = sizeof(RuleTbl)/sizeof(RuleRec_stru);
    TimerRuleNum = sizeof(TimerRuleTbl)/sizeof(RuleRec_stru);
    PreRuleNum = sizeof(PreRuleTbl)/sizeof(RuleRec_stru);
    PostRuleNum = sizeof(PostRuleTbl)/sizeof(RuleRec_stru);

}


void Action_cls::ActionProc()
{
    uint8_t i;
    uint8_t State;
    uint8_t MovingState, EnvInfoState;

/* EnvInfoFlag = Messager.IsEnvInfoValid()
 *   MovingFlag  EnvInfoFlag
 *   false        false        than send detecting msg
 *   false        true         pre  + action  + post
 *   true         false        NA
 *   true         true         NA
 * if (false == MovingFlag) && (true == EnvInfoFlag)
 * */

    InteriorInfoUpdate();

    if ( 0 < PreRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("PreAction:");
#endif
        ActionRule( PreRuleTbl, PreRuleNum );
    }

    if ( 0 < TimerRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("TimerAction:");
#endif
        ActionRule( TimerRuleTbl, TimerRuleNum );
    }

    GetInteriorInfo( IS_MOVING, &MovingState );
    GetInteriorInfo( IS_ENVINFO, &EnvInfoState );

    if ( (INFO_STATE_FALSE == MovingState) && (INFO_STATE_TRUE == EnvInfoState) )
    {
#if _DEBUG_ACTION
        Serial.print("*MainAction- TaskState=0x");
        Serial.print(InteriorInfoTbl[TASK_STAGE].State, HEX);
        Serial.print(" Num=");
        Serial.println(RuleNum);
#endif
        ASSERT_T( NULL != RuleTbl );
        ActionRule( RuleTbl, RuleNum );

        if ( 0 < PostRuleNum )
        {
#if _DEBUG_ACTION
Serial.print("PostAction:");
Serial.print(" Num=");
Serial.println(PostRuleNum);
#endif
            ActionRule( PostRuleTbl, PostRuleNum );
        }

        Messager.ClrEnvInfo();
    }
    
}

void Action_cls::InteriorInfoUpdate()
{
    uint8_t State, RobotID, MsgID;
    uint8_t Ret;

    /* */
    State = (true == Wheels.isMoving()) ? INFO_STATE_TRUE : INFO_STATE_FALSE;
    SetInteriorInfo( IS_MOVING, State );

    /* */
    State = (true == Messager.IsEnvInfoValid()) ? INFO_STATE_TRUE : INFO_STATE_FALSE;
    SetInteriorInfo( IS_ENVINFO, State );

    /* */
    SetInteriorInfo( IS_ACTION_MOVE, INFO_STATE_FALSE );

    /* */
    GetInteriorInfo( TARGET_NEXT, &State, &RobotID );
    if ( INFO_STATE_EMPTY != State )
    {
        MsgID = MSGID_ANY;
        Ret = Messager.GetEnvRobotInfo( &RobotID, &MsgID );
        if ( SUCCESS != Ret )
        {
            SetInteriorInfo( TARGET_NEXT, INFO_STATE_EMPTY );
        }
    }
}

/*
 * only one
 */
void Action_cls::ActionRule( RuleRec_stru *pTbl, uint8_t RecNum )
{
    uint8_t i;
    uint8_t RobotID, MsgID, RobotPara, RobotPos;
    uint8_t InfoID, InfoPara;
    uint8_t InState, InPara;
    uint8_t ActionID, ActParaA, ActParaB;
    uint8_t Ret;
    bool InfoFlag;

    for ( i = 0; i < RecNum; ++i )
    {
        /*to find the block*/
        if ( (INTERIORINFO == pTbl[i].InfoType)
                && (TASK_STAGE == pTbl[i].InfoID)
                && ( (InteriorInfoTbl[TASK_STAGE].State == pTbl[i].InfoPara)
                        || (TS_ANY == pTbl[i].InfoPara)) )
        {
            i++;
            break;
        }
    }


    for ( ; i < RecNum; ++i )
    {
        if ( (INTERIORINFO == pTbl[i].InfoType)
                && (TASK_STAGE == pTbl[i].InfoID) )
        {
            /*New block and then break*/
            break;
        }

        InfoFlag = false;
        InfoID = pTbl[i].InfoID;
        InfoPara = pTbl[i].InfoPara;

        if ( ENVINFO == pTbl[i].InfoType )
        {
            RobotID = InfoID;
            MsgID = InfoPara;
            if ( ROBOT_ID_TARGET == RobotID )
            {
                GetInteriorInfo( TARGET_NEXT, &InState, &InPara );
                ASSERT_T( INFO_STATE_EMPTY != InState );
                RobotID = InPara;
            }

            if (INFO_STATE_EMPTY != InState)
            {
                Ret = Messager.GetEnvRobotInfo( &RobotID, &MsgID, &RobotPara, &RobotPos);
#if _DEBUG_ACTION
                Serial.print(" GetEnvRobotInfo ret=");
                Serial.print(Ret);
                Serial.print(" RobotID=");
                Serial.print(RobotID);
                Serial.print(" RobotPara=");
                Serial.println(MsgID);
#endif
                if ( SUCCESS ==  Ret)
                {
                    InfoFlag = true;
                }
            }
        }
        else  //INTERIORINFO
        {
            InfoID = pTbl[i].InfoID;
            InfoPara = pTbl[i].InfoPara;
            GetInteriorInfo( InfoID, &InState );
            if ( InfoPara == InState )
            {
                InfoFlag = true;
            }
        }

        if ( true == InfoFlag )
        {
            ActionID = pTbl[i].ActionID;
            ActParaA = pTbl[i].ActParaA;
            ActParaB = pTbl[i].ActParaB;

#if _DEBUG_ACTION
Serial.print("Act: ");
Serial.print(ActionID);
Serial.print(" ParaA=");
Serial.print(ActParaA);
Serial.print(" ParaB=");
Serial.print(ActParaB);
Serial.print(" /Rule:");
Serial.println(i);
#endif

            switch ( ActionID )
            {
            case ACTION_FLOW_STOP:
                i = RecNum;
                break;

            case ACTION_IN_SET:
                ASSERT_T( ActParaA < INNERIOR_ID_END);

                InteriorInfoTbl[ActParaA].State = (InteriorInfoState_enum)ActParaB;

                if ( INFO_STATE_SET == ActParaB )
                {   //todo: optimize later
                    InteriorInfoTbl[ActParaA].Para = RobotID;
                }


                break;

            case ACTION_EXT_MOVE:
                i = RecNum;

                ActionMove( ActParaA );
                break;

            case ACTION_EXT_SENDMSG:
                i = RecNum;

                //todo: optimize later
                if ( ROBOT_ID_SOURCE == ActParaA )
                {
                    ActParaA == RobotID;
                }

                ActionSendMsg( ActParaA, ActParaB );

                break;
            case ACTION_EXT_FINISH:
                i = RecNum;
                ActionFinish();

                break;

            default:
                ASSERT_T(0);
            }

        }

    }



}


void Action_cls::ActionSendMsg( uint8_t RobotID, uint8_t MsgID )
{
    uint8_t InPara, InState;

    if ( ROBOT_ID_TARGET == RobotID )
    {
        GetInteriorInfo( TARGET_NEXT, &InState, &InPara );
        ASSERT_T( INFO_STATE_EMPTY != InState );
        RobotID = InPara;
    }

    if ( MSGID_SELFTYPE == MsgID )
    {
        GetInteriorInfo( ROBOT_TYPE, &InState, &InPara );
        MsgID = InPara;
    }

    GetInteriorInfo( WEIGHT, &InState, &InPara );

    Sensor.SendMessage( RobotID, MsgID, InPara );
}

void Action_cls::ActionMove( uint8_t MoveType )
{
    static uint8_t aMoveTagetRule[] = { TARGET_POS_F, TARGET_POS_L, TARGET_POS_R, TARGET_POS_B, \
            TARGET_POS_FL, TARGET_POS_FR, TARGET_POS_BL, TARGET_POS_BR };

    uint8_t RobotID, State, MsgID, RobotPara, RobotPos;
    uint8_t Direction;
    uint8_t Ret;
    uint8_t i;


    if ( MOVE_WALKAROUND == MoveType )
    {   /*Walk around*/
        Direction = TARGET_POS_F;
    }
    else
    {   /*Move to target*/
        GetInteriorInfo(TARGET_NEXT, &State, &RobotID );
        ASSERT_T( INFO_STATE_EMPTY != State );
        MsgID = MSGID_ANY;

        Ret = Messager.GetEnvRobotInfo( &RobotID, &MsgID, &RobotPara, &RobotPos );
        if ( SUCCESS != Ret )
        {
            return;
        }

        for ( i = 0; i < POSITION_NUM; ++i )
        {
            Direction = aMoveTagetRule[i];
            if ( 1 == bitRead( RobotPos, Direction) )
            {
                break;
            }
        }
    }

    ExecuteMove( Direction );

    SetInteriorInfo( IS_ACTION_MOVE, INFO_STATE_TRUE );

}

void Action_cls::ActionFinish()
{
    /*todo: Give visiable behavior or light */
    Wheels.Move( MT_CLOCKWISE, 36 );

    /*todo: Go in sleep mode */
    for (;;);
}

void Action_cls::ExecuteMove( uint8_t Direct )
{
    static uint8_t ContinueStepCnt = 0;  //only used in move straight to single direction
    static MTMovDir_enum MoveLastDiretion = MT_MOV_NULL;
    static uint8_t aObstacleRule[] = { TARGET_POS_F, TARGET_POS_FL, TARGET_POS_FR, TARGET_POS_L, \
                                        TARGET_POS_R, TARGET_POS_BL, TARGET_POS_BR, TARGET_POS_B };
    static MoveRule_stru aMoveRule[] =
    {
                { MT_ANTICLOCK, 3 },//TARGET_POS_FL = IR_POSITION_FL,
                { MT_CLOCKWISE, 3 },//TARGET_POS_FR = IR_POSITION_FR,
                { MT_ANTICLOCK, 12 },//TARGET_POS_BL =IR_POSITION_BL,
                { MT_CLOCKWISE, 12 },//TARGET_POS_BR = IR_POSITION_BR,
                { MT_FORWARD, 1 },//TARGET_POS_F,
                { MT_ANTICLOCK, 8 },//TARGET_POS_L,
                { MT_CLOCKWISE, 8 },//TARGET_POS_R,
                { MT_CLOCKWISE, 16 },//TARGET_POS_B    //it must be the last
    };

    MTMovDir_enum MoveDiret;
    uint8_t Para;
    uint8_t ObstacleInfo, State;
    uint8_t i;

    /* if there is a obstacle in the 'Direction', it will avoid obstacles firstly */
    Messager.GetEnvObstacleInfo( &ObstacleInfo );
    
#if _DEBUG_ACTION
    Serial.print("**Obstacle=0b");
    Serial.println(ObstacleInfo, BIN);
#endif

    if ( 1 == bitRead( ObstacleInfo, Direct) )
    {
        Direct = POSITION_INVALID;
        /*move to the direction where is not obstacle*/
        for ( i = 0; i < POSITION_NUM; ++i )
        {

            if ( 0 == bitRead( ObstacleInfo, aObstacleRule[i]) )
            {
                Direct = aObstacleRule[i];
                break;
            }
        }

    }

    if ( POSITION_INVALID != Direct )
    {
        MoveDiret = aMoveRule[Direct].Direction;
        Para = aMoveRule[Direct].Para;

#if 0  //todo??
        GetInteriorInfo( IS_BIGSTEP, &State );
        if ( (INFO_STATE_TRUE == State) && (MT_FORWARD == MoveDiret) )
        {
            MoveDiret = MT_CLOCKRANDOM;
            Para = ANGLE_RANDOM;
        }
#endif
    }
    else
    {   //obstacle in all directions
        MoveDiret = MT_CLOCKRANDOM;
        Para = ANGLE_RANDOM;
    }



    Wheels.Move( MoveDiret, Para);


    /*judge IS_BIGSTEP*/
    GetInteriorInfo( TARGET_NEXT, &State );

    if ( (MoveDiret != MoveLastDiretion) || (INFO_STATE_EMPTY != State) )   //todo
    {   //if target exist once, BIGSTEP reset to init-state. in order to reach the target quickly.
        MoveLastDiretion = MoveDiret;
        ContinueStepCnt = 1;

        SetInteriorInfo( IS_BIGSTEP, INFO_STATE_FALSE);

    }
    else
    {
        ContinueStepCnt++;
    }

    if ( ContinueStepCnt >= BIGSTEP_TIMES )
    {
        ContinueStepCnt = 1;

        if ( MT_FORWARD == MoveDiret )
        {
            SetInteriorInfo( IS_BIGSTEP, INFO_STATE_TRUE);
        }
        else
        {
            ;  //nothing
        }
    }



}

inline void Action_cls::GetInteriorInfo( uint8_t InfoID, uint8_t *pState, uint8_t *pPara )
{
    *pState = InteriorInfoTbl[InfoID].State;
    if ( NULL != pPara )
    {
        *pPara = InteriorInfoTbl[InfoID].Para;
    }
}

inline void Action_cls::SetInteriorInfo( uint8_t InfoID, uint8_t State )
{
    InteriorInfoTbl[InfoID].State = (InteriorInfoState_enum)State;

#if 0
    Serial.print("SetInteriorInfo: I=");
    Serial.print(InfoID);
    Serial.print("S=");
    Serial.println(State);
#endif
}

#define ACTION_BASE_FREQUENCY 20
#define ACTION_TIMER_FREQUENCY 2

void Action_cls::TimerProc()
{
    static uint16_t ActionTimerBase = (g_BaseTimeFrequency/ACTION_BASE_FREQUENCY);
    static uint16_t BaseCnt = 0;


    BaseCnt++;
    if ( BaseCnt < ActionTimerBase )
    {
        return;
    }
    BaseCnt = 0;

    uint8_t &TimerCnt = InteriorInfoTbl[TIMER].Para;
    TimerCnt++;

    if ( TimerCnt >= (ACTION_BASE_FREQUENCY/ACTION_TIMER_FREQUENCY) )
    {
        TimerCnt = 0;
        InteriorInfoTbl[TIMER].State = INFO_STATE_TRUE;
    }

}


