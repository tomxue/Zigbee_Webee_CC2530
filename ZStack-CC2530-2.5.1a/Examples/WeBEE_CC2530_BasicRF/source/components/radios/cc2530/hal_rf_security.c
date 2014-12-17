/***********************************************************************************

  Filename:       hal_rf_security.c

  Description:    CC2430/CC2530 CCM security

***********************************************************************************/

/***********************************************************************************
* INCLUDES
*/
#include "hal_board.h"

#include "hal_mcu.h"
#include "hal_rf.h"
#include "hal_rf_security.h"
#include "util.h"
#include "string.h"


/***********************************************************************************
* CONSTANTS AND DEFINES
*/


// Staring AES operation
#define AES_START()             st (ENCCS |= 0x01;)

// Setting the mode of the AES operation
#define AES_SET_MODE(mode)      st( ENCCS &= ~0x70; ENCCS |= mode; )

// Starting or stopping encryption or decryption
#define AES_SET_OPERATION(op)   st( ENCCS = (ENCCS & ~0x07) | op; )

// Poll the AES ready bit
#define AES_RDY()              ( ENCCS & 8)

// Encryption mode
#define AES_MODE_CBC            0x00
#define AES_MODE_CFB            0x10
#define AES_MODE_OFB            0x20
#define AES_MODE_CTR            0x30
#define AES_MODE_ECB            0x40
#define AES_MODE_CBCMAC         0x50

// Operations
#define AES_ENCRYPT             0x00
#define AES_DECRYPT             0x02
#define AES_LOAD_KEY            0x04
#define AES_LOAD_IV             0x06



// Convert MIC to MIC length
#define MIC_2_MICLEN(m)         ( BV((m&3)+1) & ~3 )


/***********************************************************************************
* LOCAL DATA
*/
static  uint8 nonceTx[16];
static  uint8 nonceRx[16];
static  uint8 IV[16];
static  uint8 cipherText[128];
static  uint8 buf[128];


/***********************************************************************************
* LOCAL FUNCTIONS
*/
static void halAesEncrypt(uint8 mode, uint8 *pDataIn, uint16 length, uint8 *pDataOut,
                          uint8 *pInitVector);
static void halAesDecrypt(uint8 mode, uint8 *pDataIn, uint16 length, uint8 *pDataOut,
                          uint8 *pInitVector);
static uint8 generateAuthData(uint8 c, uint8 *pIv, uint8 *pData, uint8 f, uint8 lm);




/***********************************************************************************
* GLOBAL FUNCTIONS
*/
extern void halRfAppendTxBuf(uint8* pData, uint8 length);


/***********************************************************************************
* @fn      halRfSecurityInit
*
* @brief   Security init. Write nonces and key to chip.
*
* @param   none
*
* @return  none
*/
void halRfSecurityInit(uint8* pKey, uint8* pNonceRx, uint8* pNonceTx)
{
    uint8 i;

    AES_SET_OPERATION(AES_LOAD_KEY);

    // Starting loading of key or vector.
    AES_START();

    // Store key and nonces
    for (i=0; i<NONCE_LENGTH; i++) {
        nonceRx[i]= *pNonceRx;
        nonceTx[i]= *pNonceTx;
        pNonceTx++;
        pNonceRx++;
        ENCDI = *pKey++;
    }
}



/***********************************************************************************
* @fn      halRfWriteTxBufSecure
*
* @brief   Encrypt and authenticate plaintext then fill TX buffer
*
* @param   uint8* data - data buffer. This must be allocated by caller.
*          uint8 length - number of bytes
*          uint8 c - number of bytes to encrypt
*          uint8 f - number of bytes to authenticate
*          uint8 m - integrity code (m=1,2,3 gives lenght of integrity
*                   field l(m)= 4,8,16)
*
* @return  none
*/
void halRfWriteTxBufSecure(uint8* pPkt, uint8 length, uint8 c, uint8 f, uint8 m)
{
    uint8 i,lm;

    // Generate B(i) vector
    memset(buf,0,sizeof(buf));
    lm= MIC_2_MICLEN(m);
    i= generateAuthData(c,nonceTx,pPkt+1,f,lm);

    // Calculate CBC-MAC value
    memset(IV,0,sizeof(IV));
    halAesEncrypt(AES_MODE_CBCMAC,buf, i, cipherText, IV);

    // Encrypt authentication tag
    memcpy(IV,nonceTx,16);                      // Use nonce as IV
    IV[0]= 0x01;                                // Flag for CTR
    IV[15]= 0;                                  // Counter= 0

    memcpy(buf,cipherText,16);
    halAesEncrypt(AES_MODE_CTR,buf, 16, cipherText, IV);

    // Encrypt plaintext
    memset(buf+i,0,16-(i&0xf));                 // Zero padding
    IV[15]= 1;                                  // Counter= 1
    halAesEncrypt(AES_MODE_CTR,&buf[i-c], c, &cipherText[i-c], IV);

    // Fill TX buffer
    halRfWriteTxBuf(pPkt,1);                    // Length byte
    halRfAppendTxBuf(pPkt+1,f);                 // MPDU
    halRfAppendTxBuf(&cipherText[i-c],c);       // Payload
    halRfAppendTxBuf(cipherText,lm);            // MIC
}


