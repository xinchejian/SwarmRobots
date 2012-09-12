/*public header*/

#ifndef SwarmRobt_h
#define SwarmRobt_h

#define _DEBUG 0
#if _DEBUG
#define _DEBUG_MAIN 0
#endif

/*Definition of robot: */
#define ROBOT_ID_INVALID 0X0
#define ROBOT_ID_ANY     0XFF
#define ROBOT_ID_SELF 0XFA  //todo,  if there ff, it will have good to detecting start bits.

#define RUNING_LED_PIN 13

#define SUCCESS 0
enum SW_ERRINFO
{
    ERR_ASSERT = 0x01,
    ERR_DEBUG = 0x02,

    ERR_PARA = 0x08,

    INFO_BUSY = 0X80
};


#define NULL 0
#if _DEBUG
    #define ASSERT_T(A) if( !(A) ) { Serial.println("ERROR"); }
#else
    #define ASSERT_T(A)
#endif



#endif
