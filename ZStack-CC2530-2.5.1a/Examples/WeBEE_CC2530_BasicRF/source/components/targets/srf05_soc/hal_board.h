/***********************************************************************************

  Filename:     hal_board.h

  Description:  SmartRF05 board with CCxxxxEM.

***********************************************************************************/

#ifndef HAL_BOARD_H
#define HAL_BOARD_H

#if (chip==2430)
#include "ioCC2430.h"
#elif (chip==2431)
#include "ioCC2431.h"
#elif (chip==2530 || chip==2531)
#include "ioCC2530.h"
#elif (chip==2510)
#include "ioCC2510.h"
#elif (chip==2511)
#include "ioCC2511.h"
#elif (chip==1110)
#include "ioCC1110.h"
#elif (chip==1111)
#include "ioCC1111.h"
#else
#error "Chip not supported!"
#endif

#include "hal_cc8051.h"
#include "hal_defs.h"
#include "hal_mcu.h"

/***********************************************************************************
 * CONSTANTS
 */

// Board properties
#define BOARD_NAME                     "SRF05EB"
#define NUM_LEDS                        4
#define NUM_BUTTONS                     2
#define NUM_JSTKS                       1
#define NUM_POTS                        1

// Clock
#if (chip==2430) || (chip==2431) || (chip==2530) || (chip==2531)
#define BSP_CONFIG_CLOCK_MHZ                32
#elif (chip==2510) || (chip==1110)
#define BSP_CONFIG_CLOCK_MHZ                26
#endif

// SPI
#define HAL_BOARD_IO_SPI_MISO_PORT     1
#define HAL_BOARD_IO_SPI_MISO_PIN      7
#define HAL_BOARD_IO_SPI_MOSI_PORT     1
#define HAL_BOARD_IO_SPI_MOSI_PIN      6
#define HAL_BOARD_IO_SPI_CLK_PORT      1
#define HAL_BOARD_IO_SPI_CLK_PIN       5
#define HAL_BOARD_IO_EM_CS_PORT        1
#define HAL_BOARD_IO_EM_CS_PIN         4

// LCD
#define HAL_BOARD_IO_LCD_CS_PORT       1
#define HAL_BOARD_IO_LCD_CS_PIN        2
#define HAL_BOARD_IO_LCD_MODE_PORT     0
#define HAL_BOARD_IO_LCD_MODE_PIN      0

// LEDs
#define HAL_BOARD_IO_LED_1_PORT        1   // Green************LED1  by boo
#define HAL_BOARD_IO_LED_1_PIN         0
#define HAL_BOARD_IO_LED_2_PORT        1   // Red**************LED2  by boo
#define HAL_BOARD_IO_LED_2_PIN         1
#define HAL_BOARD_IO_LED_3_PORT        1   // Yellow
#define HAL_BOARD_IO_LED_3_PIN         4
#define HAL_BOARD_IO_LED_4_PORT        0   // Orange
#define HAL_BOARD_IO_LED_4_PIN         1


// Buttons
#define HAL_BOARD_IO_BTN_1_PORT        0   // Button S1
#define HAL_BOARD_IO_BTN_1_PIN         0

// Potmeter
#define HAL_POTMETER_ADC_PORT               0
#define HAL_POTMETER_ADC_CH                 7

// Joystick
#define HAL_BOARD_IO_JOYSTICK_ADC_PORT      0
#define HAL_BOARD_IO_JOYSTICK_ADC_PIN       6
#define HAL_BOARD_IO_JOY_MOVE_PORT          2
#define HAL_BOARD_IO_JOY_MOVE_PIN           0

// UART
#define HAL_BOARD_IO_UART_RTS_PORT          0
#define HAL_BOARD_IO_UART_RTS_PIN           5


// Debounce
#define HAL_DEBOUNCE(expr)    { int i; for (i=0; i<500; i++) { if (!(expr)) i = 0; } }

/***********************************************************************************
 * MACROS
 */

// LED

#ifdef SRF05EB_VERSION_1_3
// SmartRF05EB rev 1.3 has only one accessible LED
#define HAL_LED_SET_1()                 MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_SET_2()
#define HAL_LED_SET_3()
#define HAL_LED_SET_4()

#define HAL_LED_CLR_1()                 MCU_IO_SET_LOW(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_CLR_2()
#define HAL_LED_CLR_3()
#define HAL_LED_CLR_4()

#define HAL_LED_TGL_1()                 MCU_IO_TGL(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_TGL_2()
#define HAL_LED_TGL_3()
#define HAL_LED_TGL_4()

#else
// SmartRF05EB rev 1.7 and later has four LEDs available
#define HAL_LED_SET_1()                 MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_SET_2()                 MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)
#define HAL_LED_SET_3()                 MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_3_PORT, HAL_BOARD_IO_LED_3_PIN)
#define HAL_LED_SET_4()                 MCU_IO_SET_HIGH(HAL_BOARD_IO_LED_4_PORT, HAL_BOARD_IO_LED_4_PIN)

