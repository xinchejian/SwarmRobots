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
 * Only one block is executed each time
 * If the condition is false, it will shift to the next rule automatically.
 */


#if ( ROBOT_TYPE_ANT == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {INTERIORINFO, POWER, INFO_STATE_FULL,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false },

    {INTERIORINFO, TASK_STAGE, TS_LOOKING,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
    {ENVINFO, ROBOT_ID_TYPE, ROBOT_TYPE_POWERSTATION,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {ENVINFO, ROBOT_ID_TYPE, ROBOT_TYPE_POWERSTATION,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

    {INTERIORINFO, TASK_STAGE, TS_FOUND,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false},
    {ENVINFO, ROBOT_ID_TARGET, MSGID_NEARRANGE,  ACTION_IN_SET, TASK_STAGE, TS_CLOSED,  false},
    {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },


    {INTERIORINFO, TASK_STAGE, TS_CLOSED,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
    {INTERIORINFO, COUNTER, 6,  ACTION_IN_SET, TASK_STAGE, TS_TASKOVER,  false},  //if next is not Game Over, shout set the state to false
    {ANYINFO, 0, 0,  ACTION_IN_INCR, COUNTER, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },

    {INTERIORINFO, TASK_STAGE, TS_TASKOVER,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_GAMEWIN, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_NOTIFY_DONE,  true },
    {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_STOP, 0,  false},

};


RuleRec_stru Action_cls::TimerRuleTbl[] = {};

RuleRec_stru Action_cls::PreRuleTbl[] = {};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};

#elif ( ROBOT_TYPE_ANT_LINEUP == ROBOT_INIT_TYPE )

/*The msg between two ants
 *    ANT-A       ANT-B
 *    --MSGID_REQUEST-->
 *    <--MSGID_RESPONCE--   than ANT-A become follower
 *    --MSGID_ACK    -->    than ANT-B become followed
 *
 */
RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },
//
    {INTERIORINFO, TASK_STAGE, TS_Lonely,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
    {ENVINFO, ROBOT_ID_ANY, MSGID_ACK,  ACTION_IN_SET, FOLLOWER, INFO_STATE_SET, true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeFollowed,  false},

    {ENVINFO, ROBOT_ID_ANY, MSGID_RESPONCE, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_ACK, true},
    {ENVINFO, ROBOT_ID_ANY, MSGID_RESPONCE,  ACTION_IN_SET, TARGET, INFO_STATE_SET, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_Following,  false},

    {ENVINFO, ROBOT_ID_LESS, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_REQUEST,  false},
//
    {INTERIORINFO, TASK_STAGE, TS_BeFollowed,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {ENVINFO, ROBOT_ID_ANY, MSGID_RESPONCE, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_ACK, true},
    {ENVINFO, ROBOT_ID_ANY, MSGID_RESPONCE,  ACTION_IN_SET, TARGET, INFO_STATE_SET, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeNode,  false},

    {ENVINFO, ROBOT_ID_FOLLOWER, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_REQUEST,  false},
//
    {INTERIORINFO, TASK_STAGE, TS_Following,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B10,  true},
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {ENVINFO, ROBOT_ID_ANY, MSGID_ACK,  ACTION_IN_SET, FOLLOWER, INFO_STATE_SET, true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeNode,  false},

    {ENVINFO, ROBOT_ID_LESS, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_ACK,  true},

    {INTERIORINFO, TARGET, INFO_STATE_ARRIVED,  ACTION_EXT_MOVE, MOVE_STOP, 0,  false },  //stop
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },

//
    {INTERIORINFO, TASK_STAGE, TS_BeNode,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Following,  false },
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_BeFollowed,  false },

    {ENVINFO, ROBOT_ID_FOLLOWER, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_ACK,  true},

    {INTERIORINFO, TARGET, INFO_STATE_ARRIVED,  ACTION_EXT_MOVE, MOVE_STOP, 0,  false },  //stop
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },
};



RuleRec_stru Action_cls::TimerRuleTbl[] ={};

RuleRec_stru Action_cls::PreRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0 },
    {INTERIORINFO, IS_MOVING, INFO_STATE_TRUE,  ACTION_NONE, 0, 0,  false },
    {INTERIORINFO, IS_ENVINFO, INFO_STATE_FALSE,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_SELFTYPE,  false },
};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};

