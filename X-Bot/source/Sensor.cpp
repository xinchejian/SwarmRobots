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

    Contact: leo.yan.cn@gmail.com
*/

#include "Arduino.h"
#include "SwarmRobot.h"
#include "Sensor.h"

/* Analysis of critical resource:
1.
critical resource:  RxBuf - (type is struct)

main thread:   GetMessage()-> if state == RX_MSG_OK ->SetRxBufState() -> state = RX_MSG_EMPTY
timer thread:  ReceiveData()-> if state == RX_MSG_EMPTY -> SetRxBufState() -> state = !RX_MSG_EMPTY
$ conclude:
1. no constraint

2. message cycle quene: headindex tailindex
 timer - write,
 main  - read
$ conclude: if overlap is prohibited
1. no constraint


 */

#if ( IR_POSITION_BR >= IR_RECIEVER_NUM )
#error
#endif

#if ( (9 != IR_E_PIN) && (10 != IR_E_PIN) )
#error
#endif


//
// Constructor
//
IR_Sensor::IR_Sensor(uint8_t RobotID)
{
    uint8_t i;

    PhaseCur = 0;
    IRPower = IR_POWER_DUTY;

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
        RxCtrl[i].PhaseFlag = true;

        pinMode(RxPin[i], INPUT_PULLUP); //INPUT_PULLUP
    }

    /*initialize emitter*/
    TxPin = IR_E_PIN;

    if ( 9 == IR_E_PIN )
    {
        pEmitterAddr = &OCR1A;
    }
    else
    {
        pEmitterAddr = &OCR1B;
    }

    TxCtrl.RunPhase = IR_Phase_TX;

    pinMode(TxPin, OUTPUT);

    randomSeed(millis());
    
    /**/
    MsgFifo.Head = QUE_CURSOR_INVALID;
    MsgFifo.Tail = 0;
}


void IR_Sensor::InitFrequence( uint32_t f )
{
    static const uint16_t pscLst[] = { 0, 1, 8, 64, 256, 1024};

    if(f > 2000000 || f < 1)
    {
        ASSERT_T(0);
        return;
    }

    /**setting the waveform generation mode of two channels: Phase correct PWM mode**/
    uint8_t wgm = 8;

    TCCR1A = (TCCR1A & B11111100) | (wgm & 3);
    TCCR1B = (TCCR1B & B11100111) | ((wgm & 12) << 1);

    bitSet(TCCR1A, COM1A1);
    bitSet(TCCR1A, COM1B1);

    /****/



    //find the smallest usable multiplier
    uint16_t multiplier = (uint16_t)(F_CPU / (2 * f * 65535));

    byte iterate = 0;
    while(multiplier > pscLst[iterate++]);

    multiplier = pscLst[iterate];

    //Set timer top,  it determine the frenquence.
    ICR1 = (uint16_t)(F_CPU/(2* f * multiplier));

    //Set Prescaler
    TCCR1B = (TCCR1B & ~7) | (iterate & 7);

    /**close emitter**/
    *pEmitterAddr = 0;

}


/*to send data at a random time;
 * SendDuration = duration of sending a massage,
 * randon time =  SendDuration*random(0-1))
 * ? todo: need different chance;  send msg differ from collide
 * */
void IR_Sensor::SetSendChance()
{
    static const uint8_t SendMessageCycle = (IR_BUFF_LENTH * 8);

    SendEnableCounter = random(16, SendMessageCycle);  //todo
}

/*If the IRPower is less than (IR_POWER_OBSTACLE_LEASTTEHTH/10 * IR_POWER_DUTY), it cannot be used as to detect obstacles.
 *IRPowerTenth < 3
 *
 */
