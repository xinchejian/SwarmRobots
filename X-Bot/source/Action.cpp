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
#include "Action.h"

/* Rules are consisted with blocks.  The 'INTERIORINFO, TASK_STAGE' is the first sentence of each block.
 * Only one block is executed each time
 * If the condition is false, it will shift to the next rule automatically.
 */


#if ( ROBOT_TYPE_ANT == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_LOOKING,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
    {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_FOOD,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_MOVINGFOOD,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

    {INTERIORINFO, TASK_STAGE, TS_FOUND,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_SET, COUNTER, 0,  true},
    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_SET, TASK_STAGE, TS_NEAR,  false},
    {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },


    {INTERIORINFO, TASK_STAGE, TS_NEAR,  0, 0, 0,  true},
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

RuleRec_stru Action_cls::PreRuleTbl[] = {
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false },

    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false},
    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},
};

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
    {INTERIORINFO, TASK_STAGE, TS_Lonely,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
    {ROBOTINFO_MSG, ROBOT_ID_ANY, MSGID_ACK,  ACTION_IN_SET, FOLLOWER, INFO_STATE_SET, true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeFollowed,  false},

    {ROBOTINFO_MSG, ROBOT_ID_ANY, MSGID_RESPONCE, ACTION_IN_SET, TARGET, INFO_STATE_SET, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET, ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_ACK, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET, ACTION_IN_SET, TASK_STAGE, TS_Following,  false},

    {ROBOTINFO_MSG_LESSID, ROBOT_ID_ANY, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_REQUEST,  false},
//
    {INTERIORINFO, TASK_STAGE, TS_BeFollowed,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {ROBOTINFO_MSG, ROBOT_ID_ANY, MSGID_RESPONCE,  ACTION_IN_SET, TARGET, INFO_STATE_SET, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_ACK, true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeNode,  false},

    {ROBOTINFO_MSG, ROBOT_ID_FOLLOWER, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_ANY, MSGID_REQUEST,  true},
    {ROBOTINFO_NEAR, ROBOT_ID_FOLLOWER, false,  ACTION_EXT_MOVE, MOVE_SWING, 3, false },

    {ROBOTINFO_NEAR, ROBOT_ID_FOLLOWER, true,  ACTION_IN_SET, COUNTER, 0,  true},
    {INTERIORINFO, COUNTER, 15,  ACTION_EXT_MOVE, MOVE_SWING, 3,  false},  //if next is not Game Over, shout set the state to false
    {ROBOTINFO_NEAR, ROBOT_ID_FOLLOWER, false,  ACTION_IN_INCR, COUNTER, 0,  false},

//
    {INTERIORINFO, TASK_STAGE, TS_Following,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B10,  true},
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {ROBOTINFO_MSG, ROBOT_ID_ANY, MSGID_ACK,  ACTION_IN_SET, FOLLOWER, INFO_STATE_SET, true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_BeNode,  false},

    {ROBOTINFO_MSG_LESSID, ROBOT_ID_ANY, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_ACK,  true},

    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, COUNTER, 0,  true},
    {INTERIORINFO, COUNTER, 6,  ACTION_EXT_MOVE, MOVE_SWING, 3,  false},  //if next is not Game Over, shout set the state to false
    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_INCR, COUNTER, 0,  true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },

//
    {INTERIORINFO, TASK_STAGE, TS_BeNode,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
    {INTERIORINFO, FOLLOWER, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Following,  false },
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_BeFollowed,  false },

    {ROBOTINFO_MSG, ROBOT_ID_FOLLOWER, MSGID_REQUEST, ACTION_EXT_SENDMSG, ROBOT_ID_SOURCE, MSGID_RESPONCE, false},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_SENDMSG, ROBOT_ID_TARGET, MSGID_ACK,  true},

    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, COUNTER, 0,  true},
    {INTERIORINFO, COUNTER, 6,  ACTION_EXT_MOVE, MOVE_SWING, 3,  false},  //if next is not Game Over, shout set the state to false
    {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_INCR, COUNTER, 0,  true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_EXT_MOVE, MOVE_TARGET, 0,  false },
};



RuleRec_stru Action_cls::TimerRuleTbl[] ={};

RuleRec_stru Action_cls::PreRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },
};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};