#elif ( ROBOT_TYPE_POWERSTATION == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,   0, 0, 0, true },
    {ANYINFO, 0, 0,  ACTION_IN_SET_PARA, COUNTER, 1, true},
    {ANYINFO, 0, 0,  ACTION_IN_SET, STATISTIC_CONDITION, CONDITION_LARGER, true},
    {ANYINFO, 0, 0,  ACTION_IN_SET_PARA, STATISTIC_CONDITION, 2, true},   //set the arrived quantity.
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_RUNNING,  false},

    {INTERIORINFO, TASK_STAGE, TS_RUNNING,   0, 0, 0, true },
    {ANYINFO, 0, 0, ACTION_EXT_STATISTIC, MSGID_NOTIFY_DONE, 0, true},
    {INTERIORINFO, STATISTIC_STATE, INFO_STATE_TRUE,  ACTION_IN_SET, TASK_STAGE, TS_TASKOVER, false },

    {INTERIORINFO, TASK_STAGE, TS_TASKOVER,   0, 0, 0, true },
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_GAMEWIN, 0,  true},
    {ANYINFO, 0, 0, ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_NEARRANGE, false},
};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
  //  {INTERIORINFO, TASK_STAGE, TS_ANY,   0, 0, 0, true },
  //  {INTERIORINFO, TIMER, INFO_STATE_TRUE, ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_SELFTYPE, true },
  //  {INTERIORINFO, TIMER, INFO_STATE_TRUE, ACTION_IN_SET, TIMER, INFO_STATE_FALSE, false },
};


RuleRec_stru Action_cls::PreRuleTbl[] = {};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, COUNTER, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_SELFTYPE,  true},
    {INTERIORINFO, COUNTER, 1,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_NEARRANGE,  true},
    {ANYINFO, 0, 0,  ACTION_IN_INCR, COUNTER, 0,  true},
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_STOP, 0, false },  //delay time
};

#endif


Action_cls::Action_cls(IR_Sensor &Sensor, BiMotor &Wheels, Messager_cls &Messager): Sensor(Sensor), Wheels(Wheels), Messager(Messager)
{

    InteriorInfoTbl[ROBOT_TYPE].Para = ROBOT_INIT_TYPE;
    InteriorInfoTbl[ROBOT_TYPE].State = INFO_STATE_TRUE;

    InteriorInfoTbl[TASK_STAGE].State = TS_INIT;
    InteriorInfoTbl[IS_ACTION_MOVE].State = INFO_STATE_FALSE;

    InteriorInfoTbl[IS_REPEAT_SWING].State = INFO_STATE_FALSE;
    InteriorInfoTbl[IS_BIGSTEP].State = INFO_STATE_FALSE;
    InteriorInfoTbl[TARGET].State = INFO_STATE_EMPTY;
    InteriorInfoTbl[FOLLOWER].State = INFO_STATE_EMPTY;

    InteriorInfoTbl[WEIGHT].Para = 0xFE;
    InteriorInfoTbl[WEIGHT].State = INFO_STATE_TRUE;

    InteriorInfoTbl[POWER].State = INFO_STATE_FULL;

    InteriorInfoTbl[TIMER].Para = 0;
    InteriorInfoTbl[TIMER].State = INFO_STATE_FALSE;

    /*COUNTER; State = Count uumber;  Para = Threshold, if Count reach the Threshold, it will be set 0*/
    InteriorInfoTbl[COUNTER].State = 0;
    InteriorInfoTbl[COUNTER].Para = 0xFE;

    
    InteriorInfoTbl[STATISTIC_STATE].State = INFO_STATE_FALSE;

    SetInteriorInfo( LASTMOVE, MT_FORWARD, 0 );

    RuleNum = sizeof(RuleTbl)/sizeof(RuleRec_stru);
    TimerRuleNum = sizeof(TimerRuleTbl)/sizeof(RuleRec_stru);
    PreRuleNum = sizeof(PreRuleTbl)/sizeof(RuleRec_stru);
    PostRuleNum = sizeof(PostRuleTbl)/sizeof(RuleRec_stru);

    MsgInfo.Flag = false;

}


