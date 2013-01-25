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
    
    for ( i = 0; i < POSITION_NUM; ++i )
    {
        InfoManager.NearRobot[i] = ROBOT_ID_NONE;
    }

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
    /*if the record has been stored and then to refresh the record*/
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
    if ( ENVROBOT_INFOTBL_NUM <= i )
    {
        Index = OldRec;
    }

    ASSERT_T( Index < ENVROBOT_INFOTBL_NUM );

    pRobotInfo = InfoManager.RobotInfo;
    pRobotInfo += Index;

    pRobotInfo->RobotID = SenderID;

    /**Deal with special MSG, broadcast msg**/
    if ( MSGID_SELFTYPE == MessageID )
    {
        pRobotInfo->Type = Para;
    }
    else if ( MSGID_NEARRANGE == MessageID )
    {
        pRobotInfo->Near = NEARRANGE_TTL;
    }
    else //single-cast msg
    {
        pRobotInfo->MsgID = MessageID;
        if ( INVALID != Para )
        {
            pRobotInfo->Para = Para;
        }
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
    pRobotInfo->Para = INVALID;
    pRobotInfo->Position = 0;
    pRobotInfo->LocAngle = INVALID;
    pRobotInfo->FaceAngle = INVALID;
    pRobotInfo->Near = 0;
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
            if ( 0 < pRobotInfo->TTLCnt )
            {
                if (INVALID != pRobotInfo->LocAngle)
                {
                    pRobotInfo->LocAngle += DeltaAngle;
                    pRobotInfo->LocAngle = pRobotInfo->LocAngle%( MT_DEGREE(360) );
                }

                if (INVALID != pRobotInfo->FaceAngle)
                {
                    pRobotInfo->FaceAngle += DeltaAngle;
                    pRobotInfo->FaceAngle = pRobotInfo->FaceAngle%( MT_DEGREE(360) );
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
    uint8_t i, j;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    static TargetToAngle_stru aTagetPriority[] = {
            { TARGET_POS_F, MT_DEGREE(0) },
            { TARGET_POS_L, MT_DEGREE(90) },
            { TARGET_POS_R, MT_DEGREE(270) },
            { TARGET_POS_B, MT_DEGREE(180) },
            { TARGET_POS_FL, MT_DEGREE(20) },
            { TARGET_POS_FR, MT_DEGREE(340) },
            { TARGET_POS_BL, MT_DEGREE(110) },
            { TARGET_POS_BR, MT_DEGREE(200) }, };

    RefreshPos( InfoManager.ObstacleInfo, MS_OR );

    /**Calculate the robot angle**/
    pRobotInfo = InfoManager.RobotInfo;
    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( 0 < pRobotInfo->TTLCnt )
        {

            RefreshPos( pRobotInfo->Position, MS_AND );

            /*If receive msg than update the LocAngle, otherwise calibrate the LocAngle according to Move*/
            if ( 0 != pRobotInfo->Position )
            {
                for ( j = 0; j < POSITION_NUM; ++j )
                {
                    if ( 1 == bitRead( pRobotInfo->Position, (uint8_t)aTagetPriority[j].Direction) )
                    {
                        pRobotInfo->LocAngle = aTagetPriority[j].Angle;
                        break;
                    }
                }

                pRobotInfo->Position = 0;

                /**calculate the faceangle**/
                uint8_t S2T_Angle, T2S_Angle;
                T2S_Angle = pRobotInfo->Para;
                if ( INVALID != T2S_Angle )
                {
                    S2T_Angle = pRobotInfo->LocAngle;
                    S2T_Angle = ( S2T_Angle < MT_DEGREE(180) ) ? (S2T_Angle + MT_DEGREE(180)) : (S2T_Angle - MT_DEGREE(180));

                    pRobotInfo->FaceAngle = ( S2T_Angle >= T2S_Angle ) ? (S2T_Angle - T2S_Angle) : (MT_DEGREE(360)+S2T_Angle-T2S_Angle);
                }
            }
            else
            {
                ; //nothing
            }
        }

        pRobotInfo++;
    }


    RefreshNearRobot();
}

inline void Messager_cls::RefreshNearRobot()
{
    uint8_t i;
    TargetPos_enum Position;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    for ( i = 0; i < POSITION_NUM; ++i )
    {
        InfoManager.NearRobot[i] = ROBOT_ID_NONE;
    }

    pRobotInfo = InfoManager.RobotInfo;
    InfoManager.RobotAsObstacle = 0;

    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        if ( (0 < pRobotInfo->TTLCnt) && ( 0 < pRobotInfo->Near) )
        {
            Position = GetLocationfromAngle( pRobotInfo->LocAngle );
            InfoManager.NearRobot[Position] = pRobotInfo->RobotID;

            bitSet(InfoManager.RobotAsObstacle, Position);
        }

        pRobotInfo++;
    }

    RefreshPos( InfoManager.RobotAsObstacle, MS_OR );
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

    if ( (uint8_t)( TimeOn100ms - LastTime) < MSG_QUERYTIMER_T )
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
            else if ( 0 < pRobotInfo->Near )
            {
                pRobotInfo->Near--;
            }
            else
            {
                //nothing
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
uint8_t Messager_cls::GetEnvRobotInfo( uint8_t InfoKind, uint8_t RobotID, uint8_t Para, EnvRobotInfoRec_stru &OutInfo )
{
    uint8_t i;
    uint8_t Index;
    bool Flag;
    uint8_t &StartIndex = InfoManager.SearchIndex;
    EnvRobotInfoRec_stru *pRobotInfo = NULL;

    Flag = false;
    for ( i = 0; i < ENVROBOT_INFOTBL_NUM; ++i )
    {
        Index = (StartIndex + i)%ENVROBOT_INFOTBL_NUM;
        pRobotInfo = InfoManager.RobotInfo + Index;

        if ( 0 < pRobotInfo->TTLCnt )
        {
            switch ((RobotInfoKind_enum)InfoKind)
            {
            case ROBOTINFO_MSG:
                if ( ((pRobotInfo->RobotID == RobotID) || (ROBOT_ID_ANY == RobotID))
                        &&  ((pRobotInfo->MsgID == Para) || (MSGID_ANY == Para)) )
                {
                    Flag = true;
                }
                break;

            case ROBOTINFO_TYPE:
                if ( Para == pRobotInfo->Type )
                {
                    Flag = true;
                }
                break;

            case ROBOTINFO_NEAR:
                if ( (pRobotInfo->RobotID == RobotID) || (ROBOT_ID_ANY == RobotID) )
                {
                    Flag =  (pRobotInfo->Near > 0) ? Para : !Para;
                }
                break;

            case ROBOTINFO_MSG_LESSID:
                if ( (pRobotInfo->RobotID < ROBOT_ID_SELF)
                        && ((pRobotInfo->MsgID == Para) || (MSGID_ANY == Para)) )
                {
                    Flag = true;
                }
                break;

            default:
                ASSERT_T(0);

            }

        }

        if ( Flag )
        {
            break;
        }

    }

    if ( !Flag )
    {
        return INFO_REC_NONE;
    }

    OutInfo = *pRobotInfo;
    
    StartIndex = (Index + 1)%ENVROBOT_INFOTBL_NUM;

    return SUCCESS;
}

void Messager_cls::GetEnvObstacleInfo( uint8_t *pInfo )
{
    *pInfo = InfoManager.ObstacleInfo;
}

void Messager_cls::GetRobotAsObstacleInfo( uint8_t *pInfo )
{
    *pInfo = InfoManager.RobotAsObstacle;
}

uint8_t Messager_cls::GetEnvNearRobot( TargetPos_enum Position )
{
    return InfoManager.NearRobot[Position];
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
                (*pRslt)++;
            }
        }

        pRobotInfo++;
    }
}

