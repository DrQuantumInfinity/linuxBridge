/*******************************************************************************************
File: uart.c

********************************************************************************************/

#include "uart.h"
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define SERIAL_NUM_DEVICES 1 // TODO: define this somewhere else

#define SERIAL_RECEIVE_BUF_SIZE 255
/**************************************************************************
 *                                  Macros
 **************************************************************************/
#if SERIAL_NUM_DEVICES == 0
#error "no serial devices are enabled"
#endif

// TODO: these includes should come from a common "utils.h"
#define NELEMENTS(array) (sizeof(array) / sizeof(array[0]))
#define BIT_SET(var, mask)                                                                                                         \
    {                                                                                                                              \
        (var) |= (mask);                                                                                                           \
    }
#define ASSERT_PARAM(cond)
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void * (*THREAD_PFN)(void *);

typedef struct
{
    UART_HANDLE uartHandle;
    int32_t uartFileDescriptor;
    uint8_t rxBuf[SERIAL_RECEIVE_BUF_SIZE];
    UART_RX_PFN rxCallback;
} UART;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static UART uart[SERIAL_NUM_DEVICES];
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void UartConfigureSettings(UART * pUart, const char * pDevice, const UART_PARAMS * pParams);
// callbacks
static void UartStartReceiveThread(UART * pUart);
static void UartReceiveHandler(UART * pUart);
static UART * UartValidateHandle(UART_HANDLE uartHandle);
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
// Init
void UartInit(void)
{
    memset(uart, 0x00, sizeof(uart));
}
UART_HANDLE UartRegister(const char * pDevice, const UART_PARAMS * pParams)
{
    uint32_t uartIndex;
    for (uartIndex = 0; uartIndex < NELEMENTS(uart); uartIndex++)
    {
        if (uart[uartIndex].uartHandle == 0)
        {
            uart[uartIndex].uartHandle = uartIndex + 1;
            break;
        }
    }
    UART * pUart = NULL;
    ASSERT_PARAM(uartIndex < NELEMENTS(uart));
    if (uartIndex < NELEMENTS(uart))
    {
        pUart = &uart[uartIndex];
        pUart->rxCallback = pParams->rxCallback;
        UartConfigureSettings(pUart, pDevice, pParams);
        if (pUart->uartFileDescriptor >= 0 && (pParams->mode == UART_MODE_RX || pParams->mode == UART_MODE_RX_TX))
        {
            UartStartReceiveThread(pUart);
        }
    }
    return pUart ? pUart->uartHandle : 0;
}
void UartUnregister(UART_HANDLE uartHandle)
{
    UART * pUart = UartValidateHandle(uartHandle);
    close(pUart->uartFileDescriptor);
    memset(pUart, 0x00, sizeof(*pUart));
}
void UartWriteBlocking(UART_HANDLE uartHandle, const void * pSrc, uint32_t length)
{
    // lint -esym(529, writeResult)
    UART * pUart = UartValidateHandle(uartHandle);
    /*int32_t writeResult = */ write(pUart->uartFileDescriptor, pSrc, length);
    // ASSERT_PARAM(writeResult == length);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void UartConfigureSettings(UART * pUart, const char * pDevice, const UART_PARAMS * pParams)
{
    ASSERT_PARAM(pDevice);
    pUart->uartFileDescriptor = open(pDevice,
                                     O_RDWR |       // read and write
                                         O_NOCTTY | // not a controller terminal
                                         O_SYNC);   // write calls are blocking TODO: consider changing this? O_DIRECT????
#ifdef DEBUGGING
    if (pUart->uartFileDescriptor < 0)
    {
        puts("\nopen linux and enter \"sudo chmod 666 /dev/ttyS?\" in the command line");
        return;
    }
#endif
    ASSERT_PARAM(pUart->uartFileDescriptor >= 0); // open linux and enter "sudo chmod 666 /dev/ttyS9" in the command line.

    struct termios tIoSetup;
    memset(&tIoSetup, 0x00, sizeof(tIoSetup));
    if (pParams->mode == UART_MODE_RX || pParams->mode == UART_MODE_RX_TX)
    {
        BIT_SET(tIoSetup.c_cflag, CREAD); // Enable reading
    }
    BIT_SET(tIoSetup.c_cflag, CLOCAL); // Do not change owner of port
    BIT_SET(tIoSetup.c_cflag, CS8);    // 8 data bits
    BIT_SET(tIoSetup.c_cflag, pParams->baud);
    BIT_SET(tIoSetup.c_cflag, pParams->hardwareFlowCtrl);
    BIT_SET(tIoSetup.c_cflag, pParams->parity);
    BIT_SET(tIoSetup.c_cflag, pParams->stopBits);

    if (pParams->parity != UART_PARITY_NONE)
    {
        BIT_SET(tIoSetup.c_iflag, INPCK);  // enable parity checking
        BIT_SET(tIoSetup.c_iflag, ISTRIP); // strip the parity bits
    }

    tIoSetup.c_oflag = 0; // No need for output processing

    tIoSetup.c_lflag = 0; // Using non-canonical input

    tIoSetup.c_cc[VMIN]  = MIN(SERIAL_RECEIVE_BUF_SIZE, 255); // lint !e506 //wait for this many characters (or expiry time)
    tIoSetup.c_cc[VTIME] = 1; // time to wait for input in 10ths of a second (reset after every character received)

    tcsetattr(pUart->uartFileDescriptor, TCSANOW, &tIoSetup);
}
static void UartStartReceiveThread(UART * pUart)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (THREAD_PFN) UartReceiveHandler, pUart);
}
static void UartReceiveHandler(UART * pUart)
{
    for (;;)
    {
        int32_t readLength = read(pUart->uartFileDescriptor, pUart->rxBuf, SERIAL_RECEIVE_BUF_SIZE);
        // ASSERT_PARAM(readLength > 0);

        // TODO: use a callback here to call into EspNow transport serial handler
        //  MsgPostDouble(MID_UART_RX, &pUart->uartHandle, sizeof(pUart->uartHandle), pUart->rxBuf, (uint32_t) readLength);
        pUart->rxCallback(pUart->uartHandle, pUart->rxBuf, readLength);
    }
}
static UART * UartValidateHandle(UART_HANDLE uartHandle)
{
    uartHandle--;
    ASSERT_PARAM(uartHandle < NELEMENTS(uart));
    return &uart[uartHandle];
}