void Action_cls::ActionProc()
{
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

    QueryTimeProc(GetTimeOn100ms());

    GetInteriorInfo( IS_MOVING, &MovingState );
    GetInteriorInfo( IS_ENVINFO, &EnvInfoState );

    if ( (INFO_STATE_FALSE == MovingState) && (INFO_STATE_FALSE == EnvInfoState) )
    {
        ExecuteSendMsg();
    }

    if ( 0 < PreRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("PreAct:");
#endif
        ActionRule( PreRuleTbl, PreRuleNum );
    }

    if ( 0 < TimerRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("TimerAct:");
#endif
        ActionRule( TimerRuleTbl, TimerRuleNum );
    }


    if ( (INFO_STATE_FALSE == MovingState) && (INFO_STATE_TRUE == EnvInfoState) )
    {
#if _DEBUG_ACTION
        Serial.print("*MainAc- TskState=0x");
        Serial.print(InteriorInfoTbl[TASK_STAGE].State, HEX);
        Serial.print(" Num=");
        Serial.println(RuleNum);
#endif
        ASSERT_T( NULL != RuleTbl );
        ActionRule( RuleTbl, RuleNum );

        if ( 0 < PostRuleNum )
        {
#if _DEBUG_ACTION
Serial.println("PostAct:");
#endif
            ActionRule( PostRuleTbl, PostRuleNum );
        }

        /** **/
        Messager.ClrEnvInfoLoc();
    }
    
}

/*
 * only one
 */
