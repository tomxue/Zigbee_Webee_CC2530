/******************************************************************************
    Filename: adc.c

    These functions/macros simplifies usage of the ADC.

******************************************************************************/

#include "adc.h"


/******************************************************************************
* @fn  adcSampleSingle
*
* @brief
*      This function makes the adc sample the given channel at the given
*      resolution with the given reference.
*
* @param uint8 reference
*          The reference to compare the channel to be sampled.
*        uint8 resolution
*          The resolution to use during the sample (8, 10, 12 or 14 bit)
*        uint8 input
*          The channel to be sampled.
*
* @return int16
*          The conversion result
*
******************************************************************************/
int16 adcSampleSingle(uint8 reference, uint8 resolution, uint8 channel)
{
    int16 value;

    ADC_ENABLE_CHANNEL(channel);

    ADCIF = 0;
    ADC_SINGLE_CONVERSION(reference | resolution | channel);
    while(!ADCIF);

    value  = (ADCH << 8) & 0xff00;
    value |= ADCL;

    ADC_DISABLE_CHANNEL(channel);

    //  The variable 'value' contains 16 bits where
    //     bit 15 is a sign bit
    //     bit [14 .. 0] contain the absolute sample value
    //     Only the r upper bits are significant, where r is the resolution
    //     Resolution:
    //        12   -> [14 .. 3] (bitmask 0x7FF8)
    //        10   -> [14 .. 5] (bitmask 0x7FE0)
    //         9   -> [14 .. 6] (bitmask 0x7FC0)
    //         7   -> [14 .. 8] (bitmask 0x7F00)

    return value;
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
