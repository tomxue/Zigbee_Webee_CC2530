/***********************************************************************************
  Filename:     hal_timer_32k.c

  Description:  hal 32KHz timer

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_timer_32k.h"
#include "hal_board.h"
#include "hal_int.h"
#include "hal_assert.h"

/***********************************************************************************
* LOCAL VARIABLES
*/
static ISR_FUNC_PTR fptr;


/***********************************************************************************
* @fn      halTimer32kInit
*
* @brief    Set up timer 1  to generate an interrupt every "cycles" 32768 Hz
*           clock cycles.
*           Use halTimerIntConnect() to connect an ISR to the interrupt.
*
* @param   uint16 cycles - Number of cycles between interrupt
*		   (does not support larger cycles value than 0xFFFF/8)
*
* @return  none
*/
void halTimer32kInit(uint16 cycles)
{
   uint16 compareValue;
	
    //uint16 clock_divider = ID_0; // Default - don't divide input clock

   // Set prescaler divider value to 128
   T1CTL |= 0x0C;

   // Multiply cycles by 8 to get 32KHz cycles
   // 32MHz/128/8 gives approximately 32KHz
   if (cycles > 0xFFFF/8){
      compareValue =0xFFFF;
      HAL_ASSERT(FALSE);
   }
   else{
     compareValue = cycles*8;
   }

   // Set compare value
   T1CC0H = HI_UINT16(compareValue);
   T1CC0L = LO_UINT16(compareValue);
#ifndef CC2530_PG1
   // Set conmpare mode with interrupt enabled
   T1CCTL0 = 0x44;
#endif

   // clear interrupt pending flag, disable interrupt
#ifdef CC2530_PG1
   T1CTL &= ~0x10; // T1CTL.OVFIF = 0
#else
   T1STAT &= ~0x01; // T1STAT.CH1IF = 0
#endif
   IEN1  &=  ~0x02; // IEN1.T1EN = 0
}


/***********************************************************************************
* @fn      halTimer32kRestart
*
* @brief   Restart timer 1. The timer is first stopped, then restarted,
*          counting up from 0
*
* @param   none
*
* @return  none
*/
void halTimer32kRestart(void)
{
    T1CTL &=  ~0x02;    // Stop timer
    IEN1  &=  ~0x02;    // Disable interrupt

    T1CNTL =  0x00;     // Reset counter value

	T1CTL |=  0x02;     // Start timer
    IEN1  |=  0x02;     // Enable interrupt
}


/***********************************************************************************
* @fn      halTimer32kIntConnect
*
* @brief   Connect function to timer interrupt
*
* @param   ISR_FUNC_PTR isr - pointer to function
*
* @return  none
*/
void halTimer32kIntConnect(ISR_FUNC_PTR isr)
{
    istate_t key;
    HAL_INT_LOCK(key);
    fptr = isr;
    HAL_INT_UNLOCK(key);
}


/***********************************************************************************
* @fn      halTimer32kIntEnable
*
* @brief   Enable 32KHz timer interrupt
*
* @param   none
*
* @return  none
*/
void halTimer32kIntEnable(void)
{
    IEN1 |=  0x02; // IEN1.T1EN = 1

    // Start timer in modulo mode
    T1CTL |= 0x02; // T1CTL.MODE = 10

}


/***********************************************************************************
* @fn      halTimer32kIntDisable
*
* @brief   Disable 32KHz timer interrupt
*
* @param   none
*
* @return  none
*/
void halTimer32kIntDisable(void)
{
     IEN1  &=  ~0x02; // IEN1.T1EN = 0
}


/***********************************************************************************
* @fn      timer32k0_ISR
*
* @brief   ISR framework for the 32KHz timer component
*
* @param   none
*
* @return  none
*/

HAL_ISR_FUNCTION(T1_ISR,T1_VECTOR)
{
   if (fptr != NULL)
   {
        (*fptr)();
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
