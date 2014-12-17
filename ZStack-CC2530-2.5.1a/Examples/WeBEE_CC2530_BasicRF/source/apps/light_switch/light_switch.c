/***********************************************************************************
网蜂ZigBee-无线点灯实验代码 详细解释请参考《ZigBee实战演练》中无线电灯相关内容

实验操作：
       第一步： 找到下面内容，把appLight();  注释掉，下载到发射模块。
          appSwitch();        //节点为按键S1       P0_4
       // appLight();        //节点为指示灯LED1    P1_0

       第二步：找到相同位置，这次把appSwitch();注释掉，下载到接收模块。 
       //appSwitch();        //节点为按键S1       P0_4
        appLight();         //节点为指示灯LED1   P1_0

完成烧写后上电，按下发射模块的S1按键，可以看到接收模块的LED1被点亮。
*********************************************************************************

  Filename: light_switch.c

  Description:  This application function either as a light or a
  switch toggling the ligh. The role of the
  application is chosen in the menu with the joystick at initialisation.

  Push S1 to enter the menu. Choose either switch or
  light and confirm choice with S1.
  Joystick Up: Sends data from switch to light

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include <hal_lcd.h>
#include <hal_led.h>
#include <hal_joystick.h>
#include <hal_assert.h>
#include <hal_board.h>
#include <hal_int.h>
#include "hal_mcu.h"
#include "hal_button.h"
#include "hal_rf.h"
#include "util_lcd.h"
#include "basic_rf.h"


/***********************************************************************************
* CONSTANTS
*/
// Application parameters
#define RF_CHANNEL                25      // 2.4 GHz RF channel

// BasicRF address definitions
#define PAN_ID                0x2007
#define SWITCH_ADDR           0x2520
#define LIGHT_ADDR            0xBEEF
#define APP_PAYLOAD_LENGTH        1
#define LIGHT_TOGGLE_CMD          0

// Application states
#define IDLE                      0
#define SEND_CMD                  1

// Application role
#define NONE                      0
#define SWITCH                    1
#define LIGHT                     2
#define APP_MODES                 2

/***********************************************************************************
* LOCAL VARIABLES
*/
static uint8 pTxData[APP_PAYLOAD_LENGTH];
static uint8 pRxData[APP_PAYLOAD_LENGTH];
static basicRfCfg_t basicRfConfig;

// Mode menu
static menuItem_t pMenuItems[] =
{
#ifdef ASSY_EXP4618_CC2420
  // Using Softbaugh 7-seg display
  " L S    ", SWITCH,
  " LIGHT  ", LIGHT
#else
  // SRF04EB and SRF05EB
  "Switch",   SWITCH,
  "Light",    LIGHT
#endif
};

static menu_t pMenu =
{
  pMenuItems,
  N_ITEMS(pMenuItems)
};


#ifdef SECURITY_CCM
// Security key
static uint8 key[]= {
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
};
#endif

/***********************************************************************************
* LOCAL FUNCTIONS
*/
static void appLight();
static void appSwitch();
static uint8 appSelectMode(void);


/***********************************************************************************
* @fn          appLight
*
* @brief       Application code for light application. Puts MCU in endless
*              loop waiting for user input from joystick.
*
* @param       basicRfConfig - file scope variable. Basic RF configuration data
*              pRxData - file scope variable. Pointer to buffer for RX data
*
* @return      none
*/
static void appLight()
{

    halLcdWriteLine(HAL_LCD_LINE_1, "    W e B e e      ");
    halLcdWriteLine(HAL_LCD_LINE_2, "  ZigBee CC2530  ");
    halLcdWriteLine(HAL_LCD_LINE_4, "     LIGHT     ");

#ifdef ASSY_EXP4618_CC2420
    halLcdClearLine(1);
    halLcdWriteSymbol(HAL_LCD_SYMBOL_RX, 1);
#endif

    // Initialize BasicRF
    basicRfConfig.myAddr = LIGHT_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
    basicRfReceiveOn();

    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());

        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0) {
            if(pRxData[0] == LIGHT_TOGGLE_CMD) {
                halLedToggle(1);
            }
        }
    }
}


/***********************************************************************************
* @fn          appSwitch
*
* @brief       Application code for switch application. Puts MCU in
*              endless loop to wait for commands from from switch
*
* @param       basicRfConfig - file scope variable. Basic RF configuration data
*              pTxData - file scope variable. Pointer to buffer for TX
*              payload
*              appState - file scope variable. Holds application state
*
* @return      none
*/
static void appSwitch()
{
    halLcdWriteLine(HAL_LCD_LINE_1, "    W e B e e      ");
    halLcdWriteLine(HAL_LCD_LINE_2, "  ZigBee CC2530  ");
    halLcdWriteLine(HAL_LCD_LINE_4, "     SWITCH    ");
    
#ifdef ASSY_EXP4618_CC2420
    halLcdClearLine(1);
    halLcdWriteSymbol(HAL_LCD_SYMBOL_TX, 1);
#endif


    // Initialize BasicRF
    basicRfConfig.myAddr = SWITCH_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
    
    pTxData[0] = LIGHT_TOGGLE_CMD;

    // Keep Receiver off when not needed to save power
    basicRfReceiveOff();

    // Main loop
    while (TRUE) {
        //if( halJoystickPushed() )**********************by boo
      if(halButtonPushed()==HAL_BUTTON_1)//**************by boo
        {

            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);

            // Put MCU to sleep. It will wake up on joystick interrupt
            halIntOff();
            halMcuSetLowPowerMode(HAL_MCU_LPM_3); // Will turn on global
            // interrupt enable
            halIntOn();

        }
    }
}


/***********************************************************************************
* @fn          main
*
* @brief       This is the main entry of the "Light Switch" application.
*              After the application modes are chosen the switch can
*              send toggle commands to a light device.
*
* @param       basicRfConfig - file scope variable. Basic RF configuration
*              data
*              appState - file scope variable. Holds application state
*
* @return      none
*/
void main(void)
{
    uint8 appMode = NONE;

    // Config basicRF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
#ifdef SECURITY_CCM
    basicRfConfig.securityKey = key;
#endif

    // Initalise board peripherals
    halBoardInit();
    halJoystickInit();

    // Initalise hal_rf
    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Indicate that device is powered
    halLedClear(2);      //关闭LED2
    halLedClear(1);      //关闭LED1


    /***********************************************
    // Print Logo and splash screen on LCD
     utilPrintLogo("Light Switch");

    // Wait for user to press S1 to enter menu
    while (halButtonPushed()!=HAL_BUTTON_1);
    halMcuWaitMs(350);
    halLcdClear();

    // Set application role
    appMode = appSelectMode();
    halLcdClear();

    // Transmitter application
    if(appMode == SWITCH) {
        // No return from here
        appSwitch();
    }
    // Receiver application
    else if(appMode == LIGHT) {
        // No return from here
        appLight();
    }
    **************************************/
    
    /************Select one and shield to another***********by boo*/
    //appSwitch();        //节点为按键S1      P0_0
    appLight();         //节点为指示灯LED1   P1_0
    
   // Role is undefined. This code should not be reached
    HAL_ASSERT(FALSE);
}


/****************************************************************************************
* @fn          appSelectMode
*
* @brief       Select application mode
*
* @param       none
*
* @return      uint8 - Application mode chosen
*/
static uint8 appSelectMode(void)
{
    halLcdWriteLine(1, "Device Mode: ");

    return utilMenuSelect(&pMenu);
}

/****************************************************************************************
  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
