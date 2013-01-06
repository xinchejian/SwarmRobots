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
#include "Motor.h"
#include "Messager.h"

/*checking constrain*/

#if !( 256 > EXTERIOR_ID_END )
    #error
#endif

#if !( POSITION_NUM < 8 )
    #error
#endif


Messager_cls::Messager_cls(IR_Sensor &MySensor, BiMotor &Wheels):MsgSrc(MySensor),Wheels(Wheels)
{
    uint8_t i;

    InfoManager.RobotInfoOldRec = 0;
    InfoManager.SearchIndex = 0;
    
    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        ClrEnvInfoMsg(&(InfoManager.RobotInfo[i]));
    }   
}



void Messager_cls::MessageProc()
{
    uint8_t RetVal;
    IRPosition_enum IRLoc;
    IRMsgOutput_stru IRMsg;

    QueryTimeProc(GetTimeOn100ms());

    CalibrateEnvInfo();

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
        RefreshEnvInfo();
        InfoManager.Valid = true;
    }

#if 0

    if ( ROBOT_ID_SELF != IRMsg.SenderID && IRLoc < IR_RECIEVER_NUM )
    {
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
    }
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

    /*todo: maybe receive many times for a msg, all the info will be combine, so the position is not accurate, expecially in rotating with large angle*/
    /*if the record has been stored and than to refresh the record*/
    pRobotInfo = InfoManager.RobotInfo;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            if ( SenderID == pRobotInfo->RobotID )
            {
                Index = i;
                break;
            }
        }

        pRobotInfo++;
    }

    /*if no matched record, to occupy the oldest or blank record*/
    if ( ENVROBOT_INFOTBL_NUM == i )
    {
        Index = OldRec;
    }

    ASSERT_T( Index < ENVROBOT_INFOTBL_NUM );

    pRobotInfo = InfoManager.RobotInfo;
    pRobotInfo += Index;

    pRobotInfo->RobotID = SenderID;

    /**Deal with special MSG**/
    if ( MSGID_SELFTYPE == MessageID )
    {
        pRobotInfo->Type = Para;
    }
    else
    {
        pRobotInfo->MsgID = MessageID;
        pRobotInfo->Para = Para;
    }

    bitSet(pRobotInfo->Position,IRLoc);
    pRobotInfo->TTLCnt = MSG_TTL;


    /**refresh the oldest record**/
    pRobotInfo = InfoManager.RobotInfo;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( pRobotInfo->TTLCnt < InfoManager.RobotInfo[OldRec].TTLCnt )
        {
            OldRec = i;
            if ( 0 == pRobotInfo->TTLCnt )
            {
                break;
            }
        }
        pRobotInfo++;
    }

}



void Messager_cls::ClrEnvInfoLoc()
{
    InfoManager.Valid = false;

    InfoManager.ObstacleInfo= 0;
}


inline void Messager_cls::ClrEnvInfoMsg( EnvRobotInfoRec_stru *pRobotInfo )
{
    if ( NULL == pRobotInfo )
    {
        ASSERT_T(0);
        return;
    }
    pRobotInfo->TTLCnt = 0;
    pRobotInfo->MsgID = MSGID_INVALID;
    pRobotInfo->Para = 0;
    pRobotInfo->Position = 0;
    pRobotInfo->LocAngle = INVALID;
    pRobotInfo->FaceAngle = INVALID;
}


/** get moving info from mortor??
 * Front is 0 degree, L is 90 degree
 *      F
 *  L        R
 *      B
 */
void Messager_cls::CalibrateEnvInfo( )
{
    static bool LastMovingFlag = false;
    bool MovingFlag;

    MovingFlag = Wheels.isMoving();

    if ( (true == LastMovingFlag) &&  (false == MovingFlag) )
    {
        uint8_t i;
        EnvRobotInfoRec_stru *pRobotInfo = NULL;
        int8_t DeltaAngle;
        MTMovDir_enum Direction;
        uint8_t Para;

        Wheels.GetMoveAciton( Direction, Para);

        switch ( Direction )
        {
        case MT_CLOCKWISE:
            DeltaAngle = (int8_t)Para;
            break;

        case MT_ANTICLOCK:
            DeltaAngle = 0 - (int8_t)Para;
            break;

        default:
		    LastMovingFlag = MovingFlag;
            return;
        }

        pRobotInfo = InfoManager.RobotInfo;
        for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
        {
            if ( (0 < pRobotInfo->TTLCnt) && (INVALID != pRobotInfo->LocAngle) )
            {
                pRobotInfo->LocAngle += DeltaAngle;
                pRobotInfo->LocAngle = pRobotInfo->LocAngle%(360/MT_ANGEL_UNIT);
                if ( pRobotInfo->LocAngle > (180/MT_ANGEL_UNIT) )
                {
                    pRobotInfo->LocAngle -= (360/MT_ANGEL_UNIT);
                }
                else if ( pRobotInfo->LocAngle < -(180/MT_ANGEL_UNIT))
                {
                    pRobotInfo->LocAngle += (360/MT_ANGEL_UNIT);
                }
                else
                {
                    //nothing
                }

            }

            pRobotInfo++;
        }

    }

    LastMovingFlag = MovingFlag;
}