/***********************************************************************************
* @fn      halRfReadRxBufSecure
*
* @brief   Decrypts and reverse authenticates with CCM then reads out received
*          frame
*
* @param   uint8* pPkt - data buffer. This must be allocated by caller.
*          uint8 length - number of bytes
*          uint8 encrLength - number of bytes to decrypt
*          uint8 authLength - number of bytes to reverse authenticate
*          uuint8 m - ets length of integrity code (m=1,2,3 gives lenght of integrity
*                   field 4,8,16)
*
* @return  SUCCESS or FAILED
*/
uint8 halRfReadRxBufSecure(uint8* data, uint8 length, uint8 c, uint8 f, uint8 m)
{
    uint8 lm,l,i;

    // Get received packet
    halRfReadRxBuf(data,length);

    // Update RX nonce; frame counter
    nonceRx[12]= data[10];
    nonceRx[11]= data[11];
    nonceRx[10]= data[12];
    nonceRx[9]= data[13];
    // Update RX nonce; source address
    nonceRx[7]= data[8];
    nonceRx[8]= data[7];

    // Decrypt authentication tag
    memcpy(IV,nonceRx,16);
    IV[15]= 0;                        // Counter= 0
    IV[0]= 0x01;                      // Flag field for decryption
    lm=MIC_2_MICLEN(m);               // Length of authentication tag

    halAesDecrypt(AES_MODE_CTR,data+c+f, lm, buf, IV);
    memcpy(data+c+f,buf,lm);

    // Prepare payload for decryption
    memcpy(cipherText,data+f,c);

    // Zero padding
    l= (c & 0x0f )==0 ? c : (c&0xf0) + 0x10;
    for (i=c; i<l; i++)
        cipherText[i]= 0x00;

    // Decrypt payload
    IV[15]= 1;                       // Counter= 1
    halAesDecrypt(AES_MODE_CTR,cipherText, c, buf, IV);
    memcpy(data+f,buf,c);

    // Generate B(i) vector
    memset(buf,0,sizeof(buf));
    i= generateAuthData(c,nonceRx,data,f,lm);

    // Calculate CBC-MAC value
    memset(IV,0,sizeof(IV));
    halAesEncrypt(AES_MODE_CBCMAC,buf, i, cipherText, IV);

    // Check authentication data
    return memcmp(cipherText,data+c+f,lm)==0 ? SUCCESS : FAILED;
}


/***********************************************************************************
* @fn      halRfIncNonceTx
*
* @brief   Increments frame counter field of stored nonce TX
*
* @param   none
*
* @return  none
*/
void halRfIncNonceTx(void)
{
    uint32 *pCount;

    pCount= (uint32*)&nonceTx[9];
    utilReverseBuf((uint8*)pCount,4);
    (*pCount)++;
    utilReverseBuf((uint8*)pCount,4);
}




/***********************************************************************************
* LOCAL FUNCTIONS
*/


static void halAesLoadBlock(uint8* pData, uint8 op)
{
   uint8 i;

   // Set operation
   AES_SET_OPERATION(op);
   // Starting loading of key or vector.
   AES_START();

   // loading the data (key or vector)
   for(i = 0; i < 16; i++){
      ENCDI = pData[i];
   }

   return;
}



