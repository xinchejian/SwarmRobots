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
#include "SwarmRobot.h"
#include "Messager.h"

/*checking constrain*/
#if !((EXTERIOR_ID_END - EXTERIOR_ID_BEGIN) == EXTERIOR_ID_END)
    #error
#endif

#if !( 256 > EXTERIOR_ID_END )
    #error
#endif



/*public*/
MsgToInfoRec_stru Messager_cls::MsgToInfoRule[] =
{
    //&SenderID,    &MessageID, SensorIndex;       condition)
    { RX_MSG_OK, ROBOT_ID_SELF,  MSG_ANY, IR_POSITION_FL, OBSTACLE_FRONT_LEFT, COND_TRUE },
    { RX_MSG_OK, ROBOT_ID_SELF,  MSG_ANY, IR_POSITION_FR, OBSTACLE_FRONT_RIGHT, COND_TRUE },
    { RX_MSG_OK, ROBOT_ID_SELF,  MSG_ANY, IR_POSITION_BL, OBSTACLE_BEHIND_LEFT, COND_TRUE },
    { RX_MSG_OK, ROBOT_ID_SELF,  MSG_ANY, IR_POSITION_BR, OBSTACLE_BEHIND_RIGHT, COND_TRUE },
    { RX_UNOBSTACLE, ROBOT_ID_ANY,  MSG_ANY, IR_POSITION_FL, OBSTACLE_FRONT_LEFT, COND_FALSE },
    { RX_UNOBSTACLE, ROBOT_ID_ANY,  MSG_ANY, IR_POSITION_FR, OBSTACLE_FRONT_RIGHT, COND_FALSE },
    { RX_UNOBSTACLE, ROBOT_ID_ANY,  MSG_ANY, IR_POSITION_BL, OBSTACLE_BEHIND_LEFT, COND_FALSE },
    { RX_UNOBSTACLE, ROBOT_ID_ANY,  MSG_ANY, IR_POSITION_BR, OBSTACLE_BEHIND_RIGHT, COND_FALSE }
};



Messager_cls::Messager_cls(IR_Sensor &MySensor):MsgSrc(MySensor)
{
    uint8_t i;

    /* todo
    MsgTblCtrl.MaxRec = MS_MESSAGETBL_NUM;
    MsgTblCtrl.RecLen = sizeof( MessageTable_stru );
    MsgTblCtrl.KeyByteNum = MS_MESSAGETBL_KEYBYTENUM;
    MsgTblCtrl.EmptyIndex = 0;

    for ( i = 0; i < MS_MESSAGETBL_NUM; ++i )
    {
        MessageTbl[i].TTLCnt = NULL;
    }
    */

    ClrExteriorInfo();


}

bool Messager_cls::IsExteriorInfoValid( )
{
    return InfoManager.Valid;
}

void Messager_cls::GetExteriorInfo( ExteriorInfoID_enum CondID, uint8_t *pState )
{
    //todo
    *pState = InfoManager.Tbl[CondID].State;
}

void Messager_cls::MessageProc()
{
    uint8_t RetVal;
    uint8_t i;
    uint8_t SenderID, MessageID, Para, MsgState;
    bool NewMsgFlag;

    MessageRec_stru MsgRec;

    NewMsgFlag = false;

    for ( i =0; i < IR_RECIEVER_NUM; ++i )
    {
        RetVal = MsgSrc.GetMessage( i, &SenderID, &MessageID, &Para, &MsgState );


        if ( SUCCESS != RetVal )
        {
            NewMsgFlag = false;
            break;  //in order to get all receiver's info
        }

        if ( (RX_UNOBSTACLE == MsgState) || (RX_MSG_OK == MsgState) )
        {
            //todo
            MsgRec.State = MsgState;
            MsgRec.SenderID = SenderID;
            MsgRec.MessageID = MessageID;
            MsgRec.Para = Para;

            MsgToExteriorInfo( MsgRec, (IRPosition_enum)i);
            NewMsgFlag = true;
        }
#if _DEBUG_MS
        Serial.print("Rec-");
        Serial.print(i);
        Serial.print(" ret=");
        Serial.print(RetVal);
        Serial.print(":S=");
        Serial.print(MsgState, HEX);
        Serial.print(":F=");
        Serial.print(SenderID, HEX);
        Serial.print(" M=");
        Serial.print(MessageID, HEX);
        Serial.print(" P=");
        Serial.println(Para, HEX);
#endif        

    }

    if ( true == NewMsgFlag )
    {
        RefreshExteriorInfo();
        InfoManager.Valid = true;
    }

    //todo:  在没有障碍物的情况下是收不到消息的， 因此信息的刷新要改进
}



inline void Messager_cls::MsgToExteriorInfo(MessageRec_stru &Msg, IRPosition_enum IRLoc)
{
    static const uint8_t MsgToCondTblNum = sizeof( MsgToInfoRule )/sizeof( MsgToInfoRec_stru );
    uint8_t i;
    MsgToInfoRec_stru *pMsgTocond = NULL;

    for ( i=0; i < MsgToCondTblNum ; ++i )
    {
        pMsgTocond = &MsgToInfoRule[i];
        if ( (IRLoc == pMsgTocond->IRPos)
                && ( Msg.State == pMsgTocond->MsgState )
            && ( ( ROBOT_ID_ANY == pMsgTocond->SenderID ) || (Msg.SenderID == pMsgTocond->SenderID) )
            && ( (MSG_ANY == pMsgTocond->MessageID) || (Msg.MessageID == pMsgTocond->MessageID)) )
        {
            SetExteriorInfo( pMsgTocond->CondID, pMsgTocond->CondState );
        }
    }
}

inline void Messager_cls::SetExteriorInfo(ExteriorInfoID_enum CondID, ExteriorInfoState_enum State)
{
    InfoManager.Tbl[CondID].State = State;
}

void Messager_cls::ClrExteriorInfo()
{
    uint8_t i;
    //todo data collide
    InfoManager.Valid = false;
    for ( i = (EXTERIOR_ID_BEGIN + 1); i < MS_CONDITIONTBL_NUM; ++i )
    {
        InfoManager.Tbl[i].State = COND_UNKNOWN;
    }
}



inline void Messager_cls::RefreshExteriorInfo()
{
    //todo: optimize later,  to caculate indirect conditions from primary conditions

    //ExteriorInfoRec_stru (&CondTbl)[MS_CONDITIONTBL_NUM] = InfoManager.Tbl;

}



inline void Messager_cls::AgingProc()
{
    //todo ? maybe no use

}

void Messager_cls::TimerProc()
{
    //todo ? need?
}

#if 0
tbl_SetRec( TblCtrl_stru &MsgTblCtrl, (uint8_t *)Record )
{
    //
}

tbl_ClrRec( TblCtrl_stru &MsgTblCtrl, uint8_t Index )
{
    //
}
#endif
