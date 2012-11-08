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

#if !( 256 > EXTERIOR_ID_END )
    #error
#endif

#if !( POSITION_NUM < 8 )
    #error
#endif


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

    ClrEnvInfo();


}



void Messager_cls::MessageProc()
{
    uint8_t RetVal;
    IRPosition_enum IRLoc;
    IRMsgOutput_stru IRMsg;

    RetVal = MsgSrc.GetMessage( &IRLoc, &IRMsg );


    if ( SUCCESS != RetVal )
    {
        return;
    }

    if ( IRLoc <  IR_POSITION_CTL )
    {
        MsgToEvnInfo( IRMsg, IRLoc);
    }
    else
    {
        InfoManager.Valid = true;
    }

#if _DEBUG_MS
    Serial.print("Rec-");
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
#endif        

}



inline void Messager_cls::MsgToEvnInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc )
{

    if ( ROBOT_ID_SELF == Msg.SenderID )
    {
        bitSet(InfoManager.ObstacleInfo, IRLoc);
    }
    else
    {
        SetEnvRobotInfo( Msg, IRLoc);
    }

}

inline void Messager_cls::SetEnvRobotInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc )
{
    uint8_t SenderID, MessageID, Para;
    uint8_t &OldRec = InfoManager.RobotInfoOldRec;
    uint8_t Index;
    uint8_t i;

    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    SenderID = Msg.SenderID;
    MessageID = Msg.MessageID;
    Para = Msg.Para;

    /*to caculate TTL by received msg; (maybe it is better to using time, but critical resource).
     *because any msg must be dealed with by ACTION after a while, so the method is OK */

    pRobotInfo = InfoManager.RobotInfo;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            pRobotInfo->TTLCnt--;
        }

        if ( pRobotInfo->TTLCnt < InfoManager.RobotInfo[OldRec].TTLCnt )
        {
            OldRec = i;
        }

        pRobotInfo++;
    }


    /*todo: maybe receive many times for a msg, all the info will be combine, so the position is not accurate, expecially in rotating with large angle*/
    /*if the record has been stored and than to refresh the record*/
    pRobotInfo = InfoManager.RobotInfo;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            if ( (SenderID == pRobotInfo->RobotID) && (MessageID == pRobotInfo->MsgID) )
            {
                Index = i;
                break;
            }
        }

        pRobotInfo++;
    }

    /*if new record and than to add the record*/
    if ( ENVROBOT_INFOTBL_NUM == i )
    {
        Index = OldRec;
    }

    ASSERT_T( Index < ENVROBOT_INFOTBL_NUM );

    pRobotInfo = InfoManager.RobotInfo;
    pRobotInfo += Index;

    pRobotInfo->RobotID = SenderID;
    pRobotInfo->MsgID = MessageID;
    pRobotInfo->Para =Para;
    bitSet(pRobotInfo->Position,IRLoc);
    pRobotInfo->TTLCnt = MSG_TTL;
}



void Messager_cls::ClrEnvInfo()
{
    uint8_t i;

    InfoManager.Valid = false;
    InfoManager.RobotInfoOldRec = 0;

    InfoManager.ObstacleInfo= 0;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        InfoManager.RobotInfo[i].TTLCnt = 0;
        InfoManager.RobotInfo[i].Position = 0;
    }
}



inline void Messager_cls::RefreshEnvInfo()
{
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    RefreshPos( InfoManager.ObstacleInfo );

    pRobotInfo = InfoManager.RobotInfo;
    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            RefreshPos( pRobotInfo->Position );
        }

        pRobotInfo++;
    }

}

void Messager_cls::RefreshPos(uint8_t &Position)
{
    static PosRefreshRec_stru PosRefreshTbl[] =
    {
         // Out          InFirst           InSecond
        { TARGET_POS_F,  TARGET_POS_FL, TARGET_POS_FR },
        { TARGET_POS_B, TARGET_POS_BL, TARGET_POS_BR },
        { TARGET_POS_L,  TARGET_POS_FL, TARGET_POS_BL },
        { TARGET_POS_R,  TARGET_POS_FR, TARGET_POS_BR },
    };

    static uint8_t POSREFRESHTBL_NUM = sizeof(PosRefreshTbl)/sizeof(PosRefreshRec_stru);

    uint8_t Out, InFirst, InSecond;
    uint8_t i;
    bool BitVal;

    for ( i = 0; i < POSREFRESHTBL_NUM; ++i )
    {
        Out = PosRefreshTbl[i].Out;
        InFirst = PosRefreshTbl[i].InFirst;
        InSecond = PosRefreshTbl[i].InSecond;

        BitVal = bitRead(Position, InFirst) && bitRead(Position, InSecond);
        bitWrite(Position, Out, BitVal);

#if 0
        Serial.print(" Ref i=");
        Serial.print(i);
        Serial.print(" BitVal");
        Serial.print(BitVal);
        Serial.print(" Position");
        Serial.println(Position,BIN);
#endif
    }
}

