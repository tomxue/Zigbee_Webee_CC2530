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
    uint32 segNumber=0;
    int16 perRssiBuf[RSSI_AVG_WINDOW_SIZE] = {0};    // Ring buffer for RSSI
    uint8 perRssiBufCounter = 0;                     // Counter to keep track of the
    // oldest newest byte in RSSI
    // ring buffer
    perRxStats_t rxStats = {0,0,0,0};
    int16 rssi;
    uint8 resetStats=FALSE;

#ifdef INCLUDE_PA
    uint8 gain;

    // Select gain (for modules with CC2590/91 only)
    gain =appSelectGain();
    halRfSetGain(gain);
#endif

    // Initialize BasicRF
    basicRfConfig.myAddr = RX_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
    basicRfReceiveOn();

    halLcdClear();
    halLcdWriteLines("PER Tester", "Receiver", "Ready");

    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());
        if(basicRfReceive((uint8*)&rxPacket, MAX_PAYLOAD_LENGTH, &rssi)>0) {
            halLedSet(3);
			
	    // Change byte order from network to host order
	    UINT32_NTOH(rxPacket.seqNumber);
            segNumber = rxPacket.seqNumber;
            
            // If statistics is reset set expected sequence number to
            // received sequence number
            if(resetStats){
              rxStats.expectedSeqNum = segNumber;
              resetStats=FALSE;
            }
        
            // Subtract old RSSI value from sum
            rxStats.rssiSum -= perRssiBuf[perRssiBufCounter];
            // Store new RSSI value in ring buffer, will add it to sum later
            perRssiBuf[perRssiBufCounter] =  rssi;

            // Add the new RSSI value to sum
            rxStats.rssiSum += perRssiBuf[perRssiBufCounter];
            if (++perRssiBufCounter == RSSI_AVG_WINDOW_SIZE) {
                perRssiBufCounter = 0;      // Wrap ring buffer counter
            }

            // Check if received packet is the expected packet
            if (rxStats.expectedSeqNum == segNumber) {
                rxStats.expectedSeqNum++;
            }
            // If there is a jump in the sequence numbering this means some packets in
            // between has been lost.
            else if (rxStats.expectedSeqNum < segNumber){
                rxStats.lostPkts += segNumber - rxStats.expectedSeqNum;
                rxStats.expectedSeqNum = segNumber + 1;
            }
            // If the sequence number is lower than the previous one, we will assume a
            // new data burst has started and we will reset our statistics variables.
            else {
                // Update our expectations assuming this is a new burst
                rxStats.expectedSeqNum = segNumber + 1;
                rxStats.rcvdPkts = 0;
                rxStats.lostPkts = 0;
            }
            rxStats.rcvdPkts++;

            // reset statistics if button 1 is pressed
            if(halButtonPushed()==HAL_BUTTON_1){
                resetStats = TRUE;
                rxStats.rcvdPkts = 1;
                rxStats.lostPkts = 0;
            }
            
            // Update LCD
            // PER in units per 1000
            utilLcdDisplayValue(HAL_LCD_LINE_1, "PER: ", (int32)((rxStats.lostPkts*1000)/(rxStats.lostPkts+rxStats.rcvdPkts)), " /1000");
            utilLcdDisplayValue(HAL_LCD_LINE_2, "RSSI: ", (int32)rxStats.rssiSum/32, "dBm");
            #ifndef SRF04EB
            utilLcdDisplayValue(HAL_LCD_LINE_3, "Recv'd: ", (int32)rxStats.rcvdPkts, NULL);
            #endif
            halLedClear(3);
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
    uint32 burstSize=0;
    uint32 pktsSent=0;
    uint8 appTxPower;
	uint8 n;

    // Initialize BasicRF
    basicRfConfig.myAddr = TX_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Set TX output power
    appTxPower = appSelectOutputPower();
    halRfSetTxPower(appTxPower);

    // Set burst size
    burstSize = appSelectBurstSize();

    // Basic RF puts on receiver before transmission of packet, and turns off
    // after packet is sent
    basicRfReceiveOff();

    // Config timer and IO
    n= appSelectRate();
    appConfigTimer(n);
    halJoystickInit();

    // Initalise packet payload
    txPacket.seqNumber = 0;
    for(n = 0; n < sizeof(txPacket.padding); n++) {
        txPacket.padding[n] = n;
    }

    halLcdClear();
    halLcdWriteLines("PER Tester", "Joystick Push", "start/stop");

    // Main loop
    while (TRUE) {

        // Wait for user to start application
        while(!halJoystickPushed() );
        appStartStop();

        halLcdClear();
        halLcdWriteLines("PER Tester", "Transmitter", NULL);

        while(appStarted) {
            if( halJoystickPushed() ) {
                appStartStop();
            }

            if (pktsSent < burstSize) {
                if( appState == TRANSMIT_PACKET ) {
                    // Make sure sequence number has network byte order
                    UINT32_HTON(txPacket.seqNumber);

                    basicRfSendPacket(RX_ADDR, (uint8*)&txPacket, PACKET_SIZE);

                    // Change byte order back to host order before increment
                    UINT32_NTOH(txPacket.seqNumber);
                    txPacket.seqNumber++;

                    pktsSent++;
                    #ifdef SRF04EB
                    utilLcdDisplayValue(HAL_LCD_LINE_2, "Sent: ", (int32)pktsSent, NULL);
                    #else
                    utilLcdDisplayValue(HAL_LCD_LINE_3, "Sent: ", (int32)pktsSent, NULL);
                    #endif
                    appState = IDLE;

                    halLedToggle(3);
                }
            }
            else
                appStarted = !appStarted;
        }

        // Reset statistics and sequence number
        pktsSent = 0;
        txPacket.seqNumber = 0;
        halLcdClear();
        halLedClear(3);
        halLcdWriteLines("PER Test", "Joystick Push", "start/stop");
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
    uint8 appMode;

    appState = IDLE;
    appStarted = FALSE;
	
    // Config basicRF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.ackRequest = FALSE;

    // Initialise board peripherals
    halBoardInit();

    // Initalise hal_rf
    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Indicate that device is powered
    halLedSet(1);

    // Print Logo and splash screen on LCD
    utilPrintLogo("PER Tester");

    // Wait for user to press S1 to enter menu
    while (halButtonPushed()!=HAL_BUTTON_1);
    halMcuWaitMs(350);
    halLcdClear();

    // Set channel
    basicRfConfig.channel = appSelectChannel();

    // Set mode
    appMode = appSelectMode();

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
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