#elif ( ROBOT_TYPE_ANT_SAMEDIRECTION == ROBOT_INIT_TYPE )
RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_Lonely,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
    {ROBOTINFO_MSG_LESSID, ROBOT_ID_ANY, MSGID_ANY,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
//    {ROBOTINFO_TASKSTATE, TS_Lonely, false,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

    {INTERIORINFO, TASK_STAGE, TS_FOUND,  0, 0, 0,  true},
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },
    {ANYINFO, 0, 0,  ACTION_IN_INCR, COUNTER, 0,  true},
    {INTERIORINFO, COUNTER, 8,  ACTION_EXT_MOVE, MOVE_KEEPANGLE_VECTOR, MT_DEGREE(0),  true},
    {INTERIORINFO, COUNTER, 8,  ACTION_IN_SET, COUNTER, 0,  false},
};

RuleRec_stru Action_cls::TimerRuleTbl[] ={};

RuleRec_stru Action_cls::PreRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {ANYINFO, 0, 0,  ACTION_EXT_SENDMSG, ROBOT_ID_POLLING, MSGID_NOTIFY_IFINDYOU, false },
};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_STOP, 1, false },
};


#elif ( ROBOT_TYPE_FOOD == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_INIT,   0, 0, 0, true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, STATISTIC_CONDITION, CONDITION_LARGER, true},
    {ANYINFO, 0, 0,  ACTION_IN_SET_PARA, STATISTIC_CONDITION, 1, true},   //set the arrived quantity.
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_RUNNING,  false},

    {INTERIORINFO, TASK_STAGE, TS_RUNNING,   0, 0, 0, true },
    {ANYINFO, 0, 0, ACTION_EXT_STATISTIC, MSGID_NOTIFY_DONE, 0, true},
    {INTERIORINFO, STATISTIC_STATE, INFO_STATE_TRUE,  ACTION_IN_SET, TASK_STAGE, TS_TASKOVER, false },

    {INTERIORINFO, TASK_STAGE, TS_TASKOVER,   0, 0, 0, true },
    {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_GAMEWIN, 0,  true},
    {ANYINFO, 0, 0,   ACTION_EXT_MOVE, MOVE_SWING, 5, false},
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
    {ANYINFO, 0, 0,  ACTION_IN_INCR, COUNTER, 0,  true},
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_STOP, 0, false },  //delay time
};

#elif ( ROBOT_TYPE_SUN == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
};


RuleRec_stru Action_cls::PreRuleTbl[] = {};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_STOP, 0, false },  //delay time
};

#elif ( ROBOT_TYPE_EARTH == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
        {INTERIORINFO, TASK_STAGE, TS_LOOKING,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
        {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_SUN,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
        {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

        {INTERIORINFO, TASK_STAGE, TS_FOUND,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
 //       {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_EXT_MOVE, MOVE_KEEPANGLE_POINT, MT_DEGREE(40), true},
        {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_SET, TASK_STAGE, TS_NEAR,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(20),  false },


        {INTERIORINFO, TASK_STAGE, TS_NEAR,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
 //       {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_EXT_MOVE, MOVE_KEEPANGLE_POINT, MT_DEGREE(40), true},
        {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(80),  false },

};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
};


RuleRec_stru Action_cls::PreRuleTbl[] = {
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false },

    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false},

};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};

#elif ( ROBOT_TYPE_MOON == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
        {INTERIORINFO, TASK_STAGE, TS_LOOKING,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
        {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_EARTH,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
        {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},

        {INTERIORINFO, TASK_STAGE, TS_FOUND,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
 //       {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_EXT_MOVE, MOVE_KEEPANGLE_POINT, MT_DEGREE(40), true},
        {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_IN_SET, TASK_STAGE, TS_NEAR,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(20),  false },


        {INTERIORINFO, TASK_STAGE, TS_NEAR,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
 //       {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_EXT_MOVE, MOVE_KEEPANGLE_POINT, MT_DEGREE(40), true},
        {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, TASK_STAGE, TS_FOUND,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(80),  false },

};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
};


RuleRec_stru Action_cls::PreRuleTbl[] = {
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false },

    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_LOOKING,  false},

};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};