uint8_t IR_Sensor::SendMessage(uint8_t ReceiverID, uint8_t MessageID, uint8_t Para, uint8_t IRPowerTenth)
{

    if ( TX_EMPTY != TxCtrl.State )
    {
        return INFO_BUSY;
    }

    isDetectObstacle =  (IRPowerTenth >= IR_POWER_OBSTACLE_LEASTTEHTH) ? true : false;

    IRPower = ((uint16_t)IRPowerTenth * IR_POWER_DUTY)/10;

    TxBuf.ReceiverID = ReceiverID;
    TxBuf.SenderID = ROBOT_ID_SELF;
    TxBuf.MessageID = MessageID;
    TxBuf.Para = Para;
    TxBuf.Check = CaculateCheckData( (uint8_t *)&TxBuf + 1,sizeof(TxBuf) - 2 );


    SendCnt = IR_TXRX_REPEATTIMES;
    EnableSend();

    return SUCCESS;
}



uint8_t IR_Sensor::GetMessage(IRPosition_enum *pIRLoc, IRMsgOutput_stru *pMsg )
{
    uint8_t Index;
    uint8_t Ret;

    Ret = GetFromMsgQue( &Index );
    if ( SUCCESS != Ret )
    {
        return Ret;
    }

    *pIRLoc = (IRPosition_enum)Index;

    if ( Index < IR_RECIEVER_NUM )
    {
        //Data MSG
        ASSERT_T( RX_MSG_OK == RxCtrl[Index].State );

        pMsg->SenderID = RxBuf[Index].SenderID;
        pMsg->MessageID = RxBuf[Index].MessageID;
        pMsg->Para = RxBuf[Index].Para;
        pMsg->Check = RxBuf[Index].Check;

        SetRxBufState( Index, RX_EMPTY );
    }
    else  //sending data is done,  control MSG
    {
       ;
    }

    return SUCCESS;
}




