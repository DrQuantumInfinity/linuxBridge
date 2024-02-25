/*******************************************************************************************
File: usart.h

********************************************************************************************/

#ifndef __USART_H__
#define __USART_H__

#include "includes.h"
#include <termios.h>

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macro Tables
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef enum
{
   USART_BAUD_600    = B600,
   USART_BAUD_1200   = B1200,
   USART_BAUD_1800   = B1800,
   USART_BAUD_2400   = B2400,
   USART_BAUD_4800   = B4800,
   USART_BAUD_9600   = B9600,
   USART_BAUD_19200  = B19200,
   USART_BAUD_38400  = B38400,
   USART_BAUD_57600  = B57600,
   USART_BAUD_115200 = B115200
}USART_BAUD;

typedef enum
{
   USART_STOP_BITS_1 = 0, //most common
   USART_STOP_BITS_2 = CSTOPB
}USART_STOP_BITS;

typedef enum
{
   USART_PARITY_NONE = 0, //most common
   USART_PARITY_EVEN = PARENB,
   USART_PARITY_ODD  = PARENB | PARODD
}USART_PARITY;

typedef enum
{
   USART_HFC_DISABLED = 0, //most common
   USART_HFC_ENABLED  = CRTSCTS //often used for modems
}USART_HARDWARE_FLOW_CONTROL;

typedef enum
{
   USART_MODE_RX,
   USART_MODE_TX,
   USART_MODE_RX_TX
}USART_MODE;

typedef struct
{
   USART_BAUD baud;
   USART_MODE mode;
   USART_STOP_BITS stopBits;
   USART_PARITY parity;
   USART_HARDWARE_FLOW_CONTROL hardwareFlowCtrl;
}USART_PARAMS;

typedef UINT32 USART_HANDLE;

typedef struct
{
   USART_HANDLE usartHandle;
   UINT8 data[];
}USART_DATA_RX;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
USART_HANDLE UsartRegister(char const *pDevice, USART_PARAMS const *pParams);
void UsartUnregister(USART_HANDLE usartHandle);
void UsartWriteBlocking(USART_HANDLE usartHandle, void const *pSrc, UINT32 length);
#endif

