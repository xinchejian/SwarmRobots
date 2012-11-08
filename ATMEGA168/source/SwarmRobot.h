/*public header*/

#ifndef SwarmRobt_h
#define SwarmRobt_h

#define _DEBUG 0
#if _DEBUG
#define _DEBUG_MAIN 1
#define _DEBUG_TIMER 0
#endif




/*Definition of robot.  The type of robot can convert to another under some condition.
 *  eg. ANT->FOOD, ANT->PHEROMONE, PHEROMONE->ANT  */
enum Robot_Type
{
    ROBOT_TYPE_ANT,
    ROBOT_TYPE_FOOD,

    ROBOT_TYPE_seperate_STAIC,  // it is a sepetator, if enum is larger than it, the robot cannot move.
    ROBOT_TYPE_POWERSTATION,
    ROBOT_TYPE_NEST,
    ROBOT_TYPE_PHEROMONE,

    ROBOT_TYPE_END
};


#define ROBOT_INIT_TYPE ROBOT_TYPE_ANT
#define ROBOT_GROUP 1   //it is used in matching scenario;  and 7 is used as public object.
#define ROBOT_INNERID 8 //must be less than pow(2,ROBOT_INNERID_BITS). 0 is reserved





#define ROBOT_ID_SOURCE  0XFD
#define ROBOT_ID_TARGET  0XFE
#define ROBOT_ID_ANY     0XFF

#define ROBOT_GROUP_BITS 3
#define ROBOT_INNERID_BITS ( 8 - ROBOT_GROUP_BITS )

#define ROBOT_ID_SELF ( ((ROBOT_GROUP << ROBOT_INNERID_BITS ) & ( 0xFF << ROBOT_INNERID_BITS )) \
        | ( ROBOT_INNERID & ( 0xFF >> ROBOT_GROUP_BITS) ) )

#define IS_SAMEGROUP(id) ( ROBOT_GROUP == ((uint8_t)id >> ROBOT_INNERID_BITS) )





/*Definition of LED: */
//high = light
#define LED_RUNING_PIN SCK  //SCK
#define LED_INFO_R 3


#define LED_FL_PIN A3
#define LED_FR_PIN A2
#define LED_BL_PIN A0
#define LED_BR_PIN A1


#define SUCCESS 0
enum SW_ERRINFO
{
    ERR_ASSERT = 0x01,
    ERR_DEBUG = 0x02,

    ERR_PARA = 0x08,

    INFO_BUSY = 0X80,
    INFO_QUE_FULL,
    INFO_QUE_EMPTY
};


#define NULL 0
#define INVALID 0XFF

#if _DEBUG
    //ref AVRlib " assert(e)    __assert(__func__, __FILE__, __LINE__, #e))
    #define ASSERT_T(A) if( !(A) ) { Serial.print("ERR:F=");Serial.print(__FILE__);Serial.print(" L=");Serial.println(__LINE__);}
#else
    #define ASSERT_T(A)
#endif

extern uint32_t g_BaseTimeFrequency;

#endif





