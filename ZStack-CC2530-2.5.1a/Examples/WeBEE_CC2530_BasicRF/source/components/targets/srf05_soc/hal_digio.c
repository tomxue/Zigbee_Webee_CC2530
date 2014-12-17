
/***********************************************************************************
  Filename:     hal_digio.c

  Description:  HAL digital IO functionality

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_board.h"
#include "hal_int.h"
#include "hal_digio.h"


/***********************************************************************************
* LOCAL VARIABLES
*/
static ISR_FUNC_PTR port0_isr_tbl[8] = {0};
static ISR_FUNC_PTR port1_isr_tbl[8] = {0};
static ISR_FUNC_PTR port2_isr_tbl[5] = {0};


/* Find a better place for this! */
#if chip==2430 || chip==2431
#define PICTL_PADSC_BM 0x40
#define PICTL_P2IEN_BM 0x20
#define PICTL_P0IENH_BM 0x10
#define PICTL_P0IENL_BM 0x08
#define PICTL_P2ICON_BM 0x04
#define PICTL_P1ICON_BM 0x02
#define PICTL_P0ICON_BM 0x01
#elif chip==2530 || chip==2531
#define PICTL_PADSC_BM 0x80
#define PICTL_P2IEN_BM 0x40
#define PICTL_P0IENH_BM 0x20
#define PICTL_P0IENL_BM 0x10
#define PICTL_P2ICON_BM 0x08
#define PICTL_P1ICONH_BM 0x04
#define PICTL_P1ICONL_BM 0x02
#define PICTL_P1ICON_BM 0x06 // Combined - for compatibility
#define PICTL_P0ICON_BM 0x01
#endif

/***********************************************************************************
* GLOBAL FUNCTIONS
*/

