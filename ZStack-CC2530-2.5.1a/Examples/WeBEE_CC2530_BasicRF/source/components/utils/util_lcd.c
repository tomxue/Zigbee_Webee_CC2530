/***********************************************************************************

  Filename:     util_lcd.c

  Description:  Utility library for LCD control

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_board.h"
#include "hal_lcd.h"
#include "hal_joystick.h"
#include "hal_button.h"
#include "hal_assert.h"

#include "util_lcd.h"
#include "hal_rf.h"
#include "string.h"


#ifndef LCD_NOT_SUPPORTED

/***********************************************************************************
* LOCAL VARIABLES
*/
// Logo
#if !defined(SRF04EB) && !defined(ASSY_EXP4618_CC2420)

// For SRF05EB only
#define SRF05EB

static const char symbol1[8] = {0x03, 0x07, 0x0E, 0x0E, 0x1C, 0x19, 0x1B, 0x1B};
static const char symbol2[8] = {0x1B, 0x1B, 0x19, 0x1C, 0x0E, 0x0E, 0x07, 0x03};
static const char symbol3[8] = {0x18, 0x1E, 0x07, 0x03, 0x18, 0x1C, 0x06, 0x00};
static const char symbol4[8] = {0x00, 0x06, 0x1C, 0x18, 0x03, 0x07, 0x1E, 0x18};

