/*******************************************************************************************
File: uart.h

********************************************************************************************/

#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
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
    UART_BAUD_600    = B600,
    UART_BAUD_1200   = B1200,
    UART_BAUD_1800   = B1800,
    UART_BAUD_2400   = B2400,
    UART_BAUD_4800   = B4800,
    UART_BAUD_9600   = B9600,
    UART_BAUD_19200  = B19200,
    UART_BAUD_38400  = B38400,
    UART_BAUD_57600  = B57600,
    UART_BAUD_115200 = B115200
} UART_BAUD;

typedef enum
{
    UART_STOP_BITS_1 = 0, // most common
    UART_STOP_BITS_2 = CSTOPB
} UART_STOP_BITS;

typedef enum
{
    UART_PARITY_NONE = 0, // most common
    UART_PARITY_EVEN = PARENB,
    UART_PARITY_ODD  = PARENB | PARODD
} UART_PARITY;

typedef enum
{
    UART_HFC_DISABLED = 0,      // most common
    UART_HFC_ENABLED  = CRTSCTS // often used for modems
} UART_HARDWARE_FLOW_CONTROL;

typedef enum
{
    UART_MODE_RX,
    UART_MODE_TX,
    UART_MODE_RX_TX
} UART_MODE;

typedef uint32_t UART_HANDLE;
typedef void (*UART_RX_PFN)(UART_HANDLE /*uartHandle*/, void* /*pData*/, uint32_t /*dataLen*/);

typedef struct
{
    UART_BAUD baud;
    UART_MODE mode;
    UART_STOP_BITS stopBits;
    UART_PARITY parity;
    UART_HARDWARE_FLOW_CONTROL hardwareFlowCtrl;
    UART_RX_PFN rxCallback;
} UART_PARAMS;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
void UartInit(void);
UART_HANDLE UartRegister(char const * pDevice, UART_PARAMS const * pParams);
void UartUnregister(UART_HANDLE uartHandle);
void UartWriteBlocking(UART_HANDLE uartHandle, void const * pSrc, uint32_t length);
#endif
