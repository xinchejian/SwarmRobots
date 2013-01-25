#ifndef Ledlight_h
#define Ledlight_h


#include <inttypes.h>

#if _DEBUG
#define _DEBUG_LED 0
#endif

/***Definition of LED: ***/
//high = light
#define LED_INFO_L SCK  //SCK
#define LED_INFO_R 3

/*Below is also used as the indicator of IR receiver; So they only can be used as self-check and gameover-phase */
#define LED_FL_PIN A3
#define LED_FR_PIN A2
#define LED_BL_PIN A0
#define LED_BR_PIN A1


#define LED_STATIC 0XFF

typedef struct LedInfoRec
{
    uint8_t Pin;
    uint8_t Mode;      //reverse, on, off
    uint8_t State;      //HIGH or LOW
    uint8_t CycleTime; //uint: 100ms, if = LED_STATIC 255 means continue;  if 0 mean disable
    uint8_t LastTime;
}LedInfoRec_stru;



typedef enum
{
    LED_MODE_MANUAL,
    LED_MODE_TRANSIENT,

    LED_MODE_START,
    LED_MODE_GAMEWIN,
    LED_MODE_RUNNING,
    LED_MODE_ERROR,

    LED_MODE_NONE
}LedMode_enum;

typedef enum
{
    SINGLE_LED_NULL,
    SINGLE_LED_STATIC,
    SINGLE_LED_REVERSE
}SingleLedMode_enum;


class Ledlight_cls {

private:
    static LedInfoRec_stru LedInfoTbl[];
    static uint8_t LedIRTbl[];

    uint8_t NumOfInfoTbl;
    uint8_t LedMode;
    uint8_t LedPara;
    uint8_t RunTime;       //unit: 100ms

protected:

public:
    Ledlight_cls();
    //void SetSingleLed( uint8_t Pin, uint8_t Time, uint8_t SingleMode );
    void SetLed( LedMode_enum Mode, uint8_t Para = 0 );
    void QueryTimeProc( uint8_t TimeOn100ms );

};

#endif
