/******************************************************************************
    Filename: clock.c

    This file defines clock related functions for the CC253x family
    of RF system-on-chips from Texas Instruments.

******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */

#include "clock.h"
#include "hal_defs.h"
#include "hal_mcu.h"

/*******************************************************************************
 * PUBLIC FUNCTIONS
 */


/******************************************************************************
* @fn  clockSetMainSrc
*
* @brief  Function for setting the main system clock source.
*         The function turns off the clock source that is not being used.
*         TICKSPD is set to the same frequency as the source.
*
* @param  uint8 source (one of CLOCK_SRC_HFRC or CLOCK_SRC_XOSC)
*
* @return void
*
******************************************************************************/
void clockSetMainSrc(uint8 source)
{
    register uint8 osc32k_bm = CLKCONCMD & CLKCON_OSC32K_BM;

    // Source can have the following values:
    // CLOCK_SRC_XOSC   0x00  High speed Crystal Oscillator (XOSC)
    // CLOCK_SRC_HFRC   0x01  Low power RC Oscillator (HFRC)
    
    SLEEPCMD &= ~SLEEP_OSC_PD_BM;       // power up both oscillators
    while (!CC2530_IS_HFRC_STABLE() || ((SLEEPSTA & SLEEP_OSC_PD_BM)!=0));// wait until the oscillator is stable
    NOP();

    if (source == CLOCK_SRC_HFRC){
        CLKCONCMD = (osc32k_bm | CLKCON_OSC_BM | TICKSPD_DIV_2 | CLKCON_CLKSPD_BM);
    }
    else if (source == CLOCK_SRC_XOSC){
        CLKCONCMD = (osc32k_bm | TICKSPD_DIV_1);
    }
    CC2530_WAIT_CLK_UPDATE();
    SLEEPCMD |= SLEEP_OSC_PD_BM;        // power down the unused oscillator
}

/******************************************************************************
* @fn  clockSelect32k
*
* @brief  Function for selecting source for the 32kHz oscillator
*
* @param  uint8 source (one of CLOCK_32K_XTAL or CLOCK_32K_RCOSC)
*
* @return uint8 - SUCCESS or FAILED
*
******************************************************************************/
uint8 clockSelect32k(uint8 source)
{
    // System clock source must be high frequency RC oscillator before
    // changing 32K source. 
    if( !(CLKCONSTA & CLKCON_OSC_BM) )
      return FAILED;
    
    if (source == CLOCK_32K_XTAL){
        CLKCONCMD &= ~CLKCON_OSC32K_BM;
    }
    else if (source == CLOCK_32K_RCOSC){
        CLKCONCMD |= CLKCON_OSC32K_BM;
    }
    CC2530_WAIT_CLK_UPDATE();
    
    return SUCCESS;
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
