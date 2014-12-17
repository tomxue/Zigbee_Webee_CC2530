/***********************************************************************************
  Filename: 	per_test.c

  Description:  This application functions as a packet error rate (PER) tester.
  One node is set up as transmitter and the other as receiver. The role and
  configuration parameters for the PER test of the node is chosen on initalisation
  by navigating the joystick and confirm the choices with S1.

  The configuration parameters are channel, burst size and tx power. Push S1 to
  enter the menu. Then the configuration parameters are set by pressing
  joystick to right or left (increase/decrease value) and confirm with S1.

  After configuration of both the receiver and transmitter, the PER test is
  started by pressing joystick up on the transmitter. By pressing joystick up
  again the test is stopped.

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_int.h"
#include "hal_timer_32k.h"
#include "hal_joystick.h"
#include "hal_button.h"
#include "hal_board.h"
#include "hal_rf.h"
#include "hal_assert.h"
#include "util_lcd.h"
#include "basic_rf.h"
#include "per_test.h"
#include "string.h"

/***********************************************************************************
* CONSTANTS
*/
// Application states
#define IDLE                      0
#define TRANSMIT_PACKET           1

/***********************************************************************************
* LOCAL VARIABLES
*/
static basicRfCfg_t basicRfConfig;
static perTestPacket_t txPacket;
static perTestPacket_t rxPacket;
static volatile uint8 appState;
static volatile uint8 appStarted;

/***********************************************************************************
* LOCAL FUNCTIONS
*/
static void appTimerISR(void);
static void appStartStop(void);
static void appTransmitter();
static void appReceiver();
void uartInit(void);//**************************
void uartSend(int8 *Data,int len);//**********************
//#define MODE_SEND   1

/****************************************************************
串口初始化函数
****************************************************************/
void uartInit(void)
{ 
    PERCFG = 0x00;              //位置1 P0口
    P0SEL = 0x0c;              //P0_2,P0_3用作串口（外部设备功能）
    P2DIR &= ~0XC0;          //P0优先作为UART0

    U0CSR |= 0x80;              //设置为UART方式
    U0GCR |= 11;                       
    U0BAUD |= 216;              //波特率设为115200
    UTX0IF = 0;               //UART0 TX中断标志初始置位0
}

