/*public header*/

#ifndef SwarmRobt_h
#define SwarmRobt_h

#define _DEBUG 0  //if open _DEBUG_ACTION, the data section is 940 bytes.  if no debug, data size if less than 500 bytes
#if _DEBUG
#define _DEBUG_MAIN 1
#define _DEBUG_TIMER 0
#endif

#include "Ledlight.h"


/*Definition of robot.  The type of robot can convert to another under some condition.
 *  eg. ANT->FOOD, ANT->PHEROMONE, PHEROMONE->ANT  */


/*define the type of robot(it is also used as MsgID_enum).  because #if cannot use enumeration, so use Macro*/
#define ROBOT_TYPE_ANT  0x10
#define ROBOT_TYPE_ANT_LINEUP 0x11
#define ROBOT_TYPE_ANT_SAMEDIRECTION 0x12
#define ROBOT_TYPE_FOOD 0x21

#define ROBOT_TYPE_POWERSTATION 0x70  //cannot move.
#define ROBOT_TYPE_NEST         0x71
#define ROBOT_TYPE_PHEROMONE    0x72

#define ROBOT_TYPE_END          0x80  //refer to MsgID_enum

/*Special ID of robot*/
#define ROBOT_ID_SPECIAL_START  0XF0
#define ROBOT_ID_POLLING  0XFA   //Poll id
#define ROBOT_ID_SOURCE  0XFB
#define ROBOT_ID_TARGET  0XFC
#define ROBOT_ID_FOLLOWER 0XFD
#define ROBOT_ID_NONE    0XFE
#define ROBOT_ID_ANY     0XFF

#define ROBOT_GROUP_BITS 3
#define ROBOT_INNERID_BITS ( 8 - ROBOT_GROUP_BITS )
#define IS_SAMEGROUP(id) ( ROBOT_GROUP == ((uint8_t)id >> ROBOT_INNERID_BITS) )

#include "SwbotConfig.h"

#define ROBOT_ID_SELF ( ((ROBOT_GROUP << ROBOT_INNERID_BITS ) & ( 0xFF << ROBOT_INNERID_BITS )) \
        | ( ROBOT_INNERID & ( 0xFF >> ROBOT_GROUP_BITS) ) )



#define SUCCESS 0
enum SW_ERRINFO
{
    ERR_ASSERT = 0x01,
    ERR_DEBUG = 0x02,

    ERR_PARA = 0x08,

    INFO_BUSY = 0X80,
    INFO_QUE_FULL,
    INFO_QUE_EMPTY,
    INFO_REC_NONE
};

#ifndef NULL
#define NULL 0
#endif

#define INVALID 0XFF
#define SIGNED_INVALID 0X7F

#if _DEBUG
    //ref AVRlib " assert(e)    __assert(__func__, __FILE__, __LINE__, #e))
    #define ASSERT_T(A) if( !(A) ) { Serial.print("ERR:F=");Serial.print(__FILE__);Serial.print(" L=");Serial.println(__LINE__);}
#else
    #define ASSERT_T(A)
#endif

extern Ledlight_cls LedLight;

extern uint8_t GetTimeOn100ms();

#endif





