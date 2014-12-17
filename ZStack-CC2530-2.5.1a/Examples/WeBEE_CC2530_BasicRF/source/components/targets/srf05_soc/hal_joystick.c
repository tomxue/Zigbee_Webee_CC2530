/************************************************************************************
    Filename:     hal_joystick.c

    Description:  HAL joystick control implementation file

    Copyright 2007 Texas Instruments, Inc.
************************************************************************************/

/************************************************************************************
 * INCLUDES
 */
#include <hal_joystick.h>
#include <hal_board.h>
#include <adc.h>
#include "hal_digio.h"
#include "hal_int.h"


/***********************************************************************************
* DEFINTIONs
*/
#define xJOYSTICK_ISR

/***********************************************************************************
* CONSTANTS
*/
#define JOYSTICK_UP_BM              BM(HAL_BOARD_IO_JOY_UP_PIN)
#define JOYSTICK_DOWN_BM            BM(HAL_BOARD_IO_JOY_DN_PIN)
#define JOYSTICK_LEFT_BM            BM(HAL_BOARD_IO_JOY_LT_PIN)
#define JOYSTICK_RIGHT_BM           BM(HAL_BOARD_IO_JOY_RT_PIN)


#define HAL_BOARD_IO_JOYSTICK_ADC_CH    HAL_BOARD_IO_JOYSTICK_ADC_PIN

/***********************************************************************************
* LOCAL VARIABLES
*/
#ifdef JOYSTICK_ISR
static const digioConfig pinJoystickMove = {HAL_BOARD_IO_JOY_MOVE_PORT,
                                            HAL_BOARD_IO_JOY_MOVE_PIN,
                                            BV(HAL_BOARD_IO_JOY_MOVE_PIN),
                                            HAL_DIGIO_INPUT, 0};
static void halJoystickMoveISR(void);
static ISR_FUNC_PTR joystick_isr_tbl[HAL_JOYSTICK_EVT_MAX] = {0};
#endif

/************************************************************************************
* @fn      halJoystickInit
*
* @brief   Initializes the joystick functionality
*
* @param  void
*
* @return void
*
************************************************************************************/
void halJoystickInit(void)
{
    // Joystick push input
    MCU_IO_INPUT(HAL_BOARD_IO_JOY_MOVE_PORT, HAL_BOARD_IO_JOY_MOVE_PIN, \
        MCU_IO_TRISTATE);

    // Analog input
    MCU_IO_PERIPHERAL(HAL_BOARD_IO_JOYSTICK_ADC_PORT, HAL_BOARD_IO_JOYSTICK_ADC_PIN);

#ifdef JOYSTICK_ISR
    halDigioConfig(&pinJoystickMove);
    halDigioIntSetEdge(&pinJoystickMove, HAL_DIGIO_INT_RISING_EDGE);
    halDigioIntConnect(&pinJoystickMove, &halJoystickMoveISR);
#endif
}

#ifdef JOYSTICK_ISR
/***********************************************************************************
* @fn      halJoystickIntConnect
*
* @brief   Connect isr for joystick move interrupt. The parameter event tells for
*          which joystick direction this isr should be called.
*
* @param   event - Joystick direction
*          func - Pointer to function to connect
*
* @return  none
*/
uint8 halJoystickIntConnect(uint8 event, ISR_FUNC_PTR func)
{
    istate_t key;
    HAL_INT_LOCK(key);
    switch(event) {
    case HAL_JOYSTICK_EVT_UP:
        joystick_isr_tbl[HAL_JOYSTICK_EVT_UP] = func;
        break;
    case HAL_JOYSTICK_EVT_DOWN:
        joystick_isr_tbl[HAL_JOYSTICK_EVT_DOWN] = func;
        break;
    case HAL_JOYSTICK_EVT_LEFT:
        joystick_isr_tbl[HAL_JOYSTICK_EVT_LEFT] = func;
        break;
    case HAL_JOYSTICK_EVT_RIGHT:
        joystick_isr_tbl[HAL_JOYSTICK_EVT_RIGHT] = func;
        break;
    case HAL_JOYSTICK_EVT_PUSHED:
        joystick_isr_tbl[HAL_JOYSTICK_EVT_PUSHED] = func;
        break;
    default:
        HAL_INT_UNLOCK(key); return FAILED;
    }
    HAL_INT_UNLOCK(key);
    return SUCCESS;
}

/***********************************************************************************
* @fn      halJoystickIntEnable
*
* @brief   Enable joystick move interrupt
*
* @param   event - Joystick direction
*
* @return  none
*/
uint8 halJoystickIntEnable(uint8 event)
{
   // Ignore event parameter
    halDigioIntEnable(&pinJoystickMove);
    return SUCCESS;
}


/***********************************************************************************
* @fn      halJoystickIntDisable
*
* @brief   Disable joystick move interrupt
*
* @param   none
*
* @return  none
*/
void halJoystickIntDisable(void)
{
   halDigioIntDisable(&pinJoystickMove);
}

/***********************************************************************************
* @fn      halJoystickIntClear
*
* @brief   Clear pending joystick interrupts
*
* @param   event - Joystick direction
*
* @return  none
*/
void halJoystickIntClear(void)
{
    halDigioIntClear(&pinJoystickMove);
}
#endif


