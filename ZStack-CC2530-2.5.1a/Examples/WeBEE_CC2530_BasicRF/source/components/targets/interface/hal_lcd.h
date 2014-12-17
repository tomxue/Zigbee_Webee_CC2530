/***********************************************************************************
  Filename:     hal_lcd.h

  Description:  hal lcd library header file

***********************************************************************************/

#ifndef HAL_LCD_H
#define HAL_LCD_H

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"


/***********************************************************************************
 * CONSTANTS AND DEFINES
 */

enum {
    HAL_LCD_RADIX_DEC,
    HAL_LCD_RADIX_HEX
};

enum {
    HAL_LCD_LINE_1 = 1,
    HAL_LCD_LINE_2,
    HAL_LCD_LINE_3,
    /* 5/7/2010 ++ by outman from feibit */
    HAL_LCD_LINE_4
};

/* LCD Control lines */
#define HAL_LCD_MODE_PORT 0
#define HAL_LCD_MODE_PIN  0

#define HAL_LCD_RESET_PORT 1
#define HAL_LCD_RESET_PIN  1

#define HAL_LCD_CS_PORT 1
#define HAL_LCD_CS_PIN  2

/* LCD SPI lines */
#define HAL_LCD_CLK_PORT 1
#define HAL_LCD_CLK_PIN  5

#define HAL_LCD_MOSI_PORT 1
#define HAL_LCD_MOSI_PIN  6

/*No output data comes from LCD 30/8/2010 ++ by feibit */
//#define HAL_LCD_MISO_PORT 1
//#define HAL_LCD_MISO_PIN  7

/* LCD backlight control pin */
#define HAL_LCD_BK_PORT 0
#define HAL_LCD_BK_PIN  7
/***********************************************************************************
 * GLOBAL FUNCTIONS
 */

void halLcdInit(void);
void halLcdClear(void);
//void halLcdClearLine(uint8 line);
void halLcdSetContrast(uint8 value);
uint8 halLcdGetLineLength(void);
uint8 halLcdGetNumLines(void);

void halLcdWriteChar(uint8 line, uint8 col, char text);
void halLcdWriteLine(uint8 line, const char XDATA *text);
void halLcdWriteLines(const char XDATA *line1, const char XDATA *line2, const char XDATA *line3);

void halLcdClearAllSpecChars(void);
void halLcdCreateSpecChar(uint8 index, const char XDATA *p5x8Spec);
void halLcdWriteSpecChar(uint8 line, uint8 col, uint8 index);

#ifdef  __cplusplus
}
#endif

/**********************************************************************************/
#endif

/***********************************************************************************
  Copyright 2007-2009 Texas Instruments Incorporated. All rights reserved.

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