void Action_cls::ActionRule( RuleRec_stru *pTbl, uint8_t RecNum )
{
    uint8_t i;
    uint8_t RobotID;
    uint8_t InfoID, InfoPara;
    uint8_t InID, InState, InPara;
    uint8_t ActionID, ActParaA, ActParaB;
    uint8_t Ret;
    EnvRobotInfoRec_stru RobotInfo;
    bool InfoFlag;

    /*to look for the block from rule-table basing on State*/
    for ( i = 0; i < RecNum; ++i )
    {

        if ( (INTERIORINFO == pTbl[i].InfoType)
                && (TASK_STAGE == pTbl[i].InfoID)
                && ( (InteriorInfoTbl[TASK_STAGE].State == pTbl[i].InfoPara)
                        || (TS_ANY == pTbl[i].InfoPara)) )
        {
            i++;
            break;
        }
    }

    /*Analyse the the block and carry out the rule*/
    for ( ; i < RecNum; ++i )
    {
        /**If find a New block so stop**/
        if ( (INTERIORINFO == pTbl[i].InfoType)
                && (TASK_STAGE == pTbl[i].InfoID) )
        {
            break;
        }

        /**Analyse the condition**/
        InfoFlag = false;
        InfoID = pTbl[i].InfoID;
        InfoPara = pTbl[i].InfoPara;

        if ( ENVINFO == pTbl[i].InfoType )
        {
            RobotID = InfoID;

            InID = INNERIOR_ID_END;
            InState = INFO_STATE_EMPTY;

            if ( ROBOT_ID_TARGET == RobotID )
            {
                InID = TARGET;
            }
            else if( ROBOT_ID_FOLLOWER == RobotID )
            {
                InID = FOLLOWER;
            }
            else
            {
                //nothing
            }

            if ( INNERIOR_ID_END != InID )
            {
                /*If there is no target or follower, the rule is out of date, so stop*/
                GetInteriorInfo( InID, &InState, &InPara );
                if ( INFO_STATE_EMPTY == InState )
                {
                    break;
                }

                RobotID = InPara;
            }

            Ret = Messager.GetEnvRobotInfo( RobotID, InfoPara, RobotInfo);

            if ( SUCCESS ==  Ret)
            {
                RobotID = RobotInfo.RobotID;
                InfoFlag = true;
#if _DEBUG_ACTION
            Serial.print(" GetEnv ret=");
            Serial.print(Ret);
            Serial.print(" BotID=");
            Serial.print(RobotID);
            Serial.print(" P=");
            Serial.println(InfoPara);
#endif
            }

        }
        else if ( INTERIORINFO == pTbl[i].InfoType )
        {
            InfoID = pTbl[i].InfoID;
            InfoPara = pTbl[i].InfoPara;
            GetInteriorInfo( InfoID, &InState );
            if ( InfoPara == InState )
            {
                InfoFlag = true;
            }
        }
        else //ANYINFO
        {
            InfoFlag = true;
        }

        /**Carry out the Action**/
        if ( true == InfoFlag )
        {
            ActionID = pTbl[i].ActionID;
            ActParaA = pTbl[i].ActParaA;
            ActParaB = pTbl[i].ActParaB;

#if _DEBUG_ACTION
Serial.print(" Act: ");
Serial.print(ActionID);
Serial.print(" PA=");
Serial.print(ActParaA);
Serial.print(" ParaB=");
Serial.println(ActParaB);
//Serial.print(" Rule=");
//Serial.println(i);
#endif

            switch ( ActionID )
            {
            case ACTION_IN_SET:
                ASSERT_T( ActParaA < INNERIOR_ID_END);

                InPara = INVALID;
                if ( INFO_STATE_SET == ActParaB )
                {   //todo: optimize later
                    InPara = RobotID;
                }

                SetInteriorInfo( ActParaA, ActParaB, InPara );

                break;

            case ACTION_IN_SET_PARA:
                ASSERT_T( ActParaA < INNERIOR_ID_END );
                SetInteriorInfo( ActParaA, INFO_STATE_END, ActParaB );

                break;

            case ACTION_IN_INCR:
                ASSERT_T( ActParaA < INNERIOR_ID_END );

                GetInteriorInfo( ActParaA, &InState, &InPara );

                if ( InPara <= InState )
                {
                    InState = 0;  //State = Count number, InPara = Threshold
                }
                else
                {
                    InState++;
                }

                SetInteriorInfo( ActParaA, InState );

                break;

            case ACTION_EXT_MOVE:
                ActionMove( ActParaA, ActParaB );
                break;

            case ACTION_EXT_SENDMSG:
                //todo: optimize later
                if ( ROBOT_ID_SOURCE == ActParaA )
                {
                    ActParaA = RobotID;
                }
                else if ( ROBOT_ID_TARGET == ActParaA )
                {
                    /*If there is no target, the rule is out of date, so stop*/
                    GetInteriorInfo( TARGET, &InState, &InPara );
                    if ( INFO_STATE_EMPTY == InState )
                    {
                        break;
                    }

                    ActParaA = InPara;
                }
                else
                {
                    //nothing
                }

                ActionSetSendMsg( ActParaA, ActParaB );

                break;

            case ACTION_EXT_CLRMSG:
                //todo: optimize later
                if ( ROBOT_ID_SOURCE == ActParaA )
                {
                    ActParaA = RobotID;
                }

                ActionClrMsg( ActParaA, ActParaB );

                break;

            case ACTION_EXT_STATISTIC:
                ActionStatistic( ActParaA, ActParaB);

                break;

            case ACTION_EXT_LED:
                LedLight.SetLed( (LedMode_enum)ActParaA, ActParaB );
                break;

            case ACTION_NONE:
                break;

            default:
                ASSERT_T(0);
            }

            /*whether to stop the flow*/
            if ( false == pTbl[i].FlowFlag )
            {
                break;
            }
        }

    }
}


void Action_cls::InteriorInfoUpdate()
{
    uint8_t State, RobotID, MsgID;
    bool MoveFlag, EnvFlag;
    uint8_t Ret;
    EnvRobotInfoRec_stru RobotInfo;

    /* */
    MoveFlag = Wheels.isMoving();
    State = (true == MoveFlag) ? INFO_STATE_TRUE : INFO_STATE_FALSE;
    SetInteriorInfo( IS_MOVING, State );

    /* */
    EnvFlag = Messager.IsEnvInfoValid();
    State = (true == EnvFlag) ? INFO_STATE_TRUE : INFO_STATE_FALSE;
    SetInteriorInfo( IS_ENVINFO, State );

    /* */
    SetInteriorInfo( IS_ACTION_MOVE, INFO_STATE_FALSE );

    /* */
    GetInteriorInfo( TARGET, &State, &RobotID );
    if ( (INFO_STATE_EMPTY != State) && (INFO_STATE_ARRIVED != State) )
    {
        MsgID = MSGID_ANY;
        Ret = Messager.GetEnvRobotInfo( RobotID, MsgID, RobotInfo );

        if ( SUCCESS != Ret )
        {
            SetInteriorInfo( TARGET, INFO_STATE_EMPTY );
        }

    }

    GetInteriorInfo( FOLLOWER, &State, &RobotID );
    if ( INFO_STATE_EMPTY != State )
    {
        MsgID = MSGID_ANY;
        Ret = Messager.GetEnvRobotInfo( RobotID, MsgID, RobotInfo );
        if ( SUCCESS != Ret )
        {
            SetInteriorInfo( FOLLOWER, INFO_STATE_EMPTY );
        }
    }
}