static void halAesOperation(uint8 oper,uint8 *pDataIn, uint16 length, uint8 *pDataOut, uint8 *pInitVector)
{
   uint16 i;
   uint8 j, k;
   uint8 mode;
   uint16 nbrOfBlocks;
   uint16 convertedBlock;

   nbrOfBlocks = length / 0x10;

   if((length % 0x10) != 0){
      // length not multiplum of 16, convert one block extra with zeropadding
      nbrOfBlocks++;
   }

   // Loading the IV.
   halAesLoadBlock(pInitVector, AES_LOAD_IV);

   // Start the operation
   AES_SET_OPERATION(oper);

   // Getting the operation mode.
   mode = ENCCS & 0x70;

   for(convertedBlock = 0; convertedBlock < nbrOfBlocks; convertedBlock++){
      // Starting the conversion.
      AES_START();

      i = convertedBlock * 16;
      // Counter, Output Feedback and Cipher Feedback operates on 4 bytes and not 16 bytes.
      if((mode == AES_MODE_CFB) || (mode == AES_MODE_OFB) || (mode == AES_MODE_CTR)) {

         for(j = 0; j < 4; j++){
            // Writing the input data with zero-padding
            for(k = 0; k < 4; k++){
               ENCDI = ((i + 4*j + k < length) ? pDataIn[i + 4*j + k] : 0x00 );
            }

            // Read out data for every 4th byte
            for(k = 0; k < 4; k++){
               pDataOut[i + 4*j + k] = ENCDO;
            }

         }
      }
      else if (mode == AES_MODE_CBCMAC){
         // Writing the input data with zero-padding
         for(j = 0; j < 16; j++){
            ENCDI = ((i + j < length) ? pDataIn[i + j] : 0x00 );
         }
         // The last block of the CBC-MAC is computed by using CBC mode.
         if(convertedBlock == nbrOfBlocks - 2){
            AES_SET_MODE(AES_MODE_CBC);
            // wait for data ready
            halMcuWaitUs(1);
         }
         // The CBC-MAC does not produce an output on the n-1 first blocks
         // only the last block is read out
         else if(convertedBlock == nbrOfBlocks - 1){

             // wait for data ready
             halMcuWaitUs(1);
            for(j = 0; j < 16; j++){
               pDataOut[j] = ENCDO;
            }
         }
      } // ECB or CBC
      else{
         // Writing the input data with zero-padding
         for(j = 0; j < 16; j++){
            ENCDI = ((i+j < length) ? pDataIn[i+j] : 0x00 );
         }

         // wait for data ready
         halMcuWaitUs(1);

         // Read out data
         for(j = 0; j < 16; j++){
            pDataOut[i+j] = ENCDO;
         }
      }
   }
}

static void halAesEncrypt(uint8 mode, uint8 *pDataIn, uint16 length, uint8 *pDataOut, uint8 *pInitVector)
{
    AES_SET_MODE(mode);
    halAesOperation(AES_ENCRYPT,pDataIn,length,pDataOut,pInitVector);
}


static void halAesDecrypt(uint8 mode, uint8 *pDataIn, uint16 length, uint8 *pDataOut, uint8 *pInitVector)
{
    AES_SET_MODE(mode);
    halAesOperation(AES_DECRYPT,pDataIn,length,pDataOut,pInitVector);
}



static uint8 generateAuthData(uint8 c, uint8 *pIv, uint8 *pData, uint8 f, uint8 lm)
{
    uint8 i,j,l;

    // Prepare input for CBC-MAC, B0, Bi.... Bn
    memcpy(buf,pIv,16);             // B0

    // Create flag
    buf[0]=  0x01;                  // L'= L - 1 (L=2)
    if (f>0)
        buf[0]|= 0x40;              // Adata= f > 0
    buf[0]|=  ( (lm-2) / 2 ) <<3;   // M'= (lm-2) / 2;

    buf[14]= 0x00;
    buf[15]= c;

    // Add authentication data
    buf[16]= 0;
    buf[17]= f;
    memcpy(&buf[18],pData,f);

    // .... zero padding
    i= 18 + f;
	l= (i & 0x0f )==0 ? i : (i&0xf0) + 0x10;
    while(i<l)
        buf[i++]= 0x00;

    // Add command payload
    j= 0;
    while(i<l+c) {
        buf[i++]= pData[f+j];
        j++;
    }

    return i;
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