inline void IR_Sensor::EnableSend(void)
{
    SetSendChance();

    TxCtrl.ByteCursor = 0;
    TxCtrl.BitCursor = IR_COMM_BITCUR_INIT;

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
    uint8_t BitVal;
    uint8_t &ByteCur = TxCtrl.ByteCursor;
    uint8_t &BitCur = TxCtrl.BitCursor;
    uint8_t * pBuf = NULL;

    if (TX_DONE == TxCtrl.State)
    {
        SendBit(1); // close IR emitting

        SendCnt--;

        if ( 0 == SendCnt )
        {
            TxCtrl.State = TX_EMPTY;  //这里可以保证发送完毕且所有的接收器都接受完毕后，才会置空，起到了对临界区rxbuf的保护
            if ( isDetectObstacle )
            {
                /*Send ctl msg: a frame of msgs is end*/
                (void)PutToMsgQue( IR_POSITION_CTL );
            }
        }
        else
        {
            EnableSend();
        }

        return;
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
    uint8_t Val;

    Val = BitVal ? 0 : IRPower;
    *pEmitterAddr = Val;   
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
    bool &PhaseFlag = RxCtrl[Index].PhaseFlag;
    uint8_t Val, Ret;

    if ( false == PhaseFlag )
    {
        PhaseFlag = true;
        return;
    }

    PinVal = digitalRead( RxPin[Index] );

    /*if the robot receive anyting(0) from others do initalize sending window */
    if ( (TX_READY == TxCtrl.State) && (LOW == PinVal) )
    {
        SetSendChance();
    }

    /*receive start bits. if there is the first start bit, to record the sample phase*/
    if ( RX_EMPTY == BufState )
    {
      
        if ( LOW == PinVal )
        {
            if ( IR_COMM_BITCUR_INIT == BitCur )
            {
                Val = PhaseCur + SAMPLE_OFFSET;
                Phase = (Val >= SAMPLE_TIMES) ? 0 : Val;

                PhaseFlag = false;
            }

            ++BitCur;
        }
        else
        {
            Phase = IR_Phase_RXALL;
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

            State = VerifyRecData( Index ) ? RX_MSG_OK : RX_EMPTY;

            if ( RX_MSG_OK == State )
            {
                Ret = PutToMsgQue( Index );
                if ( SUCCESS != Ret )
                {
                    State = RX_EMPTY;
                }
            }

            SetRxBufState( Index, State );
        }
    }
    else
    {
        //nothing
    }

#if _DEBUG_SENSOR_NOTIMER
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




inline void IR_Sensor::SetRxBufState( uint8_t Index, uint8_t State )
{
    uint8_t &BufState = RxCtrl[Index].State;
    uint8_t &BitCur = RxCtrl[Index].BitCursor;
    uint8_t &ByteCur = RxCtrl[Index].ByteCursor;

    switch( State )
    {
        case RX_EMPTY:          //it will set by main thread
            BitCur = IR_COMM_BITCUR_INIT;
            RxCtrl[Index].RunPhase = IR_Phase_RXALL;
            ByteCur = 0;

            break;

        case RX_RECEIVING:
            BitCur = 0;
            ByteCur++;

            break;

        case RX_MSG_OK:
            //nothing, msg has been pushed to QUE
            break;

        default:
            ASSERT_T(0);  //error
    }
    
    BufState = State;   //RxCtrl is critical source, use RxCtrl.state as a signal, it must be put the last!

}


uint8_t IR_Sensor::CaculateCheckData( uint8_t * pData, uint8_t ByteLen )
{
    ASSERT_T( NULL != pData );

    uint8_t Sum, i;

    Sum = 0xAA;
    for ( i = 0; i < ByteLen; ++i )
    {
        Sum = Sum ^ pData[i];
    }

    return Sum;
}

bool IR_Sensor::VerifyRecData( uint8_t Index )
{
    uint8_t CheckData;


    CheckData = CaculateCheckData( (uint8_t *)&RxBuf[Index] + 1,sizeof(RxBuf[Index]) - 2 );

    if ( CheckData != RxBuf[Index].Check )
    {
#if _DEBUG_SENSOR_NOTIMER
    Serial.print("Rec Check:");
    Serial.print(RxBuf[Index].Check,HEX);
    Serial.print(" Calc Check:");
    Serial.println(CheckData,HEX);
#endif
        return false;
    }



    if (ROBOT_ID_SELF == RxBuf[Index].SenderID )
    {  //using near detecting
        ;
    }
    else
    {
        uint8_t &ReceiverID = RxBuf[Index].ReceiverID;

        if ( (ROBOT_ID_SELF == ReceiverID) || ( ROBOT_ID_ANY == ReceiverID ) )
        {
            ;
        }
        else
        {
            return false;
        }
    }

    return true;
}



uint16_t IR_Sensor::GetTimerFrequency(void)
{
    return TIMER_FREQUENCY;
}

inline void IR_Sensor::SetIRPower( uint8_t NumOfTenth )
{
    uint16_t Val;
    Val = ((uint16_t)NumOfTenth * IR_POWER_DUTY)/10;

    IRPower = (uint8_t)((Val*ICR1)/255);
}


/*the function is called by TimerInterrupt. the frenqucy is equals to GetTimerFrequency()
 *
 */

#if _DEBUG_TIMER

IRTimerTest_stru gIRTimerTest={0,0};

uint16_t gTimerPeriodBegin = 0;
uint16_t gTimerPeriodEnd = 0;

uint8_t gThread = (1000000/SAMPLE_TIMES/COMM_FREQUENCY) >> 2;

#endif

void IR_Sensor::TimerProc()
{
    uint8_t Phase;
    uint8_t i;

#if _DEBUG_TIMER
        gIRTimerTest.Who = 0;
        for ( i = 0; i < IR_RECIEVER_NUM; ++i )
        {
            Phase = RxCtrl[i].RunPhase;
            if ( (IR_Phase_RXALL == Phase) || (PhaseCur == Phase) )
            {
                gIRTimerTest.Who |= 0x1 << i;
            }
        }

        Phase = TxCtrl.RunPhase;

        if ( Phase == PhaseCur )
        {
            gIRTimerTest.Who |= 0b10000;
        }
        
    gTimerPeriodBegin = (uint16_t)micros();
#endif

    /*receive data*/
    for ( i = 0; i < IR_RECIEVER_NUM; ++i ) //
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
    
    PhaseCur ++;

    if (PhaseCur >= SAMPLE_TIMES) 
    {
       PhaseCur = 0;
    }

#if _DEBUG_TIMER
    uint8_t Val;

    gTimerPeriodEnd = (uint16_t)micros();

    Val = (gTimerPeriodEnd - gTimerPeriodBegin) >> 2;

    if ( Val > gIRTimerTest.Period )
    {
        gIRTimerTest.Period = Val;
    }

    if ( Val >= gThread ) gIRTimerTest.Cnt++;
#endif
}

uint8_t IR_Sensor::PutToMsgQue(uint8_t Index)
{
    uint8_t &Head = MsgFifo.Head;
    uint8_t &Tail = MsgFifo.Tail;
    uint8_t LastTail, Val;

    if ( Tail == Head )
    {
        return INFO_QUE_FULL;
    }

    MsgFifo.aData[Tail] = Index;

    LastTail = Tail;

    Val = Tail;
    Val++;
    Tail = (Val >= IR_MSG_QUE_NUM) ? 0 : Val;  //to avoid division for efficence.

    if ( QUE_CURSOR_INVALID == Head )   //to assure the 'Tail' is present unused rec before assignment to 'Head'.
    {
        Head = LastTail;
    }

    return SUCCESS;
}

/*GetFromQueGet in main thread, it may be interrupted by PutToMsgQue*/
uint8_t IR_Sensor::GetFromMsgQue(uint8_t *pIndex)
{
    uint8_t &Head = MsgFifo.Head;
    uint8_t &Tail = MsgFifo.Tail;
    uint8_t Val;


    if ( QUE_CURSOR_INVALID == Head )
    {
        return INFO_QUE_EMPTY;
    }

    *pIndex = MsgFifo.aData[Head];

    Val = Head;  //must use temporary varible to reduce the critical region of "Head".  To aviod 'Head == Tail' when Head is increased
    Val++;
    Val = (Val >= IR_MSG_QUE_NUM) ? 0 : Val;  //to avoid division for efficence.

    /*critical code; if unprotected, a message is maybe lost */
    noInterrupts();  
    Head = (Val == Tail) ? QUE_CURSOR_INVALID : Val;
    interrupts();

    return SUCCESS;

}

#if _DEBUG_SENSOR
void IR_Sensor::PrintMsgQue()
{
    uint8_t Cursor = MsgFifo.Head;
    uint8_t Tail = MsgFifo.Tail;
    uint8_t Index, i;

    Serial.print("MsgQue: H=");
    Serial.print(Cursor);
    Serial.print(" T=");
    Serial.println(Tail);

    for ( i = 0 ; i < IR_MSG_QUE_NUM; ++i )
    {
        if (QUE_CURSOR_INVALID == Cursor)  break;
        
        Index = MsgFifo.aData[Cursor];
        Serial.print("Rx- I=");
        Serial.print(Index);
        
        if ( Index < IR_RECIEVER_NUM )
        {
          Serial.print(" S=");
          Serial.print(RxCtrl[Index].State);
          Serial.print(" ByteC=");
          Serial.print(RxCtrl[Index].ByteCursor);
          Serial.print(" BitC=");
          Serial.print(RxCtrl[Index].BitCursor);
          Serial.print(" P=");
          Serial.print(RxCtrl[Index].RunPhase);
          Serial.print(" G=");
          Serial.print(RxCtrl[Index].PhaseFlag);
        }
        
        Serial.println("");

        Cursor ++;
        if ( Cursor >= IR_MSG_QUE_NUM ) Cursor = 0;
        
        if (Cursor ==  Tail) break;

    }

}

void IR_Sensor::PrintCtl()
{
    uint8_t i;

    for ( i = 0 ; i < IR_RECIEVER_NUM; ++i )
     {
           Serial.print(" S=");
           Serial.print(RxCtrl[i].State);
           Serial.print(" ByteC=");
           Serial.print(RxCtrl[i].ByteCursor);
           Serial.print(" BitC=");
           Serial.print(RxCtrl[i].BitCursor);
           Serial.print(" P=");
           Serial.print(RxCtrl[i].RunPhase);
           Serial.print(" G=");
           Serial.print(RxCtrl[i].PhaseFlag);
           Serial.println("");
     }

}

#endif