void Action_cls::ActionSetSendMsg( uint8_t RobotID, uint8_t MsgID )
{
    uint8_t InPara, InState;

    if ( ROBOT_ID_TARGET == RobotID )
    {
        GetInteriorInfo( TARGET, &InState, &InPara );
        ASSERT_T( INFO_STATE_EMPTY != InState );
        RobotID = InPara;
    }

    GetInteriorInfo( WEIGHT, &InState, &InPara );

    MsgInfo.RobotID = RobotID;
    MsgInfo.MsgID = MsgID;
    MsgInfo.Para = InPara;
    MsgInfo.Flag = true;

}


inline void Action_cls::ActionClrMsg( uint8_t RobotID, uint8_t MsgID )
{

    // todo Messager.ClrEnvInfoMsg( );
}

void Action_cls::ActionStatistic( uint8_t MsgID, uint8_t Para )
{
    uint8_t Num;
    uint8_t State, Logic, Threshold;
    /**  **/
    Messager.GetEnvStatistic( MsgID, Para, &Num );

    /**Set interior state**/
    Logic = 0;
    GetInteriorInfo( STATISTIC_CONDITION, &Logic, &Threshold );

    State = INFO_STATE_FALSE;


        if ( Num >= Threshold )
        {
            State = INFO_STATE_TRUE;
        }
 

    SetInteriorInfo( STATISTIC_STATE, State, Num );
}


void Action_cls::ExecuteSendMsg( )
{
    static uint8_t Cnt = 0;
    uint8_t RobotID, MsgID, Para;
    uint8_t InState, InPara;

    /**Special Msg must send after a while**/
    if ( (true == MsgInfo.Flag) && (Cnt < 3) )
    {
        RobotID = MsgInfo.RobotID;
        MsgID = MsgInfo.MsgID;
        Para = MsgInfo.Para;

        MsgInfo.Flag = false;

        Cnt++;
        //tmp todo
        if ( MSGID_SELFTYPE == MsgID )
        {
            Cnt = 0;
        }


    }
    else
    {   //send self type
        GetInteriorInfo( ROBOT_TYPE, &InState, &InPara );

        RobotID = ROBOT_ID_ANY;
        MsgID = MSGID_SELFTYPE;
        Para = InPara;

        Cnt = 0;
    }

    //tmp test todo
    if ( MSGID_NEARRANGE == MsgID )
    {
        Sensor.SendMessage( RobotID, MsgID, Para, 2 );
    }
    else
    {
        Sensor.SendMessage( RobotID, MsgID, Para );
    }
}

void Action_cls::ActionMove( uint8_t MoveType, uint8_t Para )
{
    uint8_t RobotID, State, MsgID;
    uint8_t InState;
    uint8_t Ret;
    EnvRobotInfoRec_stru RobotInfo;

    if ( MOVE_WALKAROUND == MoveType )
    {   /*Walk around*/
        GetInteriorInfo( IS_BIGSTEP, &InState );

        if ( INFO_STATE_TRUE == InState )
        {
            GetInteriorInfo( LASTMOVE, &InState );
            if (  MT_FORWARD == InState )
            {
                ExecuteMove( MT_CLOCKRANDOM, ANGLE_RANDOM );
            }
            else
            {
                ExecuteMove( MT_FORWARD, 1 );
            }
        }
        else
        {
            MoveToDirection( 0 );
        }
    }
    else if ( MOVE_TARGET == MoveType )
    {   /*Move to target*/
        GetInteriorInfo(TARGET, &State, &RobotID );
        ASSERT_T( INFO_STATE_EMPTY != State );
        MsgID = MSGID_ANY;

        Ret = Messager.GetEnvRobotInfo( RobotID, MsgID, RobotInfo );
        if ( SUCCESS == Ret )
        {
            MoveToDirection( RobotInfo.LocAngle );
        }

    }
    else if ( MOVE_SWING == MoveType )
    {
        ExecuteMove( MT_SWING, Para);
    }
    else  //MOVE_STOP, only consume time
    {
        ExecuteMove( MT_STOP, Para);;
    }

    SetInteriorInfo( IS_ACTION_MOVE, INFO_STATE_TRUE );
}