/****************************************************************
串口发送字符串函数            
****************************************************************/
void uartSend(int8 *Data,int len)
{
  int j;
  for(j=0;j<len;j++)
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

/***********************************************************************************
* @fn          appTimerISR
*
* @brief       32KHz timer interrupt service routine. Signals PER test transmitter
*              application to transmit a packet by setting application state.
*
* @param       none
*
* @return      none
*/
static void appTimerISR(void)
{
    appState = TRANSMIT_PACKET;
}


/***********************************************************************************
* @fn          appStartStop
*
* @brief       Joystick up interrupt service routine. Start or stop 32KHz timer,
*              and thereby start or stop PER test packet transmission.
*
* @param       none
*
* @return      none
*/
static void appStartStop(void)
{
    // toggle value
    appStarted ^= 1;

    if(appStarted) {
        halTimer32kIntEnable();
    }
    else {
        halTimer32kIntDisable();
    }
}


/***********************************************************************************
* @fn          appConfigTimer
*
* @brief       Configure timer interrupts for application. Uses 32KHz timer
*
* @param       uint16 rate - Frequency of timer interrupt. This value must be
*              between 1 and 32768 Hz
*
* @return      none
*/
static void appConfigTimer(uint16 rate)
{
    halTimer32kInit(TIMER_32K_CLK_FREQ/rate);
    halTimer32kIntConnect(&appTimerISR);
}


/***********************************************************************************
* @fn          appReceiver
*
* @brief       Application code for the receiver mode. Puts MCU in endless loop
*
* @param       basicRfConfig - file scope variable. Basic RF configuration data
*              rxPacket - file scope variable of type perTestPacket_t
*
* @return      none
*/
static void appReceiver()
{
  uint32 segNumber=0;                              // 数据包序列号 
  int16 perRssiBuf[RSSI_AVG_WINDOW_SIZE] = {0};    // 存储RSSI的环形缓冲区
  uint8 perRssiBufCounter = 0;                     // 计数器用于RSSI缓冲区统计
 
  perRxStats_t rxStats = {0,0,0,0};      
  int16 rssi;
  uint8 resetStats=FALSE;
  
  int8 Myper[5];        
  int8 Myrssi[2];
  int8 Myreceive[4];
  int32 temp_per;           //存放掉包率
  int32 temp_receive;       //存放接收的包的个数
  int32 temp_rssi;          //存放前32个rssi值的平均值
  uartInit();               // 初始化串口
  
#ifdef INCLUDE_PA
  uint8 gain;

  // Select gain (for modules with CC2590/91 only)
  gain =appSelectGain();
  halRfSetGain(gain);
#endif
    
   // Initialize BasicRF     初始化Basic RF 
  basicRfConfig.myAddr = RX_ADDR;
  if(basicRfInit(&basicRfConfig)==FAILED) 
  {
    HAL_ASSERT(FALSE);
  }
  //打开接收模块
  basicRfReceiveOn();

  /* 主循环 */
  uartSend("PER_test: ",strlen("PER_test: ")); //串口发送数据
    // Main loop
  while (TRUE) 
  {
    while(!basicRfPacketIsReady());  // 等待新的数据包
    //查看之后发行这里的rxPacket和发送里面的txPacket是同一种数据类型
    //basicRfReceive（指向数据缓冲区的指针，缓冲区最大数据长度，这个包的rssi值）
    //返回缓冲区实际数据长度
    if(basicRfReceive((uint8*)&rxPacket, MAX_PAYLOAD_LENGTH, &rssi)>0) {
         halLedSet(2);//*************P1_1 LED2点亮
            
      UINT32_NTOH(rxPacket.seqNumber);  // 改变接收序号的字节顺序
      segNumber = rxPacket.seqNumber;   //读取包的序号
            
      // If statistics is reset set expected sequence number to
      // received sequence number 
      //若统计被复位，设置期望收到的数据包序号为已经收到的数据包序号  
      //怎么样被认为统计复位？在后面~
      if(resetStats)
      {
        rxStats.expectedSeqNum = segNumber;
        
        resetStats=FALSE;
      }  
      
      //下面这几行代码是用来计算上32个包的RSSI值的
      //先预设一个32个长度的数组，用来存放RSSI值，一个指针，指示最旧的一个RSSI值
      //每次获取新的包后，把最旧的RSSI值从总和处减去，再把新的RSSI值放入，并把它的值加入总和
      // Subtract old RSSI value from sum
      rxStats.rssiSum -= perRssiBuf[perRssiBufCounter];  // 从sum中减去旧的RSSI值
      // Store new RSSI value in ring buffer, will add it to sum later
      perRssiBuf[perRssiBufCounter] =  rssi;  // 存储新的RSSI值到环形缓冲区，之后它将被加入sum
      rxStats.rssiSum += perRssiBuf[perRssiBufCounter];  // 增加新的RSSI值到sum
      //如果指针超出数组最大值，复位指针
      if(++perRssiBufCounter == RSSI_AVG_WINDOW_SIZE) {
        perRssiBufCounter = 0;      
      }

      
      //检查接收到的数据包是否是所期望收到的数据包
      // 是所期望收到的数据包
      if(rxStats.expectedSeqNum == segNumber)   
      {
        rxStats.expectedSeqNum++;  
      }
      
      // 不是所期望收到的数据包（大于期望收到的数据包的序号）
      // 认为丢包
      else if(rxStats.expectedSeqNum < segNumber)  
      {                                            
        rxStats.lostPkts += segNumber - rxStats.expectedSeqNum;
        rxStats.expectedSeqNum = segNumber + 1;
      }
      
      // (小于期望收到的数据包的序号）
      //认为是一个新的测试开始，复位统计变量
      else  
      {              
        rxStats.expectedSeqNum = segNumber + 1;
        rxStats.rcvdPkts = 0;
        rxStats.lostPkts = 0;
      }
      rxStats.rcvdPkts++;
      
      //以下代码都是用于串口输出计算值的
      temp_receive=(int32)rxStats.rcvdPkts;
       if(temp_receive>1000)
      {
       if(halButtonPushed()==HAL_BUTTON_1){
       resetStats = TRUE;
       rxStats.rcvdPkts = 1;
       rxStats.lostPkts = 0;
        }
      }

      Myreceive[0]=temp_receive/100+'0';
      Myreceive[1]=temp_receive%100/10+'0';
      Myreceive[2]=temp_receive%10+'0';
      Myreceive[3]='\0';
      uartSend("RECE:",strlen("RECE:"));
      uartSend(Myreceive,4);
      uartSend("    ",strlen("    "));   
      
      temp_per = (int32)((rxStats.lostPkts*1000)/(rxStats.lostPkts+rxStats.rcvdPkts));
      Myper[0]=temp_per/100+'0';
      Myper[1]=temp_per%100/10+'0'; 
      Myper[2]='.';
      Myper[3]=temp_per%10+'0';
      Myper[4]='%';
      uartSend("PER:",strlen("PER:"));
      uartSend(Myper,5);
      uartSend("    ",strlen("    "));
     
      temp_rssi=(0-(int32)rxStats.rssiSum/32);
      Myrssi[0]=temp_rssi/10+'0';
      Myrssi[1]=temp_rssi%10+'0';
      uartSend("RSSI:-",strlen("RSSI:-"));
      uartSend(Myrssi,2);        
      uartSend("\n",strlen("\n"));

      halLedClear(2);

      halMcuWaitMs(300);
    }                    
  }
}


/***********************************************************************************
* @fn          appTransmitter
*
* @brief       Application code for the transmitter mode. Puts MCU in endless
*              loop
*
* @param       basicRfConfig - file scope variable. Basic RF configuration data
*              txPacket - file scope variable of type perTestPacket_t
*              appState - file scope variable. Holds application state
*              appStarted - file scope variable. Controls start and stop of
*                           transmission
*
* @return      none
*/
static void appTransmitter()
{
  //声明变量
  uint32 burstSize=0;     //设定进行一次测试所发送的数据包数量
  uint32 pktsSent=0;      //指示当前已经发了多少个数据包
  uint8 n;

  //初始化Basic RF
  basicRfConfig.myAddr = TX_ADDR;
  if(basicRfInit(&basicRfConfig)==FAILED) 
  {
    HAL_ASSERT(FALSE);
  }

  //置输出功率
  halRfSetTxPower(2);

  //设置进行一次测试所发送的数据包数量 
  burstSize = 1000;

  //关闭接收模块，省电
  basicRfReceiveOff();

  //配置定时器和IO
  //暂时不知道有什么用...以后补上
  appConfigTimer(0xC8);

  //初始化数据包载荷
  //txPacket是什么？ 就是一个数据包~在per_test.h中！
  //里面有两个变量，seqNumber和padding[6]
  //就是说一个数据包里面有6个字节的内容和一个表示序号的seqNumber
  //讲一下seqNumber 就是拿来当序号用，发送时按012345这样的顺序发送，所以理应012345这样接受
  //如果这次收到3，下次收到5，那就表示丢包了
  txPacket.seqNumber = 0;
  for(n = 0; n < sizeof(txPacket.padding); n++)  //初始化下，数据包里面就是012345
  {
    txPacket.padding[n] = n;
  }

  //主循环
  while (TRUE) 
  {
    if (pktsSent < burstSize) //如果数据包还没有发送完，继续执行
    {
      // 改变发送序号的字节顺序
      //我也不知道为什么要改变顺序再改回来，可能和数据发送的一些协议有关吧，以后知道再补上
      UINT32_HTON(txPacket.seqNumber);
      
      //发送数据函数（发给谁， 发的内容， 数据长度） 重点就是这行代码！
      //注意下，发送的就是txPacket这一整个数据，包括实际内容和序号，这是一个完整的数据包
      basicRfSendPacket(RX_ADDR, (uint8*)&txPacket, PACKET_SIZE);

      //在增加序号前将字节顺序改回为主机顺序
      UINT32_NTOH(txPacket.seqNumber);
      txPacket.seqNumber++; //发的序号+1 

      pktsSent++;           //发送了一个数据包了 +1

      halLedToggle(1);   //改变LED1的亮灭状态
      halMcuWaitMs(500); //延时
    }
      //数据包清零
     pktsSent = 0;

  }
}


/***********************************************************************************
* @fn          main
*
* @brief       This is the main entry of the "PER test" application.
*
* @param       basicRfConfig - file scope variable. Basic RF configuration data
*              appState - file scope variable. Holds application state
*              appStarted - file scope variable. Used to control start and stop of
*              transmitter application.
*
* @return      none
*/
void main (void)
{
    //变量声明
    uint8 appMode;         //用来选择模式（发送或接收）
    
    appState = IDLE;
    
    //配置basic RF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.ackRequest = FALSE;

    //初始化外围设备
    halBoardInit();

    //初始化hal_rf
    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }
    
    //点亮led1（P1.0）用以表示程序开始运行
    halLedSet(1);

    //信道设置 11—25都可以
    basicRfConfig.channel = 0x0B;

    //这里就是模式选择啦，选择完进入那个函数，然后main函数就不需要啦
    //这个怎么选？？
    //看MODE_SEND，go to definition，找到定义的地方
    //把那行代码注释掉就是接收部分，不注释就是发送
    #ifdef MODE_SEND
     appMode = MODE_TX;
    #else
     appMode = MODE_RX;
    #endif  
    // Transmitter application
    if(appMode == MODE_TX) {
        // No return from here
        appTransmitter();
    }
    // Receiver application
    else if(appMode == MODE_RX) {
        // No return from here
        appReceiver();
    }
    // Role is undefined. This code should not be reached
    HAL_ASSERT(FALSE);
}


/***********************************************************************************
  Copyright 2008 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED ?AS IS? WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
***********************************************************************************/
