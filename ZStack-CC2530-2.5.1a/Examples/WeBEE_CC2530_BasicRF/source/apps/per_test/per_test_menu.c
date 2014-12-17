/***********************************************************************************
    Filename:     per_test_menu.c

    Description:  PER test menu library

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/

#include "per_test_menu.h"
#include "hal_lcd.h"
#include "hal_rf.h"
#include "hal_assert.h"
#include "util_lcd.h"

/***********************************************************************************
* LOCAL CONSTSANTS and MACROS
*/


/***********************************************************************************
* LOCAL VARIABLES
*/
// Channel menu
static const menuItem_t pChannels[] =
{
  "11 - 2405 MHz", 11,
  "12 - 2410 MHz", 12,
  "13 - 2415 MHz", 13,
  "14 - 2420 MHz", 14,
  "15 - 2425 MHz", 15,
  "16 - 2430 MHz", 16,
  "17 - 2435 MHz", 17,
  "18 - 2440 MHz", 18,
  "19 - 2445 MHz", 19,
  "20 - 2450 MHz", 20,
  "21 - 2455 MHz", 21,
  "22 - 2460 MHz", 22,
  "23 - 2465 MHz", 23,
  "24 - 2470 MHz", 24,
  "25 - 2475 MHz", 25,
  "26 - 2480 MHz", 26,
};

static const menu_t channelMenu =
{
  pChannels,
  N_ITEMS(pChannels)
};

// Mode menu
static const menuItem_t pModes[] =
{
  "Transmitter", MODE_TX,
  "Receiver", MODE_RX,
};

static const menu_t modeMenu =
{
  pModes,
  N_ITEMS(pModes)
};

// Burst size menu
static const menuItem_t pBurstSizes[] =
{
  "1000 Pkts", 0,
  "10000 Pkts", 1,
  "100000 Pkts", 2,
  "1000000 Pkts", 3
};

uint32 burstSizes[] =
{
  BURST_SIZE_1,
  BURST_SIZE_2,
  BURST_SIZE_3,
  BURST_SIZE_4
};

static const menu_t burstSizeMenu =
{
  pBurstSizes,
  N_ITEMS(pBurstSizes)
};

// Packet rate menu
#define RATE_ITEM(n)    { #n"/second", n }
static const menuItem_t pRate[] =
{
  RATE_ITEM(100),
  RATE_ITEM(50),
  RATE_ITEM(20),
  RATE_ITEM(10),
};

static const menu_t rateMenu =
{
  pRate,
  N_ITEMS(pRate)
};

// TX output power menu
extern const menu_t powerMenu;

#ifdef INCLUDE_PA
// Gain menu
static const menuItem_t pGainSettings[] =
{
  "high gain", HAL_RF_GAIN_HIGH,
  "low gain", HAL_RF_GAIN_LOW
};

static const menu_t gainMenu =
{
  pGainSettings,
  N_ITEMS(pGainSettings)
};

#endif

/***********************************************************************************
* GLOBAL FUNCTIONS
*/



/***********************************************************************************
* @fn          appSelectChannel
*
* @brief       Select channel
*
* @param       none
*
* @return      uint8 - Channel chosen
*/
uint8 appSelectChannel(void)
{
    halLcdWriteLine(HAL_LCD_LINE_1, "Channel: ");

    return utilMenuSelect(&channelMenu);
}

/***********************************************************************************
* @fn          appSelectMode
*
* @brief       Select application mode
*
* @param       none
*
* @return      uint8 - Application mode chosen
*/
uint8 appSelectMode(void)
{
    halLcdWriteLine(HAL_LCD_LINE_1, "Operating Mode: ");

    return utilMenuSelect(&modeMenu);
}


/***********************************************************************************
* @fn          appSelectRate
*
* @brief       Select transmit rate
*
* @param       none
*
* @return      uint8 - Rat chosen
*/
uint8 appSelectRate(void)
{
    halLcdWriteLine(HAL_LCD_LINE_1, "Packet rate: ");

    return utilMenuSelect(&rateMenu);
}


/***********************************************************************************
* @fn          appSelectBurstSize
*
* @brief       Select burst size (Number of packets to be transmitted)
*
* @param       none
*
* @return      uint32 - burst size
*/
uint32 appSelectBurstSize(void)
{
  uint8 index=0;
  halLcdWriteLine(HAL_LCD_LINE_1, "Burst Size: ");

  index = utilMenuSelect(&burstSizeMenu);
  return burstSizes[index];
}

/***********************************************************************************
* @fn          appSelectOutputPower
*
* @brief       Select output power
*
* @param       none
*
* @return      uint8 - Output power alternative
*/
uint8 appSelectOutputPower(void)
{
    halLcdWriteLine(HAL_LCD_LINE_1, "TX Output Power: ");

    return utilMenuSelect(&powerMenu);
}


#ifdef INCLUDE_PA
/***********************************************************************************
* @fn          appSelectGain
*
* @brief       Select gain for CC2590/91
*
* @param       none
*
* @return      uint8 - selected gain
*/
uint8 appSelectGain(void)
{
    halLcdWriteLine(HAL_LCD_LINE_1, "Gain: ");

    return utilMenuSelect(&gainMenu);
}
#endif


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