/************************************************************************************
* @fn  halJoystickPushed
*
* @brief
*      This function detects if the joystick is being pushed. The function
*      implements software debounce. Return true only if previuosly called
*      with joystick not pushed. Return true only once each time the joystick
*      is pressed.
*
* Parameters:
*
* @param  void
*
* @return uint8
*          1: Button is being pushed
*          0: Button is not being pushed
*
******************************************************************************/
uint8 halJoystickPushed(void)
{
  uint8 value, active;
  uint8 i;
  static uint8 prevValue = 0;
  uint16 adcValue;

  // Criterion for button pushed:
  // 3 times joystick active and in center position
  value = 1;
  for (i=0; i<3; i++) {
    active = MCU_IO_GET(HAL_BOARD_IO_JOY_MOVE_PORT, HAL_BOARD_IO_JOY_MOVE_PIN);
    adcValue = adcSampleSingle(ADC_REF_AVDD, ADC_9_BIT, \
        HAL_BOARD_IO_JOYSTICK_ADC_CH);
    // Only use 7 out of the 9 bits
    adcValue = (adcValue & 0x7FC0) >> 8;
    if (! active || adcValue < 0x54) {
      // Joystick not active or not in center position
      value = 0;
      break;
    }
    halMcuWaitUs(3);
  }

  if (value){
    if (!prevValue){
      value = prevValue = 1;
      halMcuWaitMs(100);

    }
    else {
      value = 0;
    }
  }
  else{
    prevValue = 0;
  }

  return value;
}

/******************************************************************************
* @fn  halJoystickGetDir
*
* @brief
*      This function utilizes the ADC to give an indication of the current
*      position of the joystick. Current support is for 90 degrees
*      positioning only.
*
*      The joystick control is encoded as an analog voltage.  Keep on reading
*      the ADC until two consecutive key decisions are the same.
*
*      Meassured values from the ADC:
*      -------------------------------------------
*      |Direction | Voltage | Value |  Range     |
*      -------------------------------------------
*      |UP        | 0.31 V  | 0x0D  | 0x00-0x27  |
*      |DOWN      | 1.16 V  | 0x31  | 0x28-0x3B  |
*      |LEFT      | 1.62 V  | 0x45  | 0x3C-0x49  |
*      |RIGHT     | 1.81 V  | 0x4D  | 0x4A-0x53  |
*      |CENTER    | 2.12 V  | 0x5A  | 0x54-      |
*      -------------------------------------------
*
* Parameters:
*
* @param  void
*
* @return uint8
*          DOWN:    Joystick direction is down    (270 degrees)
*          LEFT:    Joystick direction is left    (180 degrees)
*	       RIGHT:   Joystick direction is right   (0 degrees)
*	       UP:      Joystick direction is up      (90 degrees)
*	       CENTER:  Joystick direction is centred (passive position)
*
******************************************************************************/
uint8 halJoystickGetDir(void)
{
    uint16 adcValue;
    uint8  direction, directionOld;

    do {
        directionOld = direction;

        adcValue = adcSampleSingle(ADC_REF_AVDD, ADC_9_BIT, \
            HAL_BOARD_IO_JOYSTICK_ADC_CH);

        // Only use 7 out of the 9 bits
        adcValue = (adcValue & 0x7FC0) >> 8;

        if (adcValue < 0x28) {
            direction = HAL_JOYSTICK_EVT_UP;
        } else if (adcValue < 0x3C) {
            direction = HAL_JOYSTICK_EVT_DOWN;
        } else if (adcValue < 0x4A) {
            direction = HAL_JOYSTICK_EVT_LEFT;
        } else if (adcValue < 0x54) {
            direction = HAL_JOYSTICK_EVT_RIGHT;
        } else {
            direction = HAL_JOYSTICK_EVT_CENTER;
        }

    } while(direction != directionOld);

    return direction;
}

#ifdef JOYSTICK_ISR
/***********************************************************************************
* @fn      halJoystickMoveISR
*
* @brief   Interrupt service routine for joystick move interrupt
*
* @param   none
*
* @return  none
*/
static void halJoystickMoveISR(void)
{
    uint8 direction = halJoystickGetDir();

    switch(direction)
    {
    case HAL_JOYSTICK_EVT_DOWN:
      if (joystick_isr_tbl[HAL_JOYSTICK_EVT_DOWN]){
        (*joystick_isr_tbl[HAL_JOYSTICK_EVT_DOWN])();
      }
        break;
    case HAL_JOYSTICK_EVT_LEFT:
      if (joystick_isr_tbl[HAL_JOYSTICK_EVT_LEFT]){
        (*joystick_isr_tbl[HAL_JOYSTICK_EVT_LEFT])();
      }
        break;
    case HAL_JOYSTICK_EVT_RIGHT:
      if (joystick_isr_tbl[HAL_JOYSTICK_EVT_RIGHT]){
        (*joystick_isr_tbl[HAL_JOYSTICK_EVT_RIGHT])();
      }
        break;
    case HAL_JOYSTICK_EVT_UP:
      if (joystick_isr_tbl[HAL_JOYSTICK_EVT_UP]){
        (*joystick_isr_tbl[HAL_JOYSTICK_EVT_UP])();
      }
        break;
    case HAL_JOYSTICK_EVT_CENTER:
      // Interrupt while in center means joystick is pushed
      if (joystick_isr_tbl[HAL_JOYSTICK_EVT_PUSHED]){
        (*joystick_isr_tbl[HAL_JOYSTICK_EVT_PUSHED])();
      }
        break;
    default:
      // no action
      break;
    }
}
#endif
