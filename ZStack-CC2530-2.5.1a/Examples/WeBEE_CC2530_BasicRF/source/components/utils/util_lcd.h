/***********************************************************************************

  Filename:     util_lcd.h

  Description:  Utility library header file for LCD control

***********************************************************************************/

#ifndef UTIL_LCD_H
#define UTIL_LCD_H

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_lcd.h"
#include "util.h"



/***********************************************************************************
* CONSTANTS AND DEFINES
*/
#define utilPrintStr(s)     utilPrintText(s,sizeof(s))


/***********************************************************************************
* GLOBAL FUNCTIONS
*/
uint8   utilMenuSelect(const menu_t* pMenu);
void    utilPrintLogo(char* szAppName);
void    utilPrintText(uint8* pTxt, uint8 n);
void    utilLoadBarGraph(void);
void    utilDisplayBarGraph(uint8 line, uint8 col, uint8 min, uint8 value);
void    utilLcdDisplayValue(uint8 line, char XDATA *pLeft, int32 value, char XDATA *pRight);
void    utilLcdDisplayCounters(uint8 line, int32 lValue, char lChar, int32 rValue, char rChar);
void    utilLcdDisplayUint16(uint8 line, uint8 radix, uint16 value);



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