void Action_cls::MoveToDirection( int8_t Angle )
{
    static uint8_t aObstacleRule[] = { TARGET_POS_F, TARGET_POS_FL, TARGET_POS_FR, TARGET_POS_L, \
                                        TARGET_POS_R, TARGET_POS_BL, TARGET_POS_BR, TARGET_POS_B };
    static MoveRule_stru aMoveRule[] =
    {
                { MT_ANTICLOCK, MT_DEGREE(30) },//TARGET_POS_FL = IR_POSITION_FL,
                { MT_CLOCKWISE, MT_DEGREE(30) },//TARGET_POS_FR = IR_POSITION_FR,
                { MT_ANTICLOCK, MT_DEGREE(120) },//TARGET_POS_BL =IR_POSITION_BL,
                { MT_CLOCKWISE, MT_DEGREE(120) },//TARGET_POS_BR = IR_POSITION_BR,
                { MT_FORWARD, 1 },//TARGET_POS_F,
                { MT_ANTICLOCK, MT_DEGREE(90) },//TARGET_POS_L,
                { MT_CLOCKWISE, MT_DEGREE(90) },//TARGET_POS_R,
                { MT_CLOCKWISE, MT_DEGREE(180) },//TARGET_POS_B    //it must be the last
    };
    static AngleToDirection_stru aAngleToDirection[] =
    {
        {MT_DEGREE(-10), MT_DEGREE(10), TARGET_POS_F},
        {MT_DEGREE(170), MT_DEGREE(180), TARGET_POS_B},
        {MT_DEGREE(-180), MT_DEGREE(-170), TARGET_POS_B},
        {MT_DEGREE(80), MT_DEGREE(100), TARGET_POS_L},
        {MT_DEGREE(-100), MT_DEGREE(-80), TARGET_POS_R},
        {MT_DEGREE(10), MT_DEGREE(80), TARGET_POS_FL},
        {MT_DEGREE(-80), MT_DEGREE(-10), TARGET_POS_FR},
        {MT_DEGREE(100), MT_DEGREE(170), TARGET_POS_BL},
        {MT_DEGREE(-170), MT_DEGREE(-100), TARGET_POS_BR},
    };

    MTMovDir_enum MoveDirection;
    uint8_t TargetDirect;
    uint8_t Para;
    uint8_t ObstacleInfo;
    uint8_t i;

    TargetDirect = TARGET_POS_F;
    
    for ( i = 0; i < sizeof(aAngleToDirection)/sizeof(aAngleToDirection[0]); ++i )
    {
        if ( (Angle >= aAngleToDirection[i].AngleB)
                && (Angle <= aAngleToDirection[i].AngleE) )
        {
            TargetDirect = aAngleToDirection[i].Direction;
        }
    }

    /* if there is a obstacle in the 'Direction', it will avoid obstacles firstly */
    Para = INVALID;
    Messager.GetEnvObstacleInfo( &ObstacleInfo );
    
    if ( 1 == bitRead( ObstacleInfo, TargetDirect) )
    {
        TargetDirect = POSITION_INVALID;
        /*move to the direction where is not obstacle*/
        for ( i = 0; i < POSITION_NUM; ++i )
        {

            if ( 0 == bitRead( ObstacleInfo, aObstacleRule[i]) )
            {
                TargetDirect = aObstacleRule[i];
                break;
            }
        }
    }
    else
    {
        Para = (Angle >= 0 ) ? Angle : -Angle;
    }

    if ( POSITION_INVALID != TargetDirect )
    {
        MoveDirection = aMoveRule[TargetDirect].Direction;
        Para = (INVALID == Para) ? aMoveRule[TargetDirect].Para : Para;  //It is better to use the actural angle.
    }
    else
    {   //obstacle in all directions  todo: maybe decrease the power of IR LED
        MoveDirection = MT_CLOCKWISE;
        Para = MT_DEGREE(10);  ;
    }

    ExecuteMove( MoveDirection, Para);

}