#elif ( ROBOT_TYPE_GROUPS == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
        {INTERIORINFO, TASK_STAGE, TS_Lonely,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B00,  true},
        {ROBOTINFO_GRP, GRP_SAME, true,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
        {INTERIORINFO, TARGET, INFO_STATE_SET,  ACTION_IN_SET, TASK_STAGE, TASK_Safe,  false},

        {INTERIORINFO, TASK_STAGE, TASK_Danger,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B01,  true},
 //       {ROBOTINFO_NEAR, ROBOT_ID_TARGET, false,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(180),  false },

        {INTERIORINFO, TASK_STAGE, TASK_Safe,  0, 0, 0,  true},
        {ANYINFO, 0, 0,  ACTION_EXT_LED, LED_MODE_MANUAL, 0B11,  true},
        {ROBOTINFO_NEAR, ROBOT_ID_TARGET, true,  ACTION_EXT_MOVE, MOVE_STOP, 1,  false},
        {ANYINFO, 0, 0,  ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(0),  false },

};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
};


RuleRec_stru Action_cls::PreRuleTbl[] = {
    {INTERIORINFO, TASK_STAGE, TS_INIT,  0, 0, 0,  true },
    {ANYINFO, 0, 0,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false },

    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {ROBOTINFO_GRP, GRP_SAME, false,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {ROBOTINFO_GRP, GRP_SAME, false,  ACTION_IN_SET, TASK_STAGE, TASK_Danger,  false},
    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false},

//    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
//    {ROBOTINFO_GRP, GRP_SAME, false,  ACTION_IN_SET, FOLLOWER, INFO_STATE_SET,  true},
//    {ROBOTINFO_NEAR, ROBOT_ID_FOLLOWER, true,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
//    {ROBOTINFO_NEAR, ROBOT_ID_FOLLOWER, true,  ACTION_IN_SET, TASK_STAGE, TASK_Danger,  false},
//    {INTERIORINFO, TARGET, INFO_STATE_EMPTY,  ACTION_IN_SET, TASK_STAGE, TS_Lonely,  false},


};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },
};

#elif ( ROBOT_TYPE_MOVINGFOOD == ROBOT_INIT_TYPE )

RuleRec_stru Action_cls::RuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,   0, 0, 0, true },
    {ROBOTINFO_TYPE, ROBOT_ID_ANY, ROBOT_TYPE_ANT,  ACTION_IN_SET, TARGET, INFO_STATE_SET,  true},
    {INTERIORINFO, TARGET, INFO_STATE_SET, ACTION_EXT_MOVE, MOVE_TARGET, MT_DEGREE(180),  false},
};

RuleRec_stru Action_cls::TimerRuleTbl[] =
{
};


RuleRec_stru Action_cls::PreRuleTbl[] = {};

RuleRec_stru Action_cls::PostRuleTbl[] =
{
    {INTERIORINFO, TASK_STAGE, TS_ANY,  0, 0, 0, true },
    {INTERIORINFO, IS_ACTION_MOVE, INFO_STATE_FALSE,  ACTION_EXT_MOVE, MOVE_WALKAROUND, 0, false },  //delay time
};

#endif


