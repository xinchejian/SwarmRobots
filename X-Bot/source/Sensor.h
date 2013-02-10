#ifndef Sensor_h
#define Sensor_h

/*

*/

#include <inttypes.h>

#if _DEBUG
#define _DEBUG_SENSOR 0
#endif

#if _DEBUG_SENSOR
#define _DEBUG_SENSOR_NOTIMER 0
#endif


#define IR_TXRX_REPEATTIMES 1
#define IR_RECIEVER_NUM 4

/*To define the pins of MCU connected with sensors, you shoud set them in fact*/
#define IR_E_PIN 9
#define IR_R_FL_PIN A3
#define IR_R_FR_PIN A2
#define IR_R_BL_PIN A0
#define IR_R_BR_PIN A1  //cannot use 13 pin on arduino uno



#if _DEBUG_TIMER
typedef struct
{
    uint8_t Period;  // microsecond/4
    uint8_t Cnt;
    uint8_t Who;     //0bit = receiver 0; ..  4bit = emitter
}IRTimerTest_stru;

extern IRTimerTest_stru gIRTimerTest;
#endif

typedef enum
{
    IR_POSITION_FL = 0,   //it is used as index of array, it must the first
    IR_POSITION_FR,
    IR_POSITION_BL,
    IR_POSITION_BR,       //must less than IR_RECIEVER_NUM
    IR_POSITION_CTL = IR_RECIEVER_NUM
}IRPosition_enum;


/*To define the paras about communication*/
#define COMM_FREQUENCY 1600
#define SAMPLE_TIMES (IR_RECIEVER_NUM + 1)
#define TIMER_FREQUENCY (COMM_FREQUENCY * SAMPLE_TIMES)
#define SAMPLE_OFFSET  1  // (SAMPLE_TIMES >> 1)

//to balance the cpu, exuctive different task at different timer cycle
#define IR_Phase_TX ( SAMPLE_TIMES -1 )
#define IR_Phase_RXALL 0xFF

#define IR_COMM_START_BITNUM 2  //start bis number
#define IR_COMM_BITCUR_INIT ( 8 - IR_COMM_START_BITNUM )



/*To define the data format about communication*/
typedef struct{
    uint8_t StartBits;     //start bits, send from LSB. it must be the first
    uint8_t ReceiverID;    // 0 means the buff is null;  0xFF means all of robots
    uint8_t SenderID;
    uint8_t MessageID;
    uint8_t Para;
    uint8_t Check;   // Verify byte, using XOR BCC(block check character).  it must be uint8 and the last element
}TxRxBuf_stru;

typedef TxRxBuf_stru TxBuf_stru;
typedef TxRxBuf_stru RxBuf_stru;

#define IR_BUFF_LENTH sizeof(TxRxBuf_stru)

typedef struct MsgOutput
{
    uint8_t SenderID;
    uint8_t MessageID;
    uint8_t Para;
    uint8_t Check;   //only for debuging
}IRMsgOutput_stru;

typedef enum
{
    RX_EMPTY = 0,
    RX_RECEIVING,
    RX_MSG_OK,
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
    uint8_t RunPhase;
    bool PhaseFlag;     //in order to sample at the middle of a pulse, only used by receiver
}TxRxCtrl_stru;

typedef TxRxCtrl_stru TxCtrl_stru;
typedef TxRxCtrl_stru RxCtrl_stru;

/**/
typedef enum
{
    IR_MSG_NULL = 0,
    IR_MSG_WAIT_GETSELF
}MsgState_enum;

/**/
#define IR_MSG_QUE_NUM (IR_RECIEVER_NUM + 2)  //the max numbers of receiving msg is IR_RECIEVER_NUM at the same time
#define QUE_CURSOR_INVALID 0xFF

typedef struct {
    uint8_t Head;  //piont to valid data, init value = 0xFF
    uint8_t Tail;  //piont to the unused
    uint8_t aData[IR_MSG_QUE_NUM];  //to store the index of MSG
}MsgFIFOQue;



/* Class of Sensor */
class IR_Sensor {

  private:
  
    MsgFIFOQue MsgFifo;

    /*about TX*/
    uint8_t TxPin;
    volatile uint16_t *pEmitterAddr;
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

    uint8_t IRPower;
    bool isDetectObstacle;

    inline void SendBit(bool BitVal);
    void SetSendChance();

    inline void SetRxBufState( uint8_t Index, uint8_t State );
    uint8_t CaculateCheckData( uint8_t * pData, uint8_t ByteLen );
    bool VerifyRecData( uint8_t Index );

    inline void EnableSend(void);
    inline void SetIRPower(uint8_t);

    uint8_t PutToMsgQue(uint8_t Index);
    uint8_t GetFromMsgQue(uint8_t *pIndex);

#if !_DEBUG_SENSOR
    inline void SendData(void);
    inline void ReceiveData( uint8_t Index );
#endif

  protected:
  
  public:

    IR_Sensor(const uint8_t RobotID);
    uint8_t SendMessage(uint8_t ReceiverID, uint8_t MessageID, uint8_t para, uint8_t IRPowerTenth = 10 );
    uint8_t GetMessage(IRPosition_enum *pIRLoc, IRMsgOutput_stru *pMsg );
    uint16_t GetTimerFrequency(void);
    void TimerProc();
    void InitFrequence( uint32_t f );
    
#if _DEBUG_SENSOR
    void SendData(void);
    void ReceiveData( uint8_t Index );
    void PrintMsgQue();
    void PrintCtl();
#endif

};

#endif

