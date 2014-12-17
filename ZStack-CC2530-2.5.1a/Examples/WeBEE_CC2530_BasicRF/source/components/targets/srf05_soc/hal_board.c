/***********************************************************************************
  Filename:     hal_board.c

  Description:  HAL board peripherals library for the
                SmartRF05EB + CC2430EM/CC2531EM/CC2510/CC1110 platforms

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_digio.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_board.h"
#include "hal_lcd.h"
#include "hal_joystick.h"
#include "clock.h"


// comment: should this be here?
#define SPI_CLOCK_POL_LO       0x00
#define SPI_CLOCK_POL_HI       0x80
#define SPI_CLOCK_PHA_0        0x00
#define SPI_CLOCK_PHA_1        0x40
#define SPI_TRANSFER_MSB_FIRST 0x20
#define SPI_TRANSFER_MSB_LAST  0x00

/***********************************************************************************
* LOCAL FUNCTIONS
*/

/***********************************************************************************
* GLOBAL VARIABLES
*/

/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn      halBoardInit
*
* @brief   Set up board. Initialize MCU, configure I/O pins and user interfaces
*
* @param   none
*
* @return  none
*/

void halBoardInit(void)
{
    halMcuInit();

    // LEDs
#ifdef SRF05EB_VERSION_1_3
    // SmartRF05EB rev 1.3 has only one accessible LED
    MCU_IO_DIR_OUTPUT(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN);
    HAL_LED_CLR_1();
#else
    MCU_IO_DIR_OUTPUT(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN);
    HAL_LED_CLR_1();
    MCU_IO_DIR_OUTPUT(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN);
    HAL_LED_CLR_2();
    MCU_IO_DIR_OUTPUT(HAL_BOARD_IO_LED_3_PORT, HAL_BOARD_IO_LED_3_PIN);
    HAL_LED_CLR_3();
    MCU_IO_DIR_OUTPUT(HAL_BOARD_IO_LED_4_PORT, HAL_BOARD_IO_LED_4_PIN);
    HAL_LED_CLR_4();
#endif

    // Buttons
    MCU_IO_INPUT(HAL_BOARD_IO_BTN_1_PORT, HAL_BOARD_IO_BTN_1_PIN, MCU_IO_TRISTATE);

    // Joystick push input
    MCU_IO_INPUT(HAL_BOARD_IO_JOY_MOVE_PORT, HAL_BOARD_IO_JOY_MOVE_PIN, \
    MCU_IO_TRISTATE);

    // Analog input
    MCU_IO_PERIPHERAL(HAL_BOARD_IO_JOYSTICK_ADC_PORT, HAL_BOARD_IO_JOYSTICK_ADC_PIN);


    halLcdSpiInit();
    halLcdInit();

    halIntOn();
}


/***********************************************************************************
* @fn          halLcdSpiInit
*
* @brief       Initalise LCD SPI interface
*
* @param       none
*
* @return      none
*/
void halLcdSpiInit(void)
{
   register uint8 baud_exponent;
   register uint8 baud_mantissa;


   // Set SPI on UART 1 alternative 2
   PERCFG |= 0x02;

   // Use SPI on USART 1 alternative 2
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_MISO_PORT, HAL_BOARD_IO_SPI_MISO_PIN);
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_MOSI_PORT, HAL_BOARD_IO_SPI_MOSI_PIN);
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_CLK_PORT,  HAL_BOARD_IO_SPI_CLK_PIN);

   // Set SPI speed to 1 MHz (3 MHz is max on CC2511)
# if (chip==2430 || chip==2431)
   baud_exponent = 15 + CC2430_GET_CLKSPD();
#elif (chip==2530 || chip==2531)
   baud_exponent = 15 + CC2530_GET_CLKSPD();
#elif (chip==2510 || chip==1110)
   baud_exponent = 15 + CLKSPD_GET();
#endif
   baud_mantissa = 83;

   // Configure peripheral
   U1UCR  = 0x80;      // Flush and goto IDLE state. 8-N-1.
   U1CSR  = 0x00;      // SPI mode, master.
   U1GCR  = SPI_TRANSFER_MSB_FIRST | SPI_CLOCK_PHA_0 | SPI_CLOCK_POL_LO | baud_exponent;
   U1BAUD = baud_mantissa;
}

/***********************************************************************************
* @fn          halLcdSpiEn
*
* @brief       Enable LCD SPI interface
*
* @param       none
*
* @return      none
*/
void halLcdSpiEna(void)
{
   // Use SPI on USART 1 alternative 2
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_MISO_PORT, HAL_BOARD_IO_SPI_MISO_PIN);
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_MOSI_PORT, HAL_BOARD_IO_SPI_MOSI_PIN);
   MCU_IO_PERIPHERAL(HAL_BOARD_IO_SPI_CLK_PORT,  HAL_BOARD_IO_SPI_CLK_PIN);
}

/***********************************************************************************
* @fn          halLcdSpiDis
*
* @brief       Disable LCD SPI interface (set pins to input, except CS)
*
* @param       none
*
* @return      none
*/
void halLcdSpiDis(void)
{
  MCU_IO_INPUT(HAL_BOARD_IO_SPI_MISO_PORT, HAL_BOARD_IO_SPI_MISO_PIN, MCU_IO_TRISTATE);
  MCU_IO_INPUT(HAL_BOARD_IO_SPI_MOSI_PORT, HAL_BOARD_IO_SPI_MOSI_PIN, MCU_IO_TRISTATE);
  MCU_IO_INPUT(HAL_BOARD_IO_SPI_CLK_PORT, HAL_BOARD_IO_SPI_CLK_PIN, MCU_IO_TRISTATE);
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
