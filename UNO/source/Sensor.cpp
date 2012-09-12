/*
                          IR-Swarmbot Software

    Copyright (C) <2012>  <Leo Yan>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Contract: leo.yan.cn@gmail.com
*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include "PWM.h"
#include "SwarmRobot.h"
#include "Sensor.h"

/* Analysis of critical resource:
1.
critical resource:  RxBuf - (type is struct)

main thread:   GetMessage()-> if state == RX_UNOBSTACLE || RX_MSG_OK ->SetRxBufState() -> state = RX_MSG_EMPTY
timer thread:  ReceiveData()-> if state == RX_MSG_EMPTY -> SetRxBufState() -> state = !RX_MSG_EMPTY

$ conclude:
1. no constraint

 */



//
// Constructor
//
IR_Sensor::IR_Sensor(uint8_t RobotID)
{
    uint8_t i;

    PhaseCur = 0;

    /*initialize inner data*/
    SendCnt = IR_TXRX_REPEATTIMES;

    TxCtrl.State = TX_EMPTY;
    TxBuf.StartBits = 0x00;
    TxBuf.SenderID = RobotID;


    RxPin[IR_POSITION_FL] = IR_R_FL_PIN;
    RxPin[IR_POSITION_FR] = IR_R_FR_PIN;
    RxPin[IR_POSITION_BL] = IR_R_BL_PIN;
    RxPin[IR_POSITION_BR] = IR_R_BR_PIN;

    for( i=0; i<IR_RECIEVER_NUM; ++i )
    {
        RxCtrl[i].State = RX_EMPTY;
        RxCtrl[i].ByteCursor = 0;
        RxCtrl[i].BitCursor = IR_COMM_BITCUR_INIT;   //record start bits
        RxCtrl[i].RunPhase = IR_Phase_RXALL;

        pinMode(RxPin[i],INPUT);
    }

    /*initialize emitter*/
    TxPin = IR_E_PIN;
    TxCtrl.RunPhase = IR_Phase_TX;

    pinMode(TxPin, OUTPUT);

    randomSeed(millis());
    

}

//
// Destructor
//
IR_Sensor::~IR_Sensor()
{
  ;
}

/*to send data at a random time;
 * SendDuration = duration of sending a massage,
 * randon time =  SendDuration*random(0-1))
 * ? todo: need different chance;  send msg differ from collide
 * */
void IR_Sensor::SetSendChance()
{
    static const uint8_t SendMessageCycle = (IR_BUFF_LENTH * 8);
    uint8_t Var;
#if _DEBUG_SENSOR
    SendEnableCounter = 0;
#else
    SendEnableCounter = random(3, SendMessageCycle);  //todo
#endif

    TxCtrl.ByteCursor = 0;
    TxCtrl.BitCursor = IR_COMM_BITCUR_INIT;
}

/*
 *
 *
 */
uint8_t IR_Sensor::SendMessage(uint8_t ReceiverID, uint8_t MessageID, uint8_t Para)
{
    /*use the state of 0-receiver to judge the rx-buf whether is be fetch by Messager*/
    if ( (TX_EMPTY != TxCtrl.State) || ( RX_EMPTY != RxCtrl[0].State) )
    {
        return INFO_BUSY;
    }

    TxBuf.ReceiverID = ReceiverID;
    TxBuf.MessageID = MessageID;
    TxBuf.Para = Para;
    TxBuf.VerifyFirst = TxBuf.ReceiverID;
    TxBuf.VerifyLast = TxBuf.Para;

    SendCnt = IR_TXRX_REPEATTIMES;
    EnableSend();

    return SUCCESS;
}

uint8_t IR_Sensor::GetMessage(uint8_t Index, uint8_t *pSenderID, uint8_t *pMessageID, uint8_t *pPara, uint8_t *pMessageState )
{
    if ( Index >=  IR_RECIEVER_NUM )
    {
        return ERR_PARA;
    }

    if ( TX_EMPTY != TxCtrl.State )
    {
        return INFO_BUSY;
    }

    *pMessageState = RxCtrl[Index].State;

    if ( RX_MSG_OK == RxCtrl[Index].State )
    {
        *pSenderID = RxBuf[Index].SenderID;
        *pMessageID = RxBuf[Index].MessageID;
        *pPara = RxBuf[Index].Para;
    }

    if  ( (RX_UNOBSTACLE == RxCtrl[Index].State) || (RX_MSG_OK == RxCtrl[Index].State) )
    {
        SetRxBufState( Index, RX_EMPTY );
    }



    return SUCCESS;
}


