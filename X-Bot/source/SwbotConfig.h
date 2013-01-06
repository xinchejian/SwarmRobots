/*public header*/

#ifndef SwbotConfig_h
#define SwbotConfig_h

/**Control whethe there is a test of Motors and IR**/
#define _START_TEST 1

/***Define the robot properties. !!!Attention: The ID must be diffrent from each other ***/
#define ROBOT_INIT_TYPE  ROBOT_TYPE_ANT  //the type refer to SwarmRobot.h   POWERSTATION 
#define ROBOT_GROUP 1   //It is less than pow(2,ROBOT_GROUP_BITS) = 8.
#define ROBOT_INNERID 3 //must be less than pow(2,ROBOT_INNERID_BITS) = 32. 0 is reserved

/***Define the intensity of IR-LED. The value is alse the duty of PWM = ( IR_POWER_DUTY/255 ) ***/
#define IR_POWER_DUTY 25 //2  20


#endif