/***********************************************************************************
* @fn      halDigioConfig
*
* @brief   Configure the pin as specified by p.
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/

//comment: find better way of doing this. SFR registers can not be accessed by pointers, therefore code is different from MSP430 code
// comment: currently support only P0 and P1
uint8 halDigioConfig(const digioConfig* p)
{
  //  register volatile uint8* dir;
    register const uint8 bitmask = p->pin_bm;

    // Sanity check
    if ((bitmask == 0) || (bitmask != (uint8)BV(p->pin)))
    {
        return(HAL_DIGIO_ERROR);
    }

    switch(p->port)
    {
    case 0: P0SEL &= ~bitmask; 
            if (p->dir == HAL_DIGIO_OUTPUT)
            {
                if (p->initval == 1)
                {
                  P0 |= bitmask;  
                }
                else
                {
                  P0 &= ~bitmask;
                }
                P0DIR |= bitmask;
             }
             else // input
             {
                P0DIR &= ~bitmask;
             }
            break;
     case 1: P1SEL &= ~bitmask; 
            if (p->dir == HAL_DIGIO_OUTPUT)
            {
                if (p->initval == 1)
                {
                  P1 |= bitmask;  
                }
                else
                {
                  P1 &= ~bitmask;
                }
                P1DIR |= bitmask;
             }
             else // input
             {
                P1DIR &= ~bitmask;
             }
            break;
     case 2: P2SEL &= ~bitmask; 
            if (p->dir == HAL_DIGIO_OUTPUT)
            {
                if (p->initval == 1)
                {
                  P2 |= bitmask;  
                }
                else
                {
                  P2 &= ~bitmask;
                }
                P2DIR |= bitmask;
             }
             else // input
             {
                P2DIR &= ~bitmask;
             }
            break;
    //case 1: P1SEL &= ~bitmask; out = &P1OUT; dir = &P1DIR; break;
    //case 2: P2SEL &= ~bitmask; out = &P2OUT; dir = &P2DIR; break;
    default: return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      halDigioSet
*
* @brief   Set output pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioSet(const digioConfig* p)
{
   if (p->dir == HAL_DIGIO_OUTPUT)
    {
        switch (p->port)
        {
        case 0: P0 |= p->pin_bm; break;
        case 1: P1 |= p->pin_bm; break;
        case 2: P2 |= p->pin_bm; break;
        default: return(HAL_DIGIO_ERROR);
        }
        return(HAL_DIGIO_OK);
    }
    return(HAL_DIGIO_ERROR);
}


/***********************************************************************************
* @fn      halDigioClear
*
* @brief   Clear output pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioClear(const digioConfig* p)
{
   if (p->dir == HAL_DIGIO_OUTPUT)
    {
        switch (p->port)
        {
        case 0: P0 &= ~p->pin_bm; break;
        case 1: P1 &= ~p->pin_bm; break;
        case 2: P2 &= ~p->pin_bm; break;
        default: return(HAL_DIGIO_ERROR);
        }
        return(HAL_DIGIO_OK);
    }
    return(HAL_DIGIO_ERROR);
}


/***********************************************************************************
* @fn      halDigioToggle
*
* @brief   Toggle output pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioToggle(const digioConfig* p)
{
  if (p->dir == HAL_DIGIO_OUTPUT)
    {
        switch (p->port)
        {
        case 0: P0 ^= p->pin_bm; break;
        case 1: P1 ^= p->pin_bm; break;
        case 2: P2 ^= p->pin_bm; break;
        default: return(HAL_DIGIO_ERROR);
        }
        return(HAL_DIGIO_OK);
    }
    return(HAL_DIGIO_ERROR);
}


/***********************************************************************************
* @fn      halDigioGet
*
* @brief   Get value on input pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioGet(const digioConfig* p)
{
    if (p->dir == HAL_DIGIO_INPUT)
    {
        switch (p->port)
        {
        case 0: return (P0 & p->pin_bm ? 1 : 0);
        case 1: return (P1 & p->pin_bm ? 1 : 0);
        case 2: return (P2 & p->pin_bm ? 1 : 0);
        default: break;
        }
    }
    return(HAL_DIGIO_ERROR);
}


/***********************************************************************************
* @fn      halDigioIntConnect
*
* @brief   Connect function to IO interrupt
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*          ISR_FUNC_PTR func - pointer to function
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioIntConnect(const digioConfig *p, ISR_FUNC_PTR func)
{
  istate_t key;
    HAL_INT_LOCK(key);
    switch (p->port)
    {
    case 0: port0_isr_tbl[p->pin] = func; break;
    case 1: port1_isr_tbl[p->pin] = func; break;
    case 2: port2_isr_tbl[p->pin] = func; break;
    default: HAL_INT_UNLOCK(key); return(HAL_DIGIO_ERROR);
    }
    halDigioIntClear(p);
    HAL_INT_UNLOCK(key);
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      halDigioIntEnable
*
* @brief   Enable interrupt on IO pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioIntEnable(const digioConfig *p)
{
     switch (p->port)
    {
#if chip==2430 || chip==2431      
    case 0: 
      P0IE = 1;    // set P0IE bit
      if (p->pin < 4) {
        PICTL |= PICTL_P0IENL_BM; // set P0IENL
      }
      else {
        PICTL |= PICTL_P0IENH_BM; // set P0IENH
      }
            break;
    case 1:
      IEN2 |= 0x10;    // set P1IE bit
      P1IEN |= p->pin_bm;
      break;
    case 2:
      IEN2 |= 0x02;     
      PICTL |= PICTL_P2IEN_BM; // Set P2IEN
      break;
    default: 
      return(HAL_DIGIO_ERROR);
    }
#elif chip==2530 || chip==2531
    case 0: 
      P0IE = 1;    // set P0IE bit
      P0IEN |= p->pin_bm;
      break;
    case 1:
      IEN2 |= 0x10;    // set P1IE bit
      P1IEN |= p->pin_bm;
      break;
    case 2:
      IEN2 |= 0x02;     // set P2IE bit
      P2IEN |= p->pin_bm; 
      break;
    default: 
      return(HAL_DIGIO_ERROR);
    }
#endif
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      halDigioIntDisable
*
* @brief   Disable interrupt on IO pin
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioIntDisable(const digioConfig *p)
{
   switch (p->port)
    {
#if chip==2430 || chip==2431  
    case 0: 
      if (p->pin  < 4) {
        PICTL &= ~PICTL_P0IENL_BM; // clear P0IENL
      }
      else {
        PICTL &= ~PICTL_P0IENH_BM;    // clear P0IENH
      }
            break;
    case 1:
      P1IEN &= ~p->pin_bm;
      break;
    case 2:
      PICTL &= ~PICTL_P2IEN_BM; // Clear P2IEN
      break;
    default: 
      return(HAL_DIGIO_ERROR);
    }
#elif chip==2530 || chip==2531
    case 0: 
      P0IEN &= ~p->pin_bm;
      break;
    case 1:
      P1IEN &= ~p->pin_bm;
      break;
    case 2:
      P2IEN &= ~p->pin_bm;
      break;
    default: 
      return(HAL_DIGIO_ERROR);
    }
#endif
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      halDigioIntClear
*
* @brief   Clear interrupt flag
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/
uint8 halDigioIntClear(const digioConfig *p)
{
    switch (p->port)
    {
    case 0: P0IFG &= ~p->pin_bm; break;
    case 1: P1IFG &= ~p->pin_bm; break;
    case 2: P2IFG &= ~p->pin_bm; break;
    default: return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      halDigioIntSetEdge
*
* @brief   Set edge for IO interrupt
*
* @param   digioConfig* p - pointer to configuration structure for IO pin
*          edge - HAL_DIGIO_INT_FALLING_EDGE or HAL_DIGIO_INT_RISING_EDGE
*
* @return  uint8 - HAL_DIGIO_ERROR or HAL_DIGIO_OK
*/