Action_cls::Action_cls(IR_Sensor &Sensor, BiMotor &Wheels, Messager_cls &Messager): Sensor(Sensor), Wheels(Wheels), Messager(Messager)
{

    InteriorInfoTbl[ROBOT_TYPE].Para = ROBOT_INIT_TYPE;
    InteriorInfoTbl[ROBOT_TYPE].State = INFO_STATE_TRUE;

    InteriorInfoTbl[TASK_STAGE].State = TS_INIT;
    InteriorInfoTbl[IS_ACTION_MOVE].State = INFO_STATE_FALSE;

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
#if 0
Serial.print(" Movstate");
Serial.print(MovingState);
Serial.print(" Envstate");
Serial.println(EnvInfoState);
#endif
    //todo, is it necessary "(INFO_STATE_FALSE == MovingState)"
    if ( (INFO_STATE_FALSE == MovingState) && (INFO_STATE_FALSE == EnvInfoState) )
    {
        ExecuteSendMsg();
    }

    if ( 0 < PreRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("B ");
#endif
        ActionRule( PreRuleTbl, PreRuleNum );
    }

    if ( 0 < TimerRuleNum )
    {
#if _DEBUG_ACTION
Serial.println("T ");
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
        RobotID = ROBOT_ID_ANY;

        InfoFlag = false;
        InfoID = pTbl[i].InfoID;
        InfoPara = pTbl[i].InfoPara;

        if ( ROBOTINFO_END > pTbl[i].InfoType )
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

            Ret = Messager.GetEnvRobotInfo( pTbl[i].InfoType, RobotID, InfoPara, RobotInfo);
#if 0
            Serial.print(" GetEnv ret=");
            Serial.println(Ret);
#endif
            if ( SUCCESS == Ret)
            {
                RobotID = RobotInfo.RobotID;
                InfoFlag = true;
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
                if ( (INFO_STATE_SET == ActParaB) && ( ROBOT_ID_ANY != RobotID ) )
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
                else if ( ROBOT_ID_FOLLOWER == ActParaA )
                {
                    /*If there is no target, the rule is out of date, so stop*/
                    GetInteriorInfo( FOLLOWER, &InState, &InPara );
                    if ( INFO_STATE_EMPTY == InState )
                    {
                        break;
                    }
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
        Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, RobotID, MsgID, RobotInfo );

        if ( SUCCESS != Ret )
        {
            SetInteriorInfo( TARGET, INFO_STATE_EMPTY );
        }

    }

    GetInteriorInfo( FOLLOWER, &State, &RobotID );
    if ( INFO_STATE_EMPTY != State )
    {
        MsgID = MSGID_ANY;
        Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, RobotID, MsgID, RobotInfo );
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
    static bool SpecialFlag = false;
    uint8_t RobotID, MsgID, Para, PowerTenth;
    uint8_t InState, InPara;
    EnvRobotInfoRec_stru RobotInfo;
    uint8_t Ret;


    /**Special Msg must send after a while**/
    if ( (true == MsgInfo.Flag) && (Cnt < 3) )
    {
        RobotID = MsgInfo.RobotID;
        MsgID = MsgInfo.MsgID;
        PowerTenth = 10;
        Para = INVALID;

        if ( ROBOT_ID_POLLING == RobotID )
        {   //todo  optimize
            Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, ROBOT_ID_ANY, MSGID_ANY, RobotInfo );
            if ( SUCCESS == Ret )
            {
                RobotID = RobotInfo.RobotID;
                Para = RobotInfo.LocAngle;
            }
            else
            {
                MsgInfo.Flag = false;
                Cnt++;
                return;
            }
        }
        else if ( RobotID < ROBOT_ID_SPECIAL_START )
        {
            Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, RobotID, MSGID_ANY, RobotInfo );

            if ( SUCCESS == Ret )
            {
                Para = RobotInfo.LocAngle;
            }
        }

        MsgInfo.Flag = false;
        Cnt++;

    }
    else
    {
        if ( SpecialFlag )
        {
            GetInteriorInfo( ROBOT_TYPE, &InState, &InPara );

            RobotID = ROBOT_ID_ANY;
            MsgID = MSGID_SELFTYPE;
            Para = InPara;
            PowerTenth = 10;
        }
        else
        {
            RobotID = ROBOT_ID_ANY;
            MsgID = MSGID_NEARRANGE;
            Para = 0;
            PowerTenth = IR_POWER_NEARTTEHTH;
        }

        SpecialFlag = !SpecialFlag;
        Cnt = 0;
    }




    Sensor.SendMessage( RobotID, MsgID, Para, PowerTenth );
}

void Action_cls::ActionMove( uint8_t MoveType, uint8_t Para )
{
    uint8_t RobotID, State;
    MTMovDir_enum MTDir;
    uint8_t Angle;

    if ( MOVE_WALKAROUND == MoveType )
    {
        if ( true == Wheels.isStepRepeat() )
        {
            randomSeed(millis());
            MTDir = (0 == random(2)) ? MT_CLOCKWISE : MT_ANTICLOCK;
            Angle = random(MT_DEGREE(60));

            ExecuteMove( MTDir, Angle );
        }
        else if ( true == Wheels.isRotateRepeat() )
        {
            ExecuteMove( MT_FORWARD, 3 );
        }
        else
        {
            MoveAvoidObstacle();
        }
    }
    else if ( MOVE_TARGET == MoveType )
    {   /*Move to target*/
        GetInteriorInfo(TARGET, &State, &RobotID );
        ASSERT_T( INFO_STATE_EMPTY != State );

        MoveToTarget( RobotID, Para );
    }
    else if ( MOVE_SWING == MoveType )
    {
        MoveWithSwing( Para );
    }
    else if ( MOVE_KEEPANGLE_VECTOR == MoveType )
    {
        GetInteriorInfo(TARGET, &State, &RobotID );
        ASSERT_T( INFO_STATE_EMPTY != State );
        MoveKeepAngle( RobotID, Para );
    }
    else if ( MOVE_KEEPANGLE_POINT == MoveType )
    {
        GetInteriorInfo(TARGET, &State, &RobotID );
        ASSERT_T( INFO_STATE_EMPTY != State );
        MoveKeepPointAngle( RobotID, Para );
    }
    else if ( MOVE_MANUAL == MoveType )
    {
        ExecuteMove( (MTMovDir_enum)Para, 1 );
    }
    else  //MOVE_STOP, only consume time
    {
        ExecuteMove( MT_STOP, Para );
    }

    SetInteriorInfo( IS_ACTION_MOVE, INFO_STATE_TRUE );
}



