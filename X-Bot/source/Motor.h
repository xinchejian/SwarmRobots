#ifndef Motor_h
#define Motor_h

#include <inttypes.h>

#if _DEBUG
#define _DEBUG_MT 0
#endif


#define MOTOR_L_EN_PIN 9     //to control the speed by PWM(connecting to LM EN pin)
#define MOTOR_L_DIRPINA 7  //The control direction pin of left motor (connecting to LM 1A pin)
#define MOTOR_L_DIRPINB 6
#define MOTOR_R_EN_PIN 9
#define MOTOR_R_DIRPINA 4
#define MOTOR_R_DIRPINB 5


//#define MT_DUTY(Val) _SFR_MEM16(OCR1A_MEM) = (uint16_t)((uint32_t)(Val)*ICR1)/255
#define MT_DUTY(Val) OCR1A = (uint16_t)( ((uint32_t)(Val)*ICR1)>>8 )




/* Near Distance = the distance of near detecting = as 1.5 times as lenth of robot, on the condition of suggest material
 */

#define MT_STEP_ONE 400   // 2cm

#define MT_ROTATE_ONE 400  // 10degre  180degree = 18*350

#define MT_ANGEL_UNIT 10    //10degre

#define MT_DEGREE(a) ((a)/MT_ANGEL_UNIT)

typedef enum
{
    MT_MOV_NULL,
    MT_STOP,
    MT_FORWARD,
    MT_BACKWARD,
    MT_CLOCKWISE,
    MT_ANTICLOCK,
    MT_CLOCKRANDOM,   //compoud moving
    MT_SWING

}MTMovDir_enum;


/*to control the duty of PWM. the value must be less than 255
 * it is unusable for no enable function chip, eg:LM1836
 * */
typedef enum MT_SPEED
{
    MT_SPEED_ZERO = 0,
    MT_SPEED_SLOW = 230,    //  duty = MT_SLOW/255
    MT_SPEED_NORMAL = 250,
    M_SPEED_FAST = 255    
}MTSpeed_enum;

#define ANGLE_RANDOM 0  //0-180 degree


/*private define*/
typedef enum MT_SINGLE_ACTION
{
    MT_ACTION_FORWORD,
    MT_ACTION_BACKWORD,
    MT_ACTION_STOP
}MTSingleAct_enum;

#define MOTOR_NUM 2
#define MOTOR_INDEX_L  0
#define MOTOR_INDEX_R  1   // must be < MOTOR_NUM

typedef struct MOTOR_PIN
{
    uint8_t DriverA;
    uint8_t DriverB;
    uint8_t Enable;  //it is unusable for no enable function chip, eg:LM1836
}MotorPin_stru;

/*    */
class BiMotor
{
private:


    MotorPin_stru MotorPin[MOTOR_NUM];
    MTSpeed_enum SpeedVal;
    uint32_t RunCnt;   //to control the distance or angle

    MTMovDir_enum LastMoveDirection;  //only base direction: MT_FORWARD,MT_BACKWARD,MT_CLOCKWISE,MT_ANTICLOCK,MT_STOP
    uint8_t LastMovePara;
    uint8_t ContinueFowardCnt;


    inline void Run();
    inline void Stop();
    inline void SetSpeed( MTSpeed_enum Speed );
    inline void SetMoveDistance( uint8_t Distance );
    inline void SetRotateAngle( uint8_t Angle );

    inline void Walk( MTMovDir_enum Direction, uint8_t Distance = 1, MTSpeed_enum Speed = MT_SPEED_NORMAL );
    inline void Rotate( MTMovDir_enum Direction, uint8_t Angle = 9, MTSpeed_enum Speed = MT_SPEED_NORMAL );

    void SetSingleMotorAction( uint8_t Index, MTSingleAct_enum Action );

protected:


public:
    BiMotor();
    void Move( MTMovDir_enum Direction, uint8_t Para );
    bool isMoving();
    void GetMoveAciton( MTMovDir_enum &Direction, uint8_t &Para );
    void TimerProc();
};

#endif
