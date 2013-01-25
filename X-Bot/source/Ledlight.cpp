
#include "Arduino.h"
#include "SwarmRobot.h"
#include "Ledlight.h"


LedInfoRec_stru Ledlight_cls::LedInfoTbl[]= { 
    {LED_INFO_L, 0, 0, 0, 0}, 
    {LED_INFO_R, 0, 0, 0, 0},
  };
uint8_t Ledlight_cls::LedIRTbl[]={ LED_FL_PIN, LED_FR_PIN, LED_BR_PIN, LED_BL_PIN };


Ledlight_cls::Ledlight_cls()
{
    uint8_t i;

    NumOfInfoTbl = sizeof(LedInfoTbl) / sizeof(LedInfoTbl[0]);

    for ( i = 0; i < NumOfInfoTbl; ++i )
    {
        LedInfoTbl[i].Mode = SINGLE_LED_NULL;
        LedInfoTbl[i].LastTime = 0;
        pinMode(LedInfoTbl[i].Pin, OUTPUT);
    }

    LedMode = LED_MODE_NONE;
    LedPara = 0;
    RunTime = 0;

}

void Ledlight_cls::SetLed( LedMode_enum Mode, uint8_t Para )
{
    uint8_t i;

    /**To prevent setting this repeatedly**/
    if ( (Mode == LedMode) && (Para == LedPara) )
    {
        return;
    }

    LedMode = Mode;
    LedPara = Para;


    switch (Mode)
    {

    case LED_MODE_MANUAL:
        for ( i = 0; i < NumOfInfoTbl; ++i )
        {
            LedInfoTbl[i].Mode = SINGLE_LED_STATIC;
            LedInfoTbl[i].State = (0 == bitRead(Para, i)) ? LOW : HIGH;
            LedInfoTbl[i].CycleTime = 0;
            LedInfoTbl[i].LastTime = 0;
        }

        break;
    case LED_MODE_START:
        uint8_t Num;

        digitalWrite( LED_INFO_L , HIGH );
        digitalWrite( LED_INFO_R , HIGH );

        Num = sizeof(LedIRTbl) / sizeof(LedIRTbl[0]);

        for (i = 0; i < Num; ++i )
        {
            pinMode( LedIRTbl[i], OUTPUT );
            digitalWrite( LedIRTbl[i] , LOW );
            delay(200);
        }

        for (i = 0; i < Num; ++i )
        {
            digitalWrite( LedIRTbl[i], HIGH );
            delay(200);
            pinMode( LedIRTbl[i], INPUT_PULLUP );
        }

        digitalWrite( LED_INFO_L , LOW );
        digitalWrite( LED_INFO_R , LOW );

        break;

    case LED_MODE_RUNNING:
        break;

    case LED_MODE_GAMEWIN:  //for island
        for ( i = 0; i < NumOfInfoTbl; ++i )
        {
            if ( LED_INFO_L == LedInfoTbl[i].Pin )
            {
                LedInfoTbl[i].Mode = SINGLE_LED_REVERSE;
                LedInfoTbl[i].State = LOW;
                LedInfoTbl[i].CycleTime = 2;
                LedInfoTbl[i].LastTime = 0;
            }
            else if ( LED_INFO_R == LedInfoTbl[i].Pin )
            {
                LedInfoTbl[i].Mode = SINGLE_LED_REVERSE;
                LedInfoTbl[i].State = HIGH;
                LedInfoTbl[i].CycleTime = 4;
                LedInfoTbl[i].LastTime = 0;
            }
        }

        break;
    default:
        ;

    }
}


void Ledlight_cls::QueryTimeProc( uint8_t TimeOn100ms )
{
    uint8_t i;
    uint8_t Pin,Mode,CycleTime;

    for ( i = 0; i < NumOfInfoTbl; ++i )
    {
        uint8_t &LastTime = LedInfoTbl[i].LastTime;
        uint8_t &State = LedInfoTbl[i].State;
        
        Pin = LedInfoTbl[i].Pin;
        Mode = LedInfoTbl[i].Mode;
        CycleTime = LedInfoTbl[i].CycleTime;

        switch ( Mode )
        {
        case SINGLE_LED_STATIC:

            digitalWrite( Pin, State );

            break;

        case SINGLE_LED_REVERSE:
            if ( (uint8_t)(TimeOn100ms - LastTime) >= CycleTime )
            {
                digitalWrite( Pin, State );
                State = (LOW == State) ? HIGH : LOW;
                LastTime = TimeOn100ms;
            }

            break;
        default:
            ;//nothing
        }

    }

}