#define HAL_LED_CLR_1()                 MCU_IO_SET_LOW(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_CLR_2()                 MCU_IO_SET_LOW(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)
#define HAL_LED_CLR_3()                 MCU_IO_SET_LOW(HAL_BOARD_IO_LED_3_PORT, HAL_BOARD_IO_LED_3_PIN)
#define HAL_LED_CLR_4()                 MCU_IO_SET_LOW(HAL_BOARD_IO_LED_4_PORT, HAL_BOARD_IO_LED_4_PIN)

#define HAL_LED_TGL_1()                 MCU_IO_TGL(HAL_BOARD_IO_LED_1_PORT, HAL_BOARD_IO_LED_1_PIN)
#define HAL_LED_TGL_2()                 MCU_IO_TGL(HAL_BOARD_IO_LED_2_PORT, HAL_BOARD_IO_LED_2_PIN)
#define HAL_LED_TGL_3()                 MCU_IO_TGL(HAL_BOARD_IO_LED_3_PORT, HAL_BOARD_IO_LED_3_PIN)
#define HAL_LED_TGL_4()                 MCU_IO_TGL(HAL_BOARD_IO_LED_4_PORT, HAL_BOARD_IO_LED_4_PIN)
#endif

// Buttons
#ifdef SRF05EB_VERSION_1_3
#define HAL_BUTTON_1_PUSHED() (!MCU_IO_GET(HAL_BOARD_IO_BTN_1_PORT, \
    HAL_BOARD_IO_BTN_1_PIN))
#else
#define HAL_BUTTON_1_PUSHED() (MCU_IO_GET(HAL_BOARD_IO_BTN_1_PORT, \
    HAL_BOARD_IO_BTN_1_PIN))
#endif

// Joystick
#define HAL_JOYSTICK_LEVEL()  (MCU_IO_GET(HAL_BOARD_IO_JOYSTICK_ADC_PORT, \
    HAL_BOARD_IO_JOYSTICK_ADC_CH_PIN)

#define HAL_JOYSTICK_UP() ( halJoystickGetDir()==HAL_JOYSTICK_EVT_UP )
#define HAL_JOYSTICK_DOWN() ( halJoystickGetDir()==HAL_JOYSTICK_EVT_DOWN )
#define HAL_JOYSTICK_LEFT() ( halJoystickGetDir()==HAL_JOYSTICK_EVT_LEFT )
#define HAL_JOYSTICK_RIGHT() ( halJoystickGetDir()==HAL_JOYSTICK_EVT_RIGHT )

// UART RTS
#define HAL_RTS_SET()       MCU_IO_SET_HIGH(HAL_BOARD_IO_UART_RTS_PORT, \
    HAL_BOARD_IO_UART_RTS_PIN)
#define HAL_RTS_CLR()       MCU_IO_SET_LOW(HAL_BOARD_IO_UART_RTS_PORT, \
    HAL_BOARD_IO_UART_RTS_PIN)
#define HAL_RTS_DIR_OUT()   MCU_IO_OUTPUT(HAL_BOARD_IO_UART_RTS_PORT, \
    HAL_BOARD_IO_UART_RTS_PIN, 1)


// LCD SPI interface control
#define LCD_SPI_BEGIN()            MCU_IO_SET_LOW(HAL_BOARD_IO_LCD_CS_PORT, HAL_BOARD_IO_LCD_CS_PIN)
#define LCD_SPI_TX(x)              st( U1CSR &= ~0x02; U1DBUF = x; )
#define LCD_SPI_RX()               U1DBUF
#define LCD_SPI_WAIT_RXRDY()       st( while((U1CSR & 0x02) != 0x02); )
#define LCD_SPI_END()              st( NOP(); NOP(); NOP(); NOP(); \
                                       MCU_IO_SET_HIGH(HAL_BOARD_IO_LCD_CS_PORT, HAL_BOARD_IO_LCD_CS_PIN); )

// LCD pin control
#define LCD_DO_WRITE()             MCU_IO_SET_HIGH(HAL_BOARD_IO_LCD_MODE_PORT, HAL_BOARD_IO_LCD_MODE_PIN)
#define LCD_DO_CONTROL()           MCU_IO_SET_LOW(HAL_BOARD_IO_LCD_MODE_PORT, HAL_BOARD_IO_LCD_MODE_PIN)

// Port initialization
#define LCD_CTRL_INIT_PORTS()      st( MCU_IO_OUTPUT(HAL_BOARD_IO_LCD_CS_PORT, HAL_BOARD_IO_LCD_CS_PIN, 1); \
                                       MCU_IO_OUTPUT(HAL_BOARD_IO_LCD_MODE_PORT, HAL_BOARD_IO_LCD_MODE_PIN, 1); )

// HAL processing not required for this board
#define HAL_PROCESS()

/***********************************************************************************
 * FUNCTION PROTOTYPES
 */
void halBoardInit(void);
void halLcdSpiInit(void);
void halLcdSpiEna(void);
void halLcdSpiDis(void);

#endif
