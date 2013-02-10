/* The type of MSG:
 * 1)broadcast:  robotID = ROBOT_ID_ANY
 * 2)Singlecast: robotID = specific ID
 *
 * or
 * 1)common msg:
 * 2)special msg:  MsgID = special ID
 *
 */



#ifndef _h
#define _h

#include <inttypes.h>
#include "Sensor.h"
#include "Motor.h"

#if _DEBUG
#define _DEBUG_MS 0
#endif


/* TODO: the more 1 bits, the lower the power */
typedef enum
{
    MSGID_INVALID,
    MSGID_NOTIFY_IFINDYOU,    //NOTIFY: not need to respond, eg. pheromone info.
    MSGID_NOTIFY_TOBUILD,     // It's param: next id of robot
    MSGID_NOTIFY_TOFOOD,     // It's param: next id of robot(phenomone)
    MSGID_NOTIFY_TOHOME,     //It's param: next id of robot(phenomone)
    MSGID_NOTIFY_DONE,     //It's param: next id of robot(phenomone)
    MSGID_REQUEST,
    MSGID_RESPONCE,
    MSGID_ACK,

    MSGID_NEARRANGE = 0XFD,
    MSGID_SELFTYPE = 0XFE,
    MSGID_ANY = 0XFF
}MsgID_enum;

typedef enum
{
    TARGET_POS_FL = IR_POSITION_FL,  //must equal 0
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
#define NEARRANGE_TTL ( ENVROBOT_INFOTBL_NUM )

typedef enum
{
    MS_AND,
    MS_OR
}Logic_enum;

typedef enum
{
    ROBOTINFO_MSG,
    ROBOTINFO_TYPE,
    ROBOTINFO_NEAR,
    ROBOTINFO_MSG_LESSID,
    ROBOTINFO_GRP,
    ROBOTINFO_END
}RobotInfoKind_enum;

enum
{
    GRP_SAME
};


typedef struct TargetToAngle
{
    TargetPos_enum Direction;
    int8_t Angle;
}TargetToAngle_stru;


typedef struct EnvRobotInfoRec
{
    uint8_t RobotID;  //key
    uint8_t Type;
    uint8_t MsgID;
    uint8_t Para;
    uint8_t Position; // store the location tmperorily.  use bit value is TargetPos_enum
    uint8_t LocAngle; //The location angle of target to myself
    uint8_t FaceAngle; //The Face of Target to myself
    uint8_t Near;  //if > 0, Near;
    uint8_t TTLCnt;
}EnvRobotInfoRec_stru;


typedef struct EnvInfoManager
{
    bool Valid;
    uint8_t ObstacleInfo; //use bit.  TargetPos_enum
    uint8_t RobotAsObstacle; //use bit.  from near Robot
    uint8_t NearRobot[ENVROBOT_INFOTBL_NUM];  //eight location; first element is ID

    uint8_t RobotInfoOldRec;  //store the index of the oldest record or blank record
    uint8_t SearchIndex;  //define the start index when seaching record in order to prevent from finding the the same record
    EnvRobotInfoRec_stru RobotInfo[ENVROBOT_INFOTBL_NUM];
}EnvInfoManager_stru;


typedef struct
{
    TargetPos_enum Out;
    TargetPos_enum InFirst;
    TargetPos_enum InSecond;
}PosRefreshRec_stru;

typedef struct AngleToDirection
{
    int8_t AngleB;
    int8_t AngleE;
    TargetPos_enum Direction;
}AngleToDirection_stru;


class Messager_cls
{
private:
    IR_Sensor &MsgSrc;
    BiMotor &Wheels;

    EnvInfoManager_stru InfoManager;
    
    inline void MsgToEvnInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc );
    inline void SetEnvRobotInfo( IRMsgOutput_stru &Msg, IRPosition_enum IRLoc );
    void RefreshEnvInfo();
    inline void RefreshNearRobot();
    void CalibrateEnvInfo();
    void RefreshPos(uint8_t &Position, Logic_enum);
    inline void ClrEnvInfoMsg(EnvRobotInfoRec_stru *);

    void QueryTimeProc(uint8_t);


protected:

public:
    Messager_cls( IR_Sensor &, BiMotor &);
    void MessageProc();
    bool IsEnvInfoValid();
    uint8_t GetEnvRobotInfo( uint8_t InfoKind, uint8_t RobotID, uint8_t Para, EnvRobotInfoRec_stru &OutInfo );
    void GetEnvObstacleInfo( uint8_t *pInfo );
    void GetRobotAsObstacleInfo( uint8_t *pInfo );
    uint8_t GetEnvNearRobot(TargetPos_enum);
    void ClrEnvInfoLoc();

    void GetEnvStatistic( uint8_t MsgID, uint8_t Para, uint8_t *pRslt );
    TargetPos_enum GetLocationfromAngle( int8_t Angle );

    #if _DEBUG_MS
    void ShowEnvRobotInfo();
    #endif

};


#endif


