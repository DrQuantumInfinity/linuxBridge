
#include "transportPing.h"

#include "Log.h"
// Devices
#include "DeviceButton.h"
#include "DeviceLightLevel.h"
#include "DeviceLightRGB.h"

#include "pingWrapper.h"
#include "string"
#include "timer.h"

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace std;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define STR_CMP_MATCH 0
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
typedef void * (*THREAD_PFN)(void *);

typedef struct {
    char name[PING_MAX_DEVICE_NAME_LENGTH];
    char room[ENDPOINT_LOCATION_LENGTH];
}PersistPing;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
struct TransportPing::Private {
    static PING_TYPE GetDeviceType(const char* pTopic);
    static void NewDeviceOnPwr(int index, void* pPersist);
    static Device* NewDevice(uint16_t index, PersistPing* persistPing);

    static void GoogleSend(PING_TYPE type, const char* pTopic, const char* pPayload, Device* pDevice);
    static void GoogleSendOutlet(const char* pTopic, const char* pPayload, DeviceButton* pDevice);

    static void PingSend(TransportPing& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId);
};

/**************************************************************************
 *                                  Variables
 **************************************************************************/
DeviceList TransportPing::_deviceList; // static variables in a class need to be independently initialized. C++ is dumb
ping_inst* TransportPing::_pingInst;
PersistDevList TransportPing::_persistList = PersistDevList(sizeof(PersistPing), "pingPersist.bin");
/**************************************************************************
 *                                  Static Functions
 **************************************************************************/
void TransportPing::Init(void)
{
    _persistList.Apply(TransportPing::Private::NewDeviceOnPwr);
    PingStartThread(void);
}
void TransportPing::HandleTopicRx(const char* pTopic, const char* pPayload)
{
    log_info("HandleTopicRx. topic: %s. payload: %s", pTopic, pPayload);
    PING_TYPE type = Private::GetDeviceType(pTopic);
    if (type < PING_TYPE_COUNT)
    {
        char name[PING_MAX_DEVICE_NAME_LENGTH];
        memcpy(name, pTopic, pingDeviceTopicLengths[type]);
        name[pingDeviceTopicLengths[type]] = '\0';

        Device* pDevice = _deviceList.GetDevice(name);
        if (pDevice == NULL)
        {
            PersistPing persistData;
            memset(&persistData, 0x00, sizeof(persistData));
            strncat(persistData.name, name, sizeof(persistData.name)-1);
            strncat(persistData.room, "Bridge", sizeof(persistData.room)-1);
            persistData.type = type;
            pDevice = TransportPing::Private::NewDevice(DEVICE_INVALID, &persistData);
            _persistList.Upsert(pDevice->GetIndex(), &persistData);
        }
        _deviceList.Upsert(name, pDevice);
        Private::GoogleSend(type, pTopic, pPayload, pDevice);
    }
    else
    {
        log_warn("Invalid PING topic: %s", pTopic);
        // TODO: log something about an invalid PING message
    }
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
TransportPing::TransportPing(const char* pIpAddress)
{
    strcpy(_ipAddress, pIpAddress);
}
TransportPing::~TransportPing(void)
{
}
/**************************************************************************
 *                                  Protected Functions
 **************************************************************************/
void TransportPing::Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    Private::PingSend(*this, pDevice, clusterId, attributeMetadata->attributeId);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
void PingStartThread(void)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (THREAD_PFN) UartReceiveHandler, NULL);
}
void TransportPing::Private::NewDeviceOnPwr(int index, void* pPersist)
{
    Device* pDevice = NewDevice((uint16_t)index, (PersistPing*)pPersist);
    _deviceList.Upsert(((PersistPing*)pPersist)->name, pDevice);
}
Device* TransportPing::Private::NewDevice(uint16_t index, PersistPing* pPersist)
{
    TransportLayer* pTransport = new TransportPing(pPersist->type, &pPersist->name[strlen(pPingDeviceTypes[pPersist->type])]);
    return new DeviceButton(pPersist->name, pPersist->room, pTransport, index);
}
// Send to Google functions
void TransportPing::Private::GoogleSend(const char* pTopic, const char* pPayload, Device* pDevice)
{
    Private::GoogleSendOutlet(pTopic, pPayload, (DeviceButton*)pDevice);
}
void TransportPing::Private::GoogleSendOutlet(const char* pTopic, const char* pPayload, DeviceButton* pDevice)
{
    /*Parse the topic for which value is changing*/
    pDevice->SetOn(true /*or false...*/);
}
// Send to PING device functions
void TransportPing::Private::PingSend(TransportPing& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId)
{
    //This can be ignored. Users can not edit the state of whether an IP address is present.
}