inline void IR_Sensor::EnableSend(void)
{
    SetSendChance();
    TxCtrl.TTLCounter = IR_DATA_TTL;
    TxCtrl.State = TX_READY;  //State must be set at the end of the function to avoid interrupt.
}

/*The frame format of communication: (use the output of IR-receiver as the reference)
 * 3 start bits 0, sizeof(TxBuf_stru)*8 bits data, 16 bits verify data
 * use ASC2 coding
 * verify: send repeatly the first and last bytes of 'TxBuf_stru'
 * 38k carrier wave, the frequency of base signal must be under 19k. IR-receiver:min signal paulse width is ？. chose 500us,So baud is 1000ms/0.5ms=2KHz
 * The bits of message = 51 bits.   2K/51=39, 39 messages per second.
 */
inline void IR_Sensor::SendData(void)
{
    uint8_t i, BitVal;
    uint8_t &ByteCur = TxCtrl.ByteCursor;
    uint8_t &BitCur = TxCtrl.BitCursor;
    uint8_t * pBuf = NULL;

    if (TX_DONE == TxCtrl.State)
    {
        SendBit(1); // close IR emitting

        SendCnt--;

        if ( 0 == SendCnt )
        {
            AdjustRxStateAfterSend();
            TxCtrl.State = TX_EMPTY;
        }
        else
        {
            EnableSend();
        }
    }

    if (TX_EMPTY == TxCtrl.State)
    {
        return;
    }

    if ( 0 < SendEnableCounter )
    {
        SendEnableCounter --;
        return;
    }


    if ( TX_READY == TxCtrl.State )
    {
        TxCtrl.State = TX_SENDING;
    }


    //send date

    pBuf = (uint8_t*)&TxBuf;

    BitVal = bitRead(pBuf[ByteCur], BitCur);

    SendBit(BitVal);  //pinMode interference with receive

    BitCur++;

    if ( BitCur >= 8 )
    {
        ByteCur++;
        BitCur = 0;
    }

    if ( ByteCur >= IR_BUFF_LENTH )
    {
        TxCtrl.State = TX_DONE;
    }

}


inline void IR_Sensor::SendBit(bool BitVal)
{
    static uint8_t LowVal = ((uint32_t)IR_POWER_DUTY*ICR1)/255;
    static uint8_t HighVal = 0;

    uint8_t Val;

    Val = BitVal ? HighVal : LowVal;
    _SFR_MEM16(OCR1B_MEM) = Val;

    
}


/*ReceiveData
 *
 *sampling frequency is 5 times of sending frequency.
 *
 *when it is sending, the robot will in near detecting state whether it receive signal or not
 *
 */
/*todo
if NEAR_DETECTING do refresh the near-message. if receive nothing the message is no obstacle; using TTL?
.  how to avoid continuing initalization?
*/