inline void Messager_cls::AgingProc()
{
    //todo ? maybe no use

}

void Messager_cls::TimerProc()
{
    //todo ? need?
}

bool Messager_cls::IsEnvInfoValid( )
{
    return InfoManager.Valid;
}

/*If there are many records meeting the condition, it will return the record than its Para is the most.
 * If the Paras are same, it will return the newest record*/
uint8_t Messager_cls::GetEnvRobotInfo( uint8_t *pRobotID, uint8_t *pMsgID, uint8_t *pPara, uint8_t *pPosition )
{
    uint8_t i;
    uint8_t RobotID, MsgID, Para, Index;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    RefreshEnvInfo();
#if _DEBUG_MS
    ShowEnvRobotInfo();
#endif

    pRobotInfo = InfoManager.RobotInfo;
    Index = INVALID;
    RobotID = *pRobotID;
    MsgID = *pMsgID;
    Para = 0;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            if ( (pRobotInfo->RobotID == RobotID) &&  (pRobotInfo->MsgID == MsgID) )
            {
                Index = i;
                break;

            }
            else if ( (ROBOT_ID_ANY == RobotID) && (pRobotInfo->MsgID == MsgID) )
            {             
              /* If there are many records meeting the condition, it will return the record than its Para is the most.
                 * If the Paras are same, it will return any record*/
                if ( Para <= pRobotInfo->Para )
                {
                    Index = i;
                    Para = pRobotInfo->Para;
                }
            }
            else if ( (RobotID == pRobotInfo->RobotID) && (MSGID_ANY == MsgID) )
            {
                Index = i;
                break;
            }
            else
            {
                ;  //nothing
            }

        }

        pRobotInfo++;
    }

    if ( INVALID == Index )
    {
        return INFO_QUE_FULL;
    }
    
    pRobotInfo = InfoManager.RobotInfo + Index;

    *pRobotID = pRobotInfo->RobotID;
    *pMsgID = pRobotInfo->MsgID;

    if ( NULL != pPara )
    {
        *pPara = pRobotInfo->Para;
    }

    if ( NULL != pPosition )
    {
        *pPosition = pRobotInfo->Position;
    }

    return SUCCESS;
}

void Messager_cls::GetEnvObstacleInfo( uint8_t *pInfo )
{
    *pInfo = InfoManager.ObstacleInfo;
}


#if _DEBUG_MS
void Messager_cls::ShowEnvRobotInfo()
{
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;


    pRobotInfo = InfoManager.RobotInfo;

    Serial.println("RobotInfo:");

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {

            Serial.print("RobotInfo-");
            Serial.print(i);
            Serial.print(":RobotID=");
            Serial.print( pRobotInfo->RobotID );
            Serial.print(" Msg=");
            Serial.print( pRobotInfo->MsgID );
            Serial.print(" Para=");
            Serial.print( pRobotInfo->Para );
            Serial.print(" Pos=");
            Serial.print( pRobotInfo->Position, BIN );
            Serial.print(" TTL=");
            Serial.println( pRobotInfo->TTLCnt );
        }

        pRobotInfo++;
    }


}
#endif





#if 0

/*table constraint:
 * the first uint8_t colum must be used to identify the validation of record.  NUll means empty.
 * the following colums must be KEY,
 * */
typedef struct TblCtrl
{
    uint8_t MaxRec;     //
    uint8_t RecLen;     //the lenth of a record, uint:byte
    uint8_t KeyByteNum;    // the byte numbers of the key, uint:byte
    uint8_t EmptyIndex;  // an empty row that can be availible right now.
}TblCtrl_stru;

#define MS_MESSAGETBL_NUM 16
#define MS_MESSAGETBL_KEYBYTENUM 2



tbl_SetRec( TblCtrl_stru &MsgTblCtrl, (uint8_t *)Record )
{
    //
}

tbl_ClrRec( TblCtrl_stru &MsgTblCtrl, uint8_t Index )
{
    //
}

#endif