void Action_cls::MoveWithSwing( uint8_t Para )
{

    MTMovDir_enum LastMoveType, MoveType;
    uint8_t Val;

    /**when firstly rotate, the angle is half of setting.**/
    Wheels.GetMoveAciton( LastMoveType, Val );
    if ( (MT_CLOCKWISE != LastMoveType)
            && (MT_ANTICLOCK != LastMoveType) )
    {
        Para = Para>>1;
    }

    MoveType = ( MT_CLOCKWISE == LastMoveType ) ? MT_ANTICLOCK : MT_CLOCKWISE;

    ExecuteMove( MoveType, Para, false );

}

void Action_cls::MoveKeepAngle( uint8_t TargetID, uint8_t Angle)
{
    MTMovDir_enum MoveType;
    uint8_t MoveAngle;
    uint8_t Ret;

    EnvRobotInfoRec_stru RobotInfo;

    if ( Angle >= MT_DEGREE(360) )
    {
        ASSERT_T(0);
        return;
    }

    Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, TargetID, MSGID_ANY, RobotInfo );
    if ( SUCCESS != Ret )
    {
        ASSERT_T(0);
        return;
    }

    if ( INVALID == RobotInfo.FaceAngle )
    {
        return;
    }

    MoveAngle = RobotInfo.FaceAngle + Angle;
    MoveAngle = MoveAngle % MT_DEGREE(360);


    if ( MoveAngle  > MT_DEGREE(180) )
    {
        MoveType = MT_CLOCKWISE;
        MoveAngle =  (MT_DEGREE(360) - MoveAngle);
    }
    else
    {
        MoveType = MT_ANTICLOCK;
    }
    
    if ( MoveAngle <= MT_DEGREE(20) )
    {
        MoveType = MT_STOP;
        MoveAngle = 1; //become step
    }
    else
    {
        //MoveAngle/= 2; //tmp
    }

    ExecuteMove( MoveType, MoveAngle, false );
}

/* todo:   there are same codes between MoveKeepAngle and MoveKeepPointAngle
 *
 */
void Action_cls::MoveKeepPointAngle( uint8_t TargetID, uint8_t Angle)
{
    MTMovDir_enum MoveType;
    uint8_t MoveAngle;
    uint8_t Ret;

    EnvRobotInfoRec_stru RobotInfo;

    if ( Angle >= MT_DEGREE(360) )
    {
        ASSERT_T(0);
        return;
    }

    Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, TargetID, MSGID_ANY, RobotInfo );
    if ( SUCCESS != Ret )
    {
        ASSERT_T(0);
        return;
    }

    if ( INVALID == RobotInfo.LocAngle )
    {
        return;
    }

    MoveAngle = (RobotInfo.LocAngle + MT_DEGREE(360)) - Angle;
    MoveAngle = MoveAngle % MT_DEGREE(360);


    if ( MoveAngle  > MT_DEGREE(180) )
    {
        MoveType = MT_CLOCKWISE;
        MoveAngle =  (MT_DEGREE(360) - MoveAngle);
    }
    else
    {
        MoveType = MT_ANTICLOCK;
    }

    if ( MoveAngle <= MT_DEGREE(20) )
    {
        MoveType = MT_STOP;
        MoveAngle = 1; //become step
    }
    else
    {
        //MoveAngle/= 2; //tmp
    }

#if _DEBUG_ACTION
    Serial.print("-PA LOC=");
    Serial.print(RobotInfo.LocAngle);
    Serial.print(" MT=");
    Serial.print(MoveType);
    Serial.print(" A=");
    Serial.println(MoveAngle);
#endif

    ExecuteMove( MoveType, MoveAngle, false );
}

