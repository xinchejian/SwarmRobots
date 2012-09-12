#ifndef Sensor_h
#define Sensor_h

/*

*/

#include <inttypes.h>

#if _DEBUG
#define _DEBUG_SENSOR 0
#endif

#define IR_POWER_DUTY 20
#define IR_TXRX_REPEATTIMES 2

/*To define the pins of MCU connected with sensors, you shoud set them in fact*/
#define IR_E_PIN 10
#define IR_RECIEVER_NUM 4
#define IR_R_FL_PIN 8
#define IR_R_FR_PIN 12
#define IR_R_BL_PIN 11
#define IR_R_BR_PIN A2  //cannot use 13 pin on arduino uno

//must less than IR_RECIEVER_NUM
typedef enum
{
    IR_POSITION_FL = 0,
    IR_POSITION_FR = 1,
    IR_POSITION_BL = 2,
    IR_POSITION_BR = 3
}IRPosition_enum;


/*To define the paras about communication*/
#define COMM_FREQUENCY 2000
#define SAMPLE_TIMES (IR_RECIEVER_NUM + 1)
#define TIMER_FREQUENCY (COMM_FREQUENCY * SAMPLE_TIMES)

//to balance the cpu, exuctive different task at different timer cycle
#define IR_Phase_TX ( SAMPLE_TIMES -1 )
#define IR_Phase_RXALL 0xFF

#define IR_COMM_START_BITNUM 2  //start bis number
#define IR_COMM_BITCUR_INIT ( 8 - IR_COMM_START_BITNUM )
#define IR_DATA_TTL 5    //TTL message in the buffer of ReceiveBuf_stru or TxBuf_stru

#define IR_CARRIER_FRENQUENCY 38000


/*To define the data format about communication*/
typedef struct{
    uint8_t StartBits;     //start bits, send from LSB
    uint8_t ReceiverID;    // 0 means the buff is null;  0xFF means all of robots
    uint8_t SenderID;
    uint8_t MessageID;
    uint8_t Para;
    uint8_t VerifyFirst;   // Verify bytes, the value should be equal to the value of the first byte
    uint8_t VerifyLast;    // Verify bytes, the value should be equal to the value of the last byte
}TxRxBuf_stru;

typedef TxRxBuf_stru TxBuf_stru;
typedef TxRxBuf_stru RxBuf_stru;

#define IR_BUFF_LENTH sizeof(TxRxBuf_stru)

typedef enum
{
    RX_EMPTY = 0,
    RX_RECEIVING,
    RX_MSG_OK,
    RX_UNOBSTACLE     // RECEIVE NOTHING means RX_UNOBSTACLE
}RXBufState_enum;

typedef enum
{
    TX_EMPTY = 0,
    TX_READY,
    TX_SENDING,
    TX_DONE
}TxBufState_enum;

/*To define control byte of the tx and rx buff*/
typedef struct {
    uint8_t State;
    uint8_t ByteCursor;   // 0 base
    uint8_t BitCursor;    // in a byte, from 0-7
    uint8_t TTLCounter;
    uint8_t RunPhase;
}TxRxCtrl_stru;

typedef TxRxCtrl_stru TxCtrl_stru;
typedef TxRxCtrl_stru RxCtrl_stru;



/* Class of Sensor */
class IR_Sensor {

  private:
  

    /*about TX*/
    uint8_t TxPin;
    TxBuf_stru    TxBuf;
    TxCtrl_stru   TxCtrl;
    uint8_t SendEnableCounter;  //there are perhaps colliding conditions
    uint8_t SendCnt;            //to control the sending times for one message

    /*about RX*/
    uint8_t RxPin[IR_RECIEVER_NUM];
    RxBuf_stru RxBuf[IR_RECIEVER_NUM];
    RxCtrl_stru RxCtrl[IR_RECIEVER_NUM];

    /*to divide cycle of communication into several phase, in order to sample the receive signal*/
    uint8_t PhaseCur;



    inline void SendBit(bool BitVal);
    void SetSendChance();

    inline void SetRxBufState( uint8_t Index, uint8_t State );
    inline bool VerifyData( uint8_t Index );

    inline void TTLCaculate();
    inline void AdjustRxStateAfterSend();
    inline void EnableSend(void);

#if !_DEBUG_SENSOR
    inline void SendData(void);
    inline void ReceiveData( uint8_t Index );
#endif

  protected:
  
  public:

    IR_Sensor(const uint8_t RobotID);
    ~IR_Sensor();
    uint8_t SendMessage(uint8_t ReceiverID, uint8_t MessageID, uint8_t para);
    uint8_t GetMessage(uint8_t Index, uint8_t *pSenderID, uint8_t *pMessageID, uint8_t *pPara, uint8_t *pMessageState );
    uint32_t GetTimerFrequency(void);
    void TimerProc();
    
#if _DEBUG_SENSOR
    void SendData(void);
    void ReceiveData( uint8_t Index );
#endif

};

#endif

