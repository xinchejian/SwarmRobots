#ifndef _h
#define _h

#include <inttypes.h>
#include "Sensor.h"

#if _DEBUG
#define _DEBUG_MS 0
#endif

#define MS_MSG_TTL 5  // = duration of move step,  unit: ms

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

//todo, walk around not be needed
typedef struct MessageRec
{
    uint8_t State;
    uint8_t SenderID;   // KEY = SenderID and MessageID
    uint8_t MessageID;  // KEY
    uint8_t Para;
}MessageRec_stru;

/*ExteriorInfoID_enum also used as index of ExteriorInfoRec_stru[], so it must be started from 0 and be continue*/
typedef enum
{
   EXTERIOR_ID_BEGIN = 0, // CONDITION_BEGIN must be the fisrt
   OBSTACLE_FRONT_LEFT,  ////the relation aganist obstacle
   OBSTACLE_BEHIND_LEFT,
   OBSTACLE_FRONT_RIGHT,
   OBSTACLE_BEHIND_RIGHT,

   TARGET_FRONT,       //the relation aganist target
   TARGET_LEFT,
   TARGET_RIGHT,
   TARGET_BEHIND,

   EXTERIOR_ID_END             // CONDITION_END must be the last
}ExteriorInfoID_enum;

#define MS_CONDITIONTBL_NUM EXTERIOR_ID_END  //it must equal to ExteriorInfoID_enum
/*when the robot has moved, the condition is out of date*/

typedef enum
{
    COND_FALSE,
    COND_TRUE,
    COND_UNKNOWN
}ExteriorInfoState_enum;

typedef struct ExteriorInfoRec
{
    ExteriorInfoState_enum State;
    uint8_t BornTime;  //todo ? when State becomes ture, it will be set.  maybe we can use relative time, to decrease the duration of TimerProc.
}ExteriorInfoRec_stru;



typedef struct ExteriorInfoManager
{
    bool Valid;
    ExteriorInfoRec_stru Tbl[MS_CONDITIONTBL_NUM];
}ExteriorInfoManager_stru;


typedef struct MsgToCondRec
{
    RXBufState_enum MsgState;
    uint8_t SenderID;
    uint8_t MessageID;
    IRPosition_enum IRPos;
    ExteriorInfoID_enum CondID;
    ExteriorInfoState_enum CondState;
}MsgToInfoRec_stru;

typedef enum
{
    MSG_INVALID = 0X0,
    MSG_ANY = 0XFF
};




class Messager_cls
{
private:
    IR_Sensor &MsgSrc;

    ExteriorInfoManager_stru InfoManager;
    
    static MsgToInfoRec_stru MsgToInfoRule[];


    inline void MsgToExteriorInfo( MessageRec_stru &Msg, IRPosition_enum IRLoc );
    inline void RefreshExteriorInfo();
    inline void AgingProc();
    inline void SetExteriorInfo(ExteriorInfoID_enum, ExteriorInfoState_enum);


protected:

public:
    Messager_cls( IR_Sensor &);
    void MessageProc();
    bool IsExteriorInfoValid();
    void GetExteriorInfo( ExteriorInfoID_enum, uint8_t * );
    void ClrExteriorInfo();
    void TimerProc();

};


#endif