void Action_cls::ExecuteMove( MTMovDir_enum MoveDirection, uint8_t MovePara )
{
    uint8_t InState, InPara;

    uint8_t MoveLastDirection;
    uint8_t MoveLastPara;

#if _DEBUG_ACTION
    Serial.print("**ExeM D=");
    Serial.print(MoveDirection);
    Serial.print(" P=");
    Serial.println(MovePara);
#endif

    Wheels.Move( MoveDirection, MovePara);

    GetInteriorInfo( LASTMOVE, &MoveLastDirection, &MoveLastPara );

    /**judge IS_BIGSTEP**/
    GetInteriorInfo( IS_BIGSTEP, &InState, &InPara );

    if ( MoveLastDirection != MoveDirection )
    {
        InPara = 0;
        InState = INFO_STATE_FALSE;
    }
    else
    {
        InPara++;
    }

    if ( InPara >= BIGSTEP_TIMES )
    {
        InPara = 0;
        InState = INFO_STATE_TRUE;
    }


    SetInteriorInfo( IS_BIGSTEP, InState, InPara);

    /** Todo: unuseful?? judge swing repeatly **/
    GetInteriorInfo( IS_REPEAT_SWING, &InState, &InPara );

    if ( (0 == InPara) || ( (MT_CLOCKWISE != MoveDirection) && (MT_ANTICLOCK != MoveDirection) ) )
    {
        InState = INFO_STATE_FALSE;
        InPara = 0;
    }
    else if ( (((MT_CLOCKWISE == MoveDirection) &&  (MT_ANTICLOCK == MoveLastDirection))
              || ((MT_ANTICLOCK == MoveDirection) &&  (MT_CLOCKWISE == MoveLastDirection)))
            && ( MovePara == MoveLastPara ) )
    {
        InState = INFO_STATE_TRUE;
        InPara = MovePara;

    }
    else
    {
        //nothing
    }

    SetInteriorInfo( IS_REPEAT_SWING, InState, InPara );

    /** keep move state  **/
    SetInteriorInfo( LASTMOVE, MoveDirection, MovePara );
}

/*here todo ???
 *Direct: the angle of target, unit: 10 degree */
inline void Action_cls::GetInteriorInfo( uint8_t InfoID, uint8_t *pState, uint8_t *pPara )
{
    *pState = InteriorInfoTbl[InfoID].State;
    if ( NULL != pPara )
    {
        *pPara = InteriorInfoTbl[InfoID].Para;
    }
}

inline void Action_cls::SetInteriorInfo( uint8_t InfoID, uint8_t State, uint8_t Para )
{
    if ( INFO_STATE_END != (InteriorInfoState_enum)State )
    {
        InteriorInfoTbl[InfoID].State = (InteriorInfoState_enum)State;
    }

    InteriorInfoTbl[InfoID].State = (InteriorInfoState_enum)State;
    if ( INVALID != Para )
    {
        InteriorInfoTbl[InfoID].Para = Para;
    }

#if 0
    Serial.print("SetInteriorInfo: I=");
    Serial.print(InfoID);
    Serial.print("S=");
    Serial.println(State);
#endif
}

#define ACTION_QUERYTIMER_T 2  //unit: 100ms

void Action_cls::QueryTimeProc( uint8_t TimeOn100ms )
{
    uint8_t &LastTime = InteriorInfoTbl[TIMER].Para;

    if ( (unsigned uint8_t)(TimeOn100ms - LastTime) < ACTION_QUERYTIMER_T )
    {
        return;
    }

    LastTime = TimeOn100ms;
    InteriorInfoTbl[TIMER].State = INFO_STATE_TRUE;

}