// Comment: all pins on port are configured at same time
uint8 halDigioIntSetEdge(const digioConfig *p, uint8 edge)
{
  switch(edge)
    {
    case HAL_DIGIO_INT_FALLING_EDGE:
        switch(p->port)
        {
        case 0: PICTL |= PICTL_P0ICON_BM; // set P0ICON high
                break;
        case 1: PICTL |= PICTL_P1ICON_BM; // set P1ICON high
                break;
        case 2: PICTL |= PICTL_P2ICON_BM; // set P2ICON high
                break;
        default: return(HAL_DIGIO_ERROR);
        }
        break;

    case HAL_DIGIO_INT_RISING_EDGE:
        switch(p->port)
        {
        case 0: PICTL &= ~PICTL_P0ICON_BM; // set P0ICON low
                break;
        case 1: PICTL &= ~PICTL_P1ICON_BM; // set P0ICON low
                break;
        case 2: PICTL &= ~PICTL_P2ICON_BM; // set P0ICON low
                break;
        default: return(HAL_DIGIO_ERROR);
        }
        break;

    default:
        return(HAL_DIGIO_ERROR);
    }
    return(HAL_DIGIO_OK);
}


/***********************************************************************************
* @fn      port0_ISR
*
* @brief   ISR framework for P0 digio interrupt
*
* @param   none
*
* @return  none
*/
HAL_ISR_FUNCTION(port0_ISR,P0INT_VECTOR)
{
    register uint8 i;
    P0IF = 0;
    if (P0IFG)
    {
        for (i = 0; i < 8; i++)
        {
            register const uint8 pinmask = 1 << i;
            if (P0IFG & pinmask) {
                if (port0_isr_tbl[i] != 0) {
                (*port0_isr_tbl[i])();
                }
                P0IFG &= ~pinmask;
            }
        }
        //__low_power_mode_off_on_exit();
    }
}

/***********************************************************************************
* @fn      port1_ISR
*
* @brief   ISR framework for P0 digio interrupt
*
* @param   none
*
* @return  none
*/
HAL_ISR_FUNCTION(port1_ISR,P1INT_VECTOR)
{
    register uint8 i;
    P1IF = 0;
    if (P1IFG)
    {
        for (i = 0; i < 8; i++)
        {
            register const uint8 pinmask = 1 << i;
            if (P1IFG & pinmask) {
                if (port1_isr_tbl[i] != 0) {
                    (*port1_isr_tbl[i])();
                }
                P1IFG &= ~pinmask;
            }
        }
        //__low_power_mode_off_on_exit();
    }
}

/***********************************************************************************
* @fn      port2_ISR
*
* @brief   ISR framework for P2 digio interrupt
*
* @param   none
*
* @return  none
*/
HAL_ISR_FUNCTION(port2_ISR,P2INT_VECTOR)
{
    register uint8 i;
    P2IF = 0;
    if (P2IFG)
    {
        for (i = 0; i < 5; i++)
        {
            register const uint8 pinmask = 1 << i;
            if (P2IFG & pinmask) {
                if (port2_isr_tbl[i] != 0) {
                    (*port2_isr_tbl[i])();
                }
                P2IFG &= ~pinmask;
            }
        }
        //__low_power_mode_off_on_exit();
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
