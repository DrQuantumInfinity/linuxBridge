/*******************************************************************************************
File: usart.c

********************************************************************************************/

#include "usart.h"
// #include "mids.h"
#include <fcntl.h>
#include <pthread.h>
//#include <stdio.h>
#include <unistd.h>
#include <string.h>
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define SERIAL_NUM_DEVICES 1
#define SERIAL_RECEIVE_BUF_SIZE 255
/**************************************************************************
 *                                  Macros
 **************************************************************************/
#if SERIAL_NUM_DEVICES == 0
#error "no serial devices are enabled"
#endif

#define NELEMENTS(array)    (sizeof(array) / sizeof(array[0]))
#define BIT_SET(var, mask)  { (var) |=  (mask); }
#define ASSERT_PARAM(cond)
#ifndef MIN
#define MIN(x, y)		( ((x) < (y))? (x) : (y) )
#endif
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void *(*THREAD_PFN)(void*);

typedef struct
{
    USART_HANDLE usartHandle;
    int32_t usartFileDescriptor;
    uint8_t rxBuf[SERIAL_RECEIVE_BUF_SIZE];
} USART;
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static USART usart[SERIAL_NUM_DEVICES];
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
static void UsartConfigureSettings(USART* pUsart, const char* pDevice, const USART_PARAMS* pParams);
// callbacks
static void UsartStartReceiveThread(USART* pUsart);
static void UsartReceiveHandler(USART* pUsart);
static USART* UsartValidateHandle(USART_HANDLE usartHandle);
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
// Init
void UsartInit(void)
{
    memset(usart, 0x00, sizeof(usart));
}
USART_HANDLE UsartRegister(const char* pDevice, const USART_PARAMS* pParams)
{
    uint32_t usartIndex;
    for (usartIndex = 0; usartIndex < NELEMENTS(usart); usartIndex++)
    {
        if (usart[usartIndex].usartHandle == 0)
        {
            usart[usartIndex].usartHandle = usartIndex + 1;
            break;
        }
    }
    USART* pUsart = NULL;
    ASSERT_PARAM(usartIndex < NELEMENTS(usart));
    if (usartIndex < NELEMENTS(usart))
    {
        pUsart = &usart[usartIndex];
        UsartConfigureSettings(pUsart, pDevice, pParams);
        if (pUsart->usartFileDescriptor >= 0 && (pParams->mode == USART_MODE_RX || pParams->mode == USART_MODE_RX_TX))
        {
            UsartStartReceiveThread(pUsart);
        }
    }
    return pUsart ? pUsart->usartHandle : 0;
}
void UsartUnregister(USART_HANDLE usartHandle)
{
    USART* pUsart = UsartValidateHandle(usartHandle);
    close(pUsart->usartFileDescriptor);
    memset(pUsart, 0x00, sizeof(*pUsart));
}
void UsartWriteBlocking(USART_HANDLE usartHandle, const void* pSrc, uint32_t length)
{
    // lint -esym(529, writeResult)
    USART* pUsart      = UsartValidateHandle(usartHandle);
    /*int32_t writeResult = */write(pUsart->usartFileDescriptor, pSrc, length);
    //ASSERT_PARAM(writeResult == length);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
static void UsartConfigureSettings(USART* pUsart, const char* pDevice, const USART_PARAMS* pParams)
{
    ASSERT_PARAM(pDevice);
    pUsart->usartFileDescriptor = open(pDevice,
                                       O_RDWR |   // read and write
                                       O_NOCTTY | // not a controller terminal
                                       O_SYNC);   // write calls are blocking TODO: consider changing this? O_DIRECT????
#ifdef DEBUGGING
    if (pUsart->usartFileDescriptor < 0)
    {
        puts("\nopen linux and enter \"sudo chmod 666 /dev/ttyS?\" in the command line");
        return;
    }
#endif
    ASSERT_PARAM(pUsart->usartFileDescriptor >= 0); // open linux and enter "sudo chmod 666 /dev/ttyS9" in the command line.

    struct termios tIoSetup;
    memset(&tIoSetup, 0x00, sizeof(tIoSetup));
    if (pParams->mode == USART_MODE_RX || pParams->mode == USART_MODE_RX_TX)
    {
        BIT_SET(tIoSetup.c_cflag, CREAD); // Enable reading
    }
    BIT_SET(tIoSetup.c_cflag, CLOCAL); // Do not change owner of port
    BIT_SET(tIoSetup.c_cflag, CS8);    // 8 data bits
    BIT_SET(tIoSetup.c_cflag, pParams->baud);
    BIT_SET(tIoSetup.c_cflag, pParams->hardwareFlowCtrl);
    BIT_SET(tIoSetup.c_cflag, pParams->parity);
    BIT_SET(tIoSetup.c_cflag, pParams->stopBits);

    if (pParams->parity != USART_PARITY_NONE)
    {
        BIT_SET(tIoSetup.c_iflag, INPCK);  // enable parity checking
        BIT_SET(tIoSetup.c_iflag, ISTRIP); // strip the parity bits
    }

    tIoSetup.c_oflag = 0; // No need for output processing

    tIoSetup.c_lflag = 0; // Using non-canonical input

    tIoSetup.c_cc[VMIN]  = MIN(SERIAL_RECEIVE_BUF_SIZE, 255); // lint !e506 //wait for this many characters (or expiry time)
    tIoSetup.c_cc[VTIME] = 1; // time to wait for input in 10ths of a second (reset after every character received)

    tcsetattr(pUsart->usartFileDescriptor, TCSANOW, &tIoSetup);
}
static void UsartStartReceiveThread(USART* pUsart)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (THREAD_PFN)UsartReceiveHandler, pUsart);
}
static void UsartReceiveHandler(USART* pUsart)
{
    USART_DATA_RX dataReceivedEvent;
    dataReceivedEvent.usartHandle = pUsart->usartHandle;
    for (;;)
    {
        /*int32_t readLength = */read(pUsart->usartFileDescriptor, pUsart->rxBuf, SERIAL_RECEIVE_BUF_SIZE);
        //ASSERT_PARAM(readLength > 0);
        
        //TODO: use a callback here to call into EspNow transport serial handler
        // MsgPostDouble(MID_USART_RX, &dataReceivedEvent, sizeof(dataReceivedEvent), pUsart->rxBuf, (uint32_t) readLength);
    }
}
static USART* UsartValidateHandle(USART_HANDLE usartHandle)
{
    usartHandle--;
    ASSERT_PARAM(usartHandle < NELEMENTS(usart));
    return &usart[usartHandle];
}