/* Front is 0 degree, L is 90 degree
*      F
*  L        R
*      B
*/
void Messager_cls::RefreshEnvInfo()
{
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    static TargetToAngle_stru aTagetPriority[] = {
            {TARGET_POS_F, 0},
            {TARGET_POS_L, 90/MT_ANGEL_UNIT},
            {TARGET_POS_R, -90/MT_ANGEL_UNIT},
            {TARGET_POS_B, 180/MT_ANGEL_UNIT},
            {TARGET_POS_FL, 20/MT_ANGEL_UNIT},   //rotate half angle
            {TARGET_POS_FR, -20/MT_ANGEL_UNIT},
            {TARGET_POS_BL, 110/MT_ANGEL_UNIT},
            {TARGET_POS_BR, -110/MT_ANGEL_UNIT }, };

    RefreshPos( InfoManager.ObstacleInfo, MS_OR );

    pRobotInfo = InfoManager.RobotInfo;
    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {

            RefreshPos( pRobotInfo->Position, MS_AND );

            /*If receive msg than update the LocAngle, otherwise calibrate the LocAngle according to Move*/
            if ( 0 != pRobotInfo->Position )
            {
                for ( i = 0; i < POSITION_NUM; ++i )
                {
                    if ( 1 == bitRead( pRobotInfo->Position, (uint8_t)aTagetPriority[i].Direction) )
                    {
                        pRobotInfo->LocAngle = aTagetPriority[i].Angle;
                        break;
                    }
                }
                pRobotInfo->Position = 0;
            }
            else
            {
                ; //nothing
            }
        }

        pRobotInfo++;
    }

}

void Messager_cls::RefreshPos(uint8_t &Position, Logic_enum Logic)
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

        if ( MS_AND == Logic )
        {
            BitVal = bitRead(Position, InFirst) && bitRead(Position, InSecond);
        }
        else
        {
            BitVal = bitRead(Position, InFirst) || bitRead(Position, InSecond);
        }

        bitWrite(Position, Out, BitVal);

    }
}


#define MSG_QUERYTIMER_T 2  //uint: 100ms
void Messager_cls::QueryTimeProc( uint8_t TimeOn100ms)
{

    static uint8_t LastTime = 0;

    if ( (unsigned uint8_t)( TimeOn100ms - LastTime) < MSG_QUERYTIMER_T )
    {
        return;
    }

    LastTime = TimeOn100ms;

    /*Aging proc: to caculate TTL */
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = InfoManager.RobotInfo;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            pRobotInfo->TTLCnt--;

            if ( 0 == pRobotInfo->TTLCnt )
            {
                ClrEnvInfoMsg(pRobotInfo);
            }
        }

        pRobotInfo++;
    }
}


bool Messager_cls::IsEnvInfoValid( )
{
    return InfoManager.Valid;
}

/*If there are many records meeting the condition, it will return the record than its Para is the most.
 * If the Paras are same, it will return the newest record*/
uint8_t Messager_cls::GetEnvRobotInfo( uint8_t RobotID, uint8_t Para, EnvRobotInfoRec_stru &OutInfo )
{
    uint8_t i;
    uint8_t MsgID, Type, Index;
    uint8_t &StartIndex = InfoManager.SearchIndex;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;


    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        Index = (StartIndex + i)%ENVROBOT_INFOTBL_NUM;
        pRobotInfo = InfoManager.RobotInfo + Index;

        if ( 0 < pRobotInfo->TTLCnt )
        {

            if ( (pRobotInfo->RobotID == RobotID) || (ROBOT_ID_ANY == RobotID) )
            {
                MsgID = Para;
                if ( (pRobotInfo->MsgID == MsgID) || (MSGID_ANY == MsgID) )
                {
                    break;
                }
            }
            else if ( ROBOT_ID_TYPE == RobotID)
            {
                Type = Para;
                if ( Type == pRobotInfo->Type )
                {
                    break;
                }
            }
            else if ( ROBOT_ID_LESS == RobotID )
            {
                MsgID = Para;
                if ( (pRobotInfo->RobotID < ROBOT_ID_SELF)
                        && ((pRobotInfo->MsgID == MsgID) || (MSGID_ANY == MsgID)) )
                {
                    break;
                }
            }
            else
            {
                ;
            }

        }

    }

    if ( i >= ENVROBOT_INFOTBL_NUM )
    {
        return INFO_REC_NONE;
    }
    
    StartIndex = (Index + 1)%ENVROBOT_INFOTBL_NUM;
    pRobotInfo = InfoManager.RobotInfo + Index;

    OutInfo = *pRobotInfo;

    return SUCCESS;
}

void Messager_cls::GetEnvObstacleInfo( uint8_t *pInfo )
{
    *pInfo = InfoManager.ObstacleInfo;
}

void Messager_cls::GetEnvStatistic( uint8_t MsgID, uint8_t Para, uint8_t *pRslt )
{
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    pRobotInfo = InfoManager.RobotInfo;

    *pRslt = 0;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {
            if ( MsgID == pRobotInfo->MsgID )
            {
                *pRslt++;
            }
        }

        pRobotInfo++;
    }
}


#if _DEBUG_MS
void Messager_cls::ShowEnvRobotInfo()
{
    uint8_t i;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    pRobotInfo = InfoManager.RobotInfo;

    Serial.print("RInfo: V=");
    Serial.print(InfoManager.Valid);
    Serial.print(" SI=");
    Serial.println(InfoManager.SearchIndex);

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {

            Serial.print(" I-");
            Serial.print(i);
            Serial.print(":ID=");
            Serial.print( pRobotInfo->RobotID );
            Serial.print(" Type=");
            Serial.print( pRobotInfo->Type );
            Serial.print(" Msg=");
            Serial.print( pRobotInfo->MsgID );
            Serial.print(" Para=");
            Serial.print( pRobotInfo->Para );
            Serial.print(" Pos=");
            Serial.print( pRobotInfo->Position, BIN );
            Serial.print(" LocA=");
            Serial.print( pRobotInfo->LocAngle );
            Serial.print(" FaceA=");
            Serial.print( pRobotInfo->FaceAngle );
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
