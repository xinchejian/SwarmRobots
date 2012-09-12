#ifndef Action_h
#define Action_h

#include <inttypes.h>
#include "SwarmRobot.h"
#include "Messager.h"
#include "Motor.h"

#if _DEBUG
#define _DEBUG_ACTION 0
#endif

/*   */

//B-Step = Big Step = the lenth of robot (have enough space to rotate)
//S-Step = Small Step = one step of motor = one third of Big Step (avoid colliding)
#define BIGSTEP_TIMES 8

typedef enum
{
   INNERIOR_ID_BEGIN,    // must be the fisrt
   MOVE_FORWARD_BIGSTEP,
   MOVE_BACKWARD_BIGSTEP,
   INNERIOR_ID_END             // CONDITION_END must be the last
}IneriorInfoID_enum;

#define ACTION_INERIORINFO_NUM ( INNERIOR_ID_END - INNERIOR_ID_BEGIN )  //it must equal to ExteriorInfo_enum

typedef struct
{
    bool State;
}IneriorInfoRec_stru;

/*  */
typedef enum
{
    EXTERIORINFO,
    INTERIORINFO
}InfoType_enum;

typedef enum
{
    ACTION_NONE,
    ACTION_JUMP,
    ACTION_MOVE,
    ACTION_SENDMSG
}ActionID_enum;

typedef struct RuleRec
{
    InfoType_enum InfoType;
    uint8_t InfoID;
    uint8_t InfoState;
    uint8_t ActionID;
    uint8_t ParaA;
    uint8_t ParaB;
}RuleRec_stru;




class Action_cls
{
private:
    IR_Sensor &Sensor;
    BiMotor &Wheels;
    Messager_cls &Messager;

    IneriorInfoRec_stru IneriorInfoTbl[ACTION_INERIORINFO_NUM];
    static RuleRec_stru RuleTbl[];

    void AnalyzeRule( uint8_t StartIndex, uint8_t *pActionID, uint8_t *pParaA, uint8_t *pParaB );
    void LocateRuleIndex( uint8_t InfoType, uint8_t InfoID, uint8_t *pLocatedIndex );
    void ExecuteAction( uint8_t ActionID, uint8_t ParaA, uint8_t ParaB );

    inline void SetIneriorInfo( uint8_t InfoID, uint8_t State );
    inline void GetIneriorInfo( uint8_t InfoID, uint8_t *pState );

protected:


public:
    Action_cls(IR_Sensor &, BiMotor &, Messager_cls &);
    void ActionProc();
};

#endif


/*
 * task:
1=walk about
2=walk to target
3=line up and move


near obstacle condition
{
    { obstacle-f fl fr b bl br l r },
    { target - f fl fr b bl br l r },
    {


}
 *
robot location condition
{
   location - who
}

message condition
{
        { TargetID close/block/attack  relative-location }
}

1,detecting --  move one step/ rotate some angle  and than detecting
2,receive at all time

action
  condition Y/N     action
 stepnum=6      rotate randomly(clear step count)
 back moving with six = Y   move-backward
 nothing        judge front
 obstacle-f Y    judge left
 obstacle-f N    move-forward
 obstacle-f     stop
 obstacle-l Y   judge right
 obstacle-l N   rotate left
 obstacle-r Y   judge back
 obstacle-r N   rotate right

 obstacle-b Y   wait
 obstacle-b N   move-backward(6 step)

 stop          judge direction
 stop          send: i am

 nothing      move random

  judge proper
 obstacle-fl
rotate(any, 90degree)
*/