TargetPos_enum Messager_cls::GetLocationfromAngle( int8_t Angle )
{
    uint8_t i, Num;
    TargetPos_enum Location;

    static AngleToDirection_stru aAngleToDirection[] =
    {
        {MT_DEGREE(0), MT_DEGREE(10), TARGET_POS_F},
        {MT_DEGREE(10), MT_DEGREE(80), TARGET_POS_FL},
        {MT_DEGREE(80), MT_DEGREE(100), TARGET_POS_L},
        {MT_DEGREE(100), MT_DEGREE(170), TARGET_POS_BL},
        {MT_DEGREE(170), MT_DEGREE(190), TARGET_POS_B},
        {MT_DEGREE(190), MT_DEGREE(260), TARGET_POS_BR},
        {MT_DEGREE(260), MT_DEGREE(280), TARGET_POS_R},
        {MT_DEGREE(280), MT_DEGREE(350), TARGET_POS_FR},
        {MT_DEGREE(350), MT_DEGREE(360), TARGET_POS_F},
    };

    ASSERT_T( Angle <= MT_DEGREE(360) );

    Location = TARGET_POS_F;
    Num = sizeof(aAngleToDirection)/sizeof(aAngleToDirection[0]);

    for ( i = 0; i < Num; ++i )
    {
        if ( (Angle >= aAngleToDirection[i].AngleB)
                && (Angle <= aAngleToDirection[i].AngleE) )
        {
            Location = aAngleToDirection[i].Direction;
            break;
        }
    }

    return Location;
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
            Serial.print(" Near=");
            Serial.print( pRobotInfo->Near );
            Serial.print(" TTL=");
            Serial.println( pRobotInfo->TTLCnt );
        }

        pRobotInfo++;
    }


}
#endif





#if 0

/*table constraint:
 * the first uint8_t colum must be used to identify the validation of record.  NULL means empty.
 * the following columns must be KEY,
 * */
typedef struct TblCtrl
{
    uint8_t MaxRec;     //
    uint8_t RecLen;     //the length of a record, uint:byte
    uint8_t KeyByteNum;    // the byte numbers of the key, uint:byte
    uint8_t EmptyIndex;  // an empty row that can be available right now.
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
