/***********************************************************************************

	Filename:		spectrum_analyzer.c

	Description:	Samples energy on each channel (11-26).

                    Mode 1: display values as bar graphs only
                    Mode 2: display values as bar graphs AND RSSI value for the
                              selected channel. Select channel by moving the joystick left/right.

                    - Toggle between the display modes by moving the joystick up.
                    - The program starts in mode 1.

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/

#include <hal_lcd.h>
#include <hal_led.h>
#include <hal_joystick.h>
#include <hal_board.h>
#include <hal_int.h>
#include <hal_assert.h>
#include "hal_rf.h"
#include "util.h"
#include "hal_rf_util.h"
#include "util_lcd.h"

/***********************************************************************************
* CONSTANTS
*/
#define SAMPLE_COUNT       8
#define CHANNELS           16
#define CHANNEL_11         11
#define SAMPLE_TIME        2000 // us

// Display range; minimum and maximum RSSI-values in dBm
#define MIN_RSSI_DBM       -120
#define MAX_RSSI_DBM       -10

/***********************************************************************************
* LOCAL VARIABLES
*/
static volatile uint8 appShowText;
static volatile uint8 txtChannel;
static int8 ppRssi[CHANNELS][SAMPLE_COUNT];

static const char *channelText[CHANNELS] =
{
    "CH11 ", "CH12 ", "CH13 ", "CH14 ", "CH15 ", "CH16 ", "CH17 ", "CH18 ",
    "CH19 ", "CH20 ", "CH21 ", "CH22 ", "CH23 ", "CH24 ", "CH25 ", "CH26 "
};

/***********************************************************************************
* LOCAL FUNCTIONS
*/


/***********************************************************************************
* @fn          appShowText
*
* @brief
*
* @param       none
*
* @return      none
*/
static void appSetShowText(void)
{
    // toggle value
    appShowText ^= 1;
}


/***********************************************************************************
* @fn          appIncChannel
*
* @brief
*
* @param       none
*
* @return      none
*/
static void appIncChannel(void)
{
    if (appShowText) {
        txtChannel++;
        if(txtChannel>=CHANNELS)
            txtChannel=0;
    }
}


/***********************************************************************************
* @fn          appDecChannel
*
* @brief
*
* @param       none
*
* @return      none
*/
static void appDecChannel(void)
{
    if (appShowText) {
        if(txtChannel==0)
            txtChannel=CHANNELS-1;
        else
            txtChannel--;
    }
}


/***********************************************************************************
* @fn          appConfigIO
*
* @brief       Configure IO interrupts for application
*
* @param       none
*
* @return      none
*/
static void appConfigIO(void)
{
    halJoystickInit();

    halJoystickIntConnect(HAL_JOYSTICK_EVT_UP,&appSetShowText);
    halJoystickIntEnable(HAL_JOYSTICK_EVT_UP);

    halJoystickIntConnect(HAL_JOYSTICK_EVT_RIGHT,&appIncChannel);
    halJoystickIntEnable(HAL_JOYSTICK_EVT_RIGHT);

    halJoystickIntConnect(HAL_JOYSTICK_EVT_LEFT,&appDecChannel);
    halJoystickIntEnable(HAL_JOYSTICK_EVT_LEFT);
}


/***********************************************************************************
* @fn          main
*
* @brief
*
* @param
*
*
* @return      none
*/
void main (void)
{
    int8 minRssi, maxRssi, rssiOffset;
    int16 barValue;
    int16 txtValue;
    uint8 barHeight, n;

    appShowText=FALSE;
    barHeight=  3;
    txtChannel= 0;

    // Initalise board peripherals
    halBoardInit();

    // Initalise hal_rf
    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }
    // Indicate that device is powered
    halLedSet(1);

    // Print Logo and splash screen on LCD
    utilPrintLogo("Spectrum Anl");
    halMcuWaitMs(3000);

    // Calculate RSSI offset and range (must be done after setting gain mode)
    halRfSetGain(HAL_RF_GAIN_HIGH);

    rssiOffset= halRfGetRssiOffset();
    minRssi=    MIN_RSSI_DBM + rssiOffset;
    maxRssi=    MAX_RSSI_DBM + rssiOffset;

    // Config IO interrupt
    appConfigIO();

    // Set chip in RX scan mode
    halSetRxScanMode();

    // Load bar graph symbols to LCD
    utilLoadBarGraph();

    while(1) {
    	uint8 sample;
    	
        // For each RSSI sample record
        for (sample = 0; sample < SAMPLE_COUNT; sample++) {
        	uint8 channel;
            // Sample channel 11-26
            for(channel = 0; channel < CHANNELS; channel++ ) {
                ppRssi[channel][sample] = halSampleED(channel+CHANNEL_11, SAMPLE_TIME);
            }

            // Update the display with the latest graph values
            for(channel = 0; channel < CHANNELS; channel++ ) {
			
                barValue = -128;
                for (n = 0; n < SAMPLE_COUNT; n++) {
                    barValue = MAX((int8) barValue, ppRssi[channel][n]);
                }
                barValue -= minRssi;

                // Saturate
                if (barValue < 0)
                    barValue = 0;
                if (barValue > ((int16) maxRssi - (int16) minRssi))
                    barValue = (int16) maxRssi - (int16) minRssi;
                // Scale
                barValue *= (barHeight == 2) ? (8 + 1 + 8) : (8 + 1 + 8 + 1 + 8);
                barValue /= maxRssi - minRssi;

                // Display the bar
                for (n = 0; n < barHeight; n++) {
                    utilDisplayBarGraph(n + 1, channel, (barHeight - 1 - n) * 9, barValue);
                }
            }
        }

        // Show RSSI in text form on display
        if(appShowText) {
            txtValue = -128;
            barHeight=2;
            // find peak value
            for (n = 0; n < SAMPLE_COUNT; n++) {
                txtValue = MAX((int8) txtValue, ppRssi[txtChannel][n]);
            }
            txtValue -= rssiOffset;
            utilLcdDisplayValue(3, (char*)channelText[txtChannel], txtValue, " dBm");
        }
        else
            barHeight=3;
    }
}


/***********************************************************************************
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
PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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