const char ppBarGraphChar[8][8] = {
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F },
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F },
    { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F },
    { 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
    { 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
    { 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
    { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },
};
#endif

static char pLcdLineBuffer[16];


/***********************************************************************************
* GLOBAL FUNCTIONS
*/

#ifdef ASSY_EXP4618_CC2420
extern void halLcdWriteLine7Seg(const char *text);
#endif


/***********************************************************************************
* @fn          utilLcdMenuSelect
*
* @brief       Show a horisontally scrolled text menu on the LCD. Text lines given in
*               an array is shown in line 2 of the LCD, these lines are browsable.
*               The menu is navigated by using the joystick left and right, and press
*               S1 button to select an option. The function then returns the element number
*               in the given menu list that was chosen by the user.
*
* @param       ppMenu - pointer to list of texts to display for menu options
*              nMenuItems - number of menu options
*
* @return      uint8 - index to which of the menu items that was chosen
*/
uint8 utilMenuSelect(const menu_t* pMenu)
{
    uint8 index;
    uint8 updateLCD;
    #ifdef SRF04EB
    char pLcdLine2[30] = "    ";
    #endif

    index= 0;
    updateLCD= TRUE;            // Force update first time

    while (halButtonPushed()!=HAL_BUTTON_1) {
        // Joystick input: Reset = UP, Decr = LEFT, Incr = RIGHT
        if (halJoystickGetDir()==HAL_JOYSTICK_EVT_RIGHT) {
            index++;
            index %= pMenu->nItems;
            updateLCD = TRUE;
        } else if (halJoystickGetDir()==HAL_JOYSTICK_EVT_LEFT) {
            if(index == 0)
               index = pMenu->nItems-1;
            else
              index--;

            updateLCD = TRUE;
        }

        if(updateLCD) {
            // Display the updated value and arrows
            #ifdef SRF04EB
             // Make space for left arrow at left end of display
            strncpy(&(pLcdLine2[1]), (char*)pMenu->pMenuItems[index].szDescr, halLcdGetLineLength()-2);
            halLcdWriteLine(HAL_LCD_LINE_2, pLcdLine2);
            halLcdWriteChar(HAL_LCD_LINE_2, 0, '<');
            halLcdWriteChar(HAL_LCD_LINE_2, halLcdGetLineLength()-1, '>');
            #elif defined(ASSY_EXP4618_CC2420)
            halLcdWriteLine7Seg((char*)pMenu->pMenuItems[index].szDescr);
            #else
            halLcdWriteLine(HAL_LCD_LINE_2, (char*)pMenu->pMenuItems[index].szDescr);
            halLcdWriteChar(HAL_LCD_LINE_3, 0, '<');
            halLcdWriteChar(HAL_LCD_LINE_3, halLcdGetLineLength()-1, '>');
            #endif

            updateLCD = FALSE;
            HAL_DEBOUNCE(halJoystickGetDir()==HAL_JOYSTICK_EVT_CENTER);
        }
    }

    return pMenu->pMenuItems[index].value;
}




/***********************************************************************************
* @fn          utilPrintLogo
*
* @brief       Prints splash screen and logo
*
* @param       szAppName - String with name of application. Length of string must be
               no longer than (LCD_LINE_LENGTH - 5)
*
* @return      none
*/
void utilPrintLogo(char* szAppName)
{
    char lcdLine1[] = "   CCxxxx ( )";
    char lcdLine2[30]="   ";                    // Support up to 30 characters LCD line length
    uint8 lcdLineLength = halLcdGetLineLength();

    strncpy(&lcdLine1[5],utilChipIdToStr(halRfGetChipId()),4);
    lcdLine1[11] = (char)halRfGetChipVer() + '0';

    if( (strlen(szAppName)+strlen(lcdLine2)) <= lcdLineLength ) {
        strcat(lcdLine2, szAppName);
    }
    #ifdef SRF04EB
    halLcdWriteLine(HAL_LCD_LINE_1, lcdLine1);
    halLcdWriteLine(HAL_LCD_LINE_2, lcdLine2);

    #elif defined(ASSY_EXP4618_CC2420)
    halLcdWriteLine7Seg(lcdLine1+3);            // Only the chip name

    #else   // SRF05EB
    halLcdCreateSpecChar(0, symbol1);
    halLcdCreateSpecChar(1, symbol2);
    halLcdCreateSpecChar(2, symbol3);
    halLcdCreateSpecChar(3, symbol4);
    halLcdWriteLine(HAL_LCD_LINE_1, lcdLine1);
    halLcdWriteLine(HAL_LCD_LINE_2, lcdLine2);
    halLcdWriteLine(HAL_LCD_LINE_3, "   TI LPW");
    halLcdWriteSpecChar(HAL_LCD_LINE_1, 0, 0);
    halLcdWriteSpecChar(HAL_LCD_LINE_2, 0, 1);
    halLcdWriteSpecChar(HAL_LCD_LINE_1, 1, 2);
    halLcdWriteSpecChar(HAL_LCD_LINE_2, 1, 3);
    #endif
}


/***********************************************************************************
* @fn          utilPrintText
*
* @brief       Prints a text string across all lines of the display. Newlines
*              cause continuation on the next line.
*
* @param       pTxt - text to display
*
* @param       n - number of characters to print
*
* @return      0
*/
void utilPrintText(uint8* pTxt, uint8 n)
{
    uint8 li[3];
    uint8  i, iLine, nChars, nLines;

    // Display properties
    nLines= halLcdGetNumLines();
    nChars= nLines*halLcdGetLineLength();

    // Split string on newlines
    i= 0;
    iLine= 0;
    li[0]= 0;
    li[1]= 0xff;
    li[2]= 0xff;

    while(i<n && i<nChars && iLine<nLines) {
        if (pTxt[i]=='\n') {
            iLine++;
            li[iLine]= i+1;
            pTxt[i]= '\0';
        }
        i++;
    }

    // Display
	for (iLine=0; iLine<nLines; iLine++) {
		if (li[iLine]!=0xFF)
			halLcdWriteLine(HAL_LCD_LINE_1+iLine, (char const*)pTxt + li[iLine] );
	}

}

#ifdef SRF05EB
/***********************************************************************************
* @fn         utilLoadBarGraph
*
* @brief      Load bar graph symbols on LCD. This function must be called before
*             utilLcdBarGraph can be used.
*
* @param      none
*
* @return     none
*/
void utilLoadBarGraph(void)
{
	uint8 n;

    // Load the bar graph characters
    for (n = 0; n < 8; n++) {
        halLcdCreateSpecChar(n, ppBarGraphChar[n]);
    }
}

/***********************************************************************************
* @fn         utilDisplayBarGraph
*
* @brief      Display bar graph on LCD
*
* @param      uint8 line - line number
*             uint8 col - column number
*             uint8 min - minimum value
*             uint8 value - value to display
*
* @return     int8 - sampled RSSI value
*/
void utilDisplayBarGraph(uint8 line, uint8 col, uint8 min, uint8 value)
{
    if (value <= min) {
        halLcdWriteChar(line, col, ' ');
    } else if (value >= min + 8) {
        halLcdWriteSpecChar(line, col, 7);
    } else {
        halLcdWriteSpecChar(line, col, value - min - 1);
    }
}
#endif


/***********************************************************************************
 * @fn          utilLcdDisplayValue
 *
 * @brief       Display value on display with optional text on right and left side
 *
 * @param       uint8 line
 *              char *pLeft
 *              int32 value
 *              char *pRight
 *
 * @return      none
 */
void utilLcdDisplayValue(uint8 line, char XDATA *pLeft, int32 value, char XDATA *pRight)
{
    char *pValue;
    uint8 n;
    uint8 nChars;

    nChars= halLcdGetLineLength();
    pValue = convInt32ToText(value);

    for (n = 0; n < nChars; n++) {
        if (pLeft && *pLeft) {
            pLcdLineBuffer[n] = *(pLeft++);
        } else if (*pValue) {
            pLcdLineBuffer[n] = *(pValue++);
        } else if (pRight && *pRight) {
            pLcdLineBuffer[n] = *(pRight++);
        } else {
            pLcdLineBuffer[n] = ' ';
        }
    }
    halLcdWriteLine(line, pLcdLineBuffer);
}



/***********************************************************************************
 * @fn          utilLcdDisplayCounters
 *
 * @brief       Display counter values with text on display
 *
 * @param       uint8 line - display line
 *              int32 lValue - left value
 *              char lChar - left text
 *              int32 rValue - right value
 *              char rChar - right text
 *
 * @return      none
 */
void utilLcdDisplayCounters(uint8 line, int32 lValue, char lChar, int32 rValue, char rChar)
{
    uint8 n;
    char *pCounter;
    uint8 nChars;

    nChars= halLcdGetLineLength();

    // Left part
    pLcdLineBuffer[0] = lChar;
    pLcdLineBuffer[1] = '=';
    pCounter = convInt32ToText(lValue);
    for (n = 2; n < 8; n++) {
        if (*pCounter) {
            pLcdLineBuffer[n] = *(pCounter++);
        } else {
            pLcdLineBuffer[n] = ' ';
        }
    }
    pLcdLineBuffer[8] = rChar;
    pLcdLineBuffer[9] = '=';
    pCounter = convInt32ToText(rValue);
    for (n = 10; n < nChars; n++) {
        if (*pCounter) {
            pLcdLineBuffer[n] = *(pCounter++);
        } else {
            pLcdLineBuffer[n] = ' ';
        }
    }
    halLcdWriteLine(line, pLcdLineBuffer);
}



static const char hex[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
/***********************************************************************************
 * @fn          utilLcdDisplayUint16
 *
 * @brief       Write a 16 bit value on the specified line on the LCD
 *
 * @param       uint8 line - line on display
 *              uint8 radix - HAL_LCD_RADIX_DEC or HAL_LCD_RADIX_HEX
 *              uint16 value - value to display
 *
 * @return      none
 */
void utilLcdDisplayUint16(uint8 line, uint8 radix, uint16 value)
{

    if (radix == HAL_LCD_RADIX_DEC)
    {
        uint8 n = 0;
        uint8 c = 5;
        char *pValue = convInt32ToText((uint32)value);
        while (pValue[n]) n++;
        while (n-- && c)
            halLcdWriteChar(line, --c, pValue[n]);
        while (c)
            halLcdWriteChar(line, --c, ' ');
    }
    else if (radix == HAL_LCD_RADIX_HEX)
    {
        halLcdWriteChar(line, 0, '0');
        halLcdWriteChar(line, 1, 'x');
        halLcdWriteChar(line, 2, hex[(value & 0xF000) >> 12]);
        halLcdWriteChar(line, 3, hex[(value & 0x0F00) >>  8]);
        halLcdWriteChar(line, 4, hex[(value & 0x00F0) >>  4]);
        halLcdWriteChar(line, 5, hex[(value & 0x000F)]);
    }
}

#else

void utilPrintText(uint8* pTxt, uint8 n) { }

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

