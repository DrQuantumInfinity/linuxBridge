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
} UART;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static UART uart[SERIAL_NUM_DEVICES];
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void UartConfigureSettings(UART * puart, const char * pDevice, const UART_PARAMS * pParams);
// callbacks
static void UartStartReceiveThread(UART * puart);
static void UartReceiveHandler(UART * puart);
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
    UART * puart = NULL;
    ASSERT_PARAM(uartIndex < NELEMENTS(uart));
    if (uartIndex < NELEMENTS(uart))
    {
        puart = &uart[uartIndex];
        UartConfigureSettings(puart, pDevice, pParams);
        if (puart->uartFileDescriptor >= 0 && (pParams->mode == UART_MODE_RX || pParams->mode == UART_MODE_RX_TX))
        {
            UartStartReceiveThread(puart);
        }
    }
    return puart ? puart->uartHandle : 0;
}
void UartUnregister(UART_HANDLE uartHandle)
{
    UART * puart = UartValidateHandle(uartHandle);
    close(puart->uartFileDescriptor);
    memset(puart, 0x00, sizeof(*puart));
}
void UartWriteBlocking(UART_HANDLE uartHandle, const void * pSrc, uint32_t length)
{
    // lint -esym(529, writeResult)
    UART * puart = UartValidateHandle(uartHandle);
    /*int32_t writeResult = */ write(puart->uartFileDescriptor, pSrc, length);
    // ASSERT_PARAM(writeResult == length);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void UartConfigureSettings(UART * puart, const char * pDevice, const UART_PARAMS * pParams)
{
    ASSERT_PARAM(pDevice);
    puart->uartFileDescriptor = open(pDevice,
                                     O_RDWR |       // read and write
                                         O_NOCTTY | // not a controller terminal
                                         O_SYNC);   // write calls are blocking TODO: consider changing this? O_DIRECT????
#ifdef DEBUGGING
    if (puart->uartFileDescriptor < 0)
    {
        puts("\nopen linux and enter \"sudo chmod 666 /dev/ttyS?\" in the command line");
        return;
    }
#endif
    ASSERT_PARAM(puart->uartFileDescriptor >= 0); // open linux and enter "sudo chmod 666 /dev/ttyS9" in the command line.

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

    tcsetattr(puart->uartFileDescriptor, TCSANOW, &tIoSetup);
}
static void UartStartReceiveThread(UART * puart)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (THREAD_PFN) UartReceiveHandler, puart);
}
static void UartReceiveHandler(UART * puart)
{
    UART_DATA_RX dataReceivedEvent;
    dataReceivedEvent.uartHandle = puart->uartHandle;
    for (;;)
    {
        /*int32_t readLength = */ read(puart->uartFileDescriptor, puart->rxBuf, SERIAL_RECEIVE_BUF_SIZE);
        // ASSERT_PARAM(readLength > 0);

        // TODO: use a callback here to call into EspNow transport serial handler
        //  MsgPostDouble(MID_UART_RX, &dataReceivedEvent, sizeof(dataReceivedEvent), puart->rxBuf, (uint32_t) readLength);
    }
}
static UART * UartValidateHandle(UART_HANDLE uartHandle)
{
    uartHandle--;
    ASSERT_PARAM(uartHandle < NELEMENTS(uart));
    return &uart[uartHandle];
}