inline void IR_Sensor::ReceiveData( uint8_t Index )
{
    uint8_t PinVal;
    uint8_t &BufState = RxCtrl[Index].State;
    uint8_t &BitCur = RxCtrl[Index].BitCursor;
    uint8_t &ByteCur = RxCtrl[Index].ByteCursor;
    uint8_t &Phase = RxCtrl[Index].RunPhase;

    PinVal = digitalRead( RxPin[Index] );

    /*if the robot receive anyting(0) from others do initalize sending window */
    if ( ((TX_EMPTY == TxCtrl.State) || (TX_READY == TxCtrl.State)) && (LOW == PinVal) )
    {
        SetSendChance();
    }

    /*receive start bits. if there is the first start bit, to record the sample phase*/
    if ( RX_EMPTY == BufState )
    {
        if ( LOW == PinVal )
        {
            Phase = PhaseCur;   //if (IR_Phase_RXALL == Phase)
            ++BitCur;
        }
        else
        {
            Phase = IR_Phase_RXALL;  // if (IR_Phase_RXALL != Phase)
            BitCur = IR_COMM_BITCUR_INIT;
        }

        if ( 8 <= BitCur )
        {
            SetRxBufState( Index, RX_RECEIVING );
        }
    }
    else if ( RX_RECEIVING == BufState )
    {
        uint8_t *pBuf = NULL;
        pBuf = (uint8_t *)&RxBuf[Index];

        bitWrite( pBuf[ByteCur], BitCur, PinVal );

        BitCur++;

        if ( BitCur >= 8 )
        {
            ByteCur++;
            BitCur = 0;
        }

        if ( ByteCur >= IR_BUFF_LENTH )
        {
            RXBufState_enum State;

            State = (VerifyData( Index )) ? RX_MSG_OK : RX_EMPTY;
            SetRxBufState( Index, State );
        }
    }
    else
    {
        //nothing
    }

#if _DEBUG_SENSOR
    Serial.print("Rev bit:");
    Serial.print("index=");
    Serial.print(Index);
    Serial.print(" state=");
    Serial.print(BufState);
    Serial.print(" val=");
    Serial.print(PinVal);
    Serial.print(" bytecur=");
    Serial.print(ByteCur);
    Serial.print(" bitcur=");
    Serial.println(BitCur);
#endif

}


inline void IR_Sensor::AdjustRxStateAfterSend()
{
    uint8_t i;

    for( i=0; i<IR_RECIEVER_NUM; ++i )
    {
        if ( RX_EMPTY == RxCtrl[i].State )
        {
            SetRxBufState( i, RX_UNOBSTACLE );
        }
    }
}


inline void IR_Sensor::SetRxBufState( uint8_t Index, uint8_t State )
{
    uint8_t &BufState = RxCtrl[Index].State;
    uint8_t &BitCur = RxCtrl[Index].BitCursor;
    uint8_t &ByteCur = RxCtrl[Index].ByteCursor;
    uint8_t &TTLCnt = RxCtrl[Index].TTLCounter;

    BufState = State;

    switch( State )
    {
        case RX_EMPTY:
            BitCur = IR_COMM_BITCUR_INIT;
            RxCtrl[Index].RunPhase = IR_Phase_RXALL;
            ByteCur = 0;
            break;

        case RX_RECEIVING:
            BitCur = 0;
            ByteCur++;
            break;

        case RX_MSG_OK:
        case RX_UNOBSTACLE:
            TTLCnt = IR_DATA_TTL;
            break;

        default:
            ;//error
    }
}


inline bool IR_Sensor::VerifyData( uint8_t Index )
{
    bool rslt;
    RxBuf_stru &Buf = RxBuf[Index];
    
    rslt = (Buf.ReceiverID == Buf.VerifyFirst) && (Buf.Para == Buf.VerifyLast) && (0xFF != Buf.VerifyLast);

    //todo: if receiveID is not ALL or SELF, to empty the rx buff

    return rslt;
}

/*todo: when sending data, if colliding occurs, should?
 *
 *
 *
 */

/* TTL
 * void TTLCaculate();
 *
 *
 */
inline void IR_Sensor::TTLCaculate()
{


    // if RxBuff ttl is reached, BuffState is set to EMPTY
}

uint32_t IR_Sensor::GetTimerFrequency(void)
{
    return TIMER_FREQUENCY;
}

/*the function is called by TimerInterrupt. the frenqucy is equals to GetTimerFrequency()
 *
 */

void IR_Sensor::TimerProc()
{
    uint8_t Phase;
    uint8_t i;

    /*receive data*/
    for ( i = 0; i < IR_RECIEVER_NUM; ++i ) 
    {
        Phase = RxCtrl[i].RunPhase;
        if ( (IR_Phase_RXALL == Phase) || (PhaseCur == Phase) )
        {
            ReceiveData(i);
        }
    }

    /*send data*/
    Phase = TxCtrl.RunPhase;

    if ( Phase == PhaseCur )
    {
        SendData();
    }

    PhaseCur = (PhaseCur >= SAMPLE_TIMES) ? 0 : ++PhaseCur;
}

