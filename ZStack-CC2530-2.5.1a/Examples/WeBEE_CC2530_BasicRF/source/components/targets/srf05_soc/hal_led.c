
/***********************************************************************************
  Filename:     hal_led.c

  Description:  hal led library

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_led.h"
#include "hal_board.h"

/***********************************************************************************
* GLOBAL VARIABLES
*/
// Stores state of led 4
volatile uint8 led4State=0;


/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn          halLedSet
*
* @brief       Turn LED on.
*
* @param       uint8 id - led to set
*
* @return      none
*/
void halLedSet(uint8 id)
{
    switch (id)
    {
    case 1: HAL_LED_SET_1(); break;
    case 2: HAL_LED_SET_2(); break;
    case 3: HAL_LED_SET_3(); break;
    case 4: HAL_LED_SET_4(); led4State=1; break;

    default: break;
    }
}


/***********************************************************************************
* @fn          halLedClear
*
* @brief       Turn LED off.
*
* @param       uint8 id - led to clear
*
* @return      none
*/
void halLedClear(uint8 id)
{
    switch (id)
    {
    case 1: HAL_LED_CLR_1(); break;
    case 2: HAL_LED_CLR_2(); break;
    case 3: HAL_LED_CLR_3(); break;
    case 4: HAL_LED_CLR_4(); led4State=0; break;
    default: break;
    }
}


/***********************************************************************************
* @fn          halLedToggle
*
* @brief       Change state of LED. If on, turn it off. Else turn on.
*
* @param       uint8 id - led to toggle
*
* @return      none
*/
void halLedToggle(uint8 id)
{
    switch (id)
    {
    case 1: HAL_LED_TGL_1(); break;
    case 2: HAL_LED_TGL_2(); break;
    case 3: HAL_LED_TGL_3(); break;
    case 4: HAL_LED_TGL_4(); led4State ^= 1; break;
    default: break;
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