void Action_cls::MoveToTarget( uint8_t TagetRobotID, uint8_t CrossAngle )
{
    EnvRobotInfoRec_stru RobotInfo;
    uint8_t ObstacleInfo;
    TargetPos_enum TargetDirect;
    MTMovDir_enum MoveDirection;
    uint8_t NearRobotID;
    uint8_t Para;
    int8_t Angle;
    uint8_t Ret;

    Ret = Messager.GetEnvRobotInfo( ROBOTINFO_MSG, TagetRobotID, MSGID_ANY, RobotInfo );
    if ( SUCCESS != Ret )
    {
        ASSERT_T(0);
        return;
    }

    /* if there is a obstacle in the 'Direction', it will avoid obstacles firstly */
    Angle = (RobotInfo.LocAngle + MT_DEGREE(360)) - CrossAngle;
    Angle = Angle % MT_DEGREE(360);


    TargetDirect = Messager.GetLocationfromAngle( Angle );

    NearRobotID = Messager.GetEnvNearRobot(TargetDirect);
    Messager.GetEnvObstacleInfo( &ObstacleInfo );

    if ( (( ROBOT_ID_NONE == NearRobotID )
            || (NearRobotID == TagetRobotID))
            && (0 == bitRead( ObstacleInfo, TargetDirect)) )
    {
        GetMTActionFromPosition( TargetDirect, MoveDirection, Para );
        if ( MT_FORWARD == MoveDirection )
        {
            Para = 1;
        }
        else
        {
            Para = (Angle > MT_DEGREE(180) ) ? (MT_DEGREE(360) - Angle) : Angle;
        }

        ExecuteMove( MoveDirection, Para );
    }
    else
    {
        MoveAvoidObstacle();
    }
}


void Action_cls::MoveAvoidObstacle()
{
    static TargetPos_enum aObstacleRule[] = { TARGET_POS_F, TARGET_POS_FL, TARGET_POS_FR, TARGET_POS_L, \
                                        TARGET_POS_R, TARGET_POS_BL, TARGET_POS_BR, TARGET_POS_B };

    uint8_t i;
    uint8_t Val,ObstacleInfo;
    TargetPos_enum TargetDirect;
    MTMovDir_enum MoveDirection;
    uint8_t Para;

    /*move to the direction where is not obstacle*/
    Messager.GetEnvObstacleInfo( &Val );
    ObstacleInfo = Val;
    Messager.GetRobotAsObstacleInfo( &Val );
    ObstacleInfo |= Val;

    for ( i = 0; i < POSITION_NUM; ++i )
    {
        if ( 0 == bitRead( ObstacleInfo, aObstacleRule[i]) )
        {
            TargetDirect = aObstacleRule[i];
            break;
        }
    }

    if ( POSITION_NUM > i )
    {
        GetMTActionFromPosition( TargetDirect, MoveDirection, Para );
    }
    else
    {   //obstacle in all directions  todo: maybe decrease the power of IR LED
        MoveDirection = MT_CLOCKWISE;
        Para = MT_DEGREE(10);  ;
    }

    ExecuteMove( MoveDirection, Para);
}

inline void Action_cls::GetMTActionFromPosition( TargetPos_enum Position, MTMovDir_enum &MTDirection, uint8_t &Para )
{
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

    ASSERT_T( Position < POSITION_NUM );


    MTDirection = aMoveRule[Position].Direction;
    Para = aMoveRule[Position].Para;
}



inline void Action_cls::ExecuteMove( MTMovDir_enum MoveDirection, uint8_t MovePara, bool PreventPingPang )
{
#if 0
    static uint8_t Angle = 0;
    if ( PreventPingPang && Wheels.isRotatePingPang() )
    {
        if ( (MT_CLOCKWISE == MoveDirection) || (MT_ANTICLOCK == MoveDirection) )
        {
            Angle = Angle/2;
            if ( Angle < MT_DEGREE(10) )
            {
                MoveDirection = MT_FORWARD;
                MovePara = 1;
            }
            else
            {
                MovePara = Angle;
            }
        }
    }
    else
    {
        Angle = MovePara;
    }
#endif

    Wheels.Move( MoveDirection, MovePara);
}

/*here todo ???
 *Direct: the angle of target, unit: 10 degrees */
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

    if ( (uint8_t)(TimeOn100ms - LastTime) < ACTION_QUERYTIMER_T )
    {
        return;
    }

    LastTime = TimeOn100ms;
    InteriorInfoTbl[TIMER].State = INFO_STATE_TRUE;

}


