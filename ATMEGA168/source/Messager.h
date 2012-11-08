#ifndef _h
#define _h

#include <inttypes.h>
#include "Sensor.h"

#if _DEBUG
#define _DEBUG_MS 1
#endif


/* TODO: the more 1 bits, the more less power */
typedef enum
{
    MSGID_INVALID = ROBOT_TYPE_END, // ROBOT_TYPE is also used as NOTIFY
    MSGID_NOTIFY_IFINDYOU,    //NOTIFY: not need to respond, eg. phenomone info.
    MSGID_NOTIFY_TOBUILD,     // It's para: next id of robot
    MSGID_NOTIFY_TOFOOD,     // It's para: next id of robot(phenomone)
    MSGID_NOTIFY_TOHOME,     //It's para: next id of robot(phenomone)
    MSGID_REQUEST,
    MSGID_RESPONCE,

    MSGID_SELFTYPE = 0XFE,
    MSGID_ANY = 0XFF
}MsgID_enum;

typedef enum
{
    TARGET_POS_FL = IR_POSITION_FL,
    TARGET_POS_FR = IR_POSITION_FR,
    TARGET_POS_BL =IR_POSITION_BL,
    TARGET_POS_BR = IR_POSITION_BR,
    TARGET_POS_F,
    TARGET_POS_L,
    TARGET_POS_R,
    TARGET_POS_B    //it must be the last
}TargetPos_enum;

#define POSITION_INVALID 0XFF
#define POSITION_NUM (TARGET_POS_B+1)

#define ENVROBOT_INFOTBL_NUM POSITION_NUM  //should not be less than POSITION_NUM
#define MSG_TTL ( ENVROBOT_INFOTBL_NUM * 2 )
typedef struct EnvRobotInfoRec
{
    uint8_t RobotID;  //key
    uint8_t MsgID;    //key
    uint8_t Para;
    uint8_t Position; //use bit value is TargetPos_enum, POSITION_INVALID
    uint8_t TTLCnt;
}EnvRobotInfoRec_stru;


typedef struct EnvInfoManager
{
    bool Valid;
    uint8_t ObstacleInfo; //use bit.  TargetPos_enum

    uint8_t RobotInfoOldRec;
    EnvRobotInfoRec_stru RobotInfo[ENVROBOT_INFOTBL_NUM];
}EnvInfoManager_stru;


typedef struct
{
    TargetPos_enum Out;
    TargetPos_enum InFirst;
    TargetPos_enum InSecond;
}PosRefreshRec_stru;



class Messager_cls
{
private:
    IR_Sensor &MsgSrc;

    EnvInfoManager_stru InfoManager;
    
    inline void MsgToEvnInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc );
    inline void SetEnvRobotInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc );
    inline void RefreshEnvInfo();
    void RefreshPos(uint8_t &Position);

    inline void AgingProc();


protected:

public:
    Messager_cls( IR_Sensor &);
    void MessageProc();
    bool IsEnvInfoValid();
    uint8_t GetEnvRobotInfo( uint8_t *pRobotID, uint8_t *pMsgID, uint8_t *pPara=NULL, uint8_t *pPosition=NULL );
    void GetEnvObstacleInfo( uint8_t *pInfo );
    void ClrEnvInfo();
    void TimerProc();

    #if _DEBUG_MS
    void ShowEnvRobotInfo();
    #endif

};


#endif


