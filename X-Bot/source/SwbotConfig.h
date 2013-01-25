/*Config file.  The macro marked by '@' should be set*/

#ifndef SwbotConfig_h
#define SwbotConfig_h


#define _START_TEST 0  //@ Control whether there is a test of Motors and IR
#define _START_REMOTE_CONTROL 0  //@ Control whether the robot is activated by remote controller (such as TV controller)

/***Define the role of robot. ***/
#define ROBOT_INIT_TYPE  ROBOT_TYPE_ANT_LINEUP   //@The type refer to SwarmRobot.h

/***Define the robot ID. !!!Attention: The ID must be different from each other ***/
#define ROBOT_GROUP 1    //@ Set the No. of your group, it must be less than pow(2,ROBOT_GROUP_BITS) = 8
#define ROBOT_INNERID 1  //@ Set the internal ID in one group. it must be less than pow(2,ROBOT_INNERID_BITS) = 32

/***Define the intensity of IR-LED***/
#define IR_POWER_DUTY 25        //@ Define the intensity of IR-LED. The value is also the duty of PWM = ( IR_POWER_DUTY/255 )
#define IR_POWER_NEARTTEHTH 4   //@ Define the relative intensity of near range.  The Near Power = IR_POWER_DUTY*(IR_POWER_NEAR_TENTH/10)



#define IR_POWER_OBSTACLE_LEASTTEHTH 7  // IR_OBSTACLE_LEASTTEHTH/10*IR_POWER_DUTY

#endif





