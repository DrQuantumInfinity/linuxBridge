
#include "transportLocal.h"

#include "Log.h"
// Devices
#include "DevicePing.h"

#include "mqttWrapper.h"
#include "string"
#include "timer.h"

using namespace ::chip;
using namespace ::chip::app::Clusters;
using namespace std;
/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define STR_CMP_MATCH 0
#define ADD_TOPIC "ping/addIP"
#define REM_TOPIC "ping/remIP"
 /**************************************************************************
  *                                  Macros
  **************************************************************************/
  /**************************************************************************
   *                                  Types
   **************************************************************************/
typedef void* (*THREAD_PFN)(void*);
#define PING_SWITCH_IP_ADDRESS_PAUL         "192.168.0.16"
#define PING_SWITCH_IP_ADDRESS_GORDON       "192.168.0.14"
#define PING_SWITCH_IP_ADDRESS_CONRAD       "192.168.0.15"
#define PING_SWITCH_IP_ADDRESS_MAX          "192.168.0.17"
/**************************************************************************
 *                                  Macros
 **************************************************************************/
 /**************************************************************************
  *                                  Types
  **************************************************************************/
typedef void* (*THREAD_PFN)(void*);

typedef struct {
    char name[PING_MAX_DEVICE_NAME_LENGTH];
    char room[ENDPOINT_LOCATION_LENGTH];
    char ipAddress[PING_IP_ADDRESS_LENGTH];
}PersistLocal;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
struct TransportLocal::Private {
    static void NewDeviceOnPwr(int index, void* pPersist);
    static Device* NewDevice(uint16_t index, PersistLocal* persistLocal);

    static void PingAddHardcodedIpAddress(const char* pName, const char* pIpAddress);
};

/**************************************************************************
 *                                  Variables
 **************************************************************************/
DeviceList TransportLocal::_deviceList; // static variables in a class need to be independently initialized. C++ is dumb
mqtt_inst* TransportLocal::_mqttInst;
PersistDevList TransportLocal::_persistList = PersistDevList(sizeof(PersistLocal), "localPersist.bin");
/**************************************************************************
 *                                  Static Functions
 **************************************************************************/
void TransportLocal::Init(void)
{

    TransportLocal::_mqttInst = mqtt_wrap_init("192.168.0.128", TransportLocal::HandleTopicRx);

    mqtt_wrap_add_sub(TransportLocal::_mqttInst, ADD_TOPIC);
    mqtt_wrap_add_sub(TransportLocal::_mqttInst, REM_TOPIC);

    mqtt_wrap_loopstart(TransportLocal::_mqttInst);

    _persistList.Apply(TransportLocal::Private::NewDeviceOnPwr);
    
    
    TransportLocal::Private::PingAddHardcodedIpAddress("Phone Paul: " PING_SWITCH_IP_ADDRESS_PAUL, PING_SWITCH_IP_ADDRESS_PAUL);
    TransportLocal::Private::PingAddHardcodedIpAddress("Phone Gordon: " PING_SWITCH_IP_ADDRESS_GORDON, PING_SWITCH_IP_ADDRESS_GORDON);
    TransportLocal::Private::PingAddHardcodedIpAddress("Phone Conrad: " PING_SWITCH_IP_ADDRESS_CONRAD, PING_SWITCH_IP_ADDRESS_CONRAD);
    TransportLocal::Private::PingAddHardcodedIpAddress("Phone Max: " PING_SWITCH_IP_ADDRESS_MAX, PING_SWITCH_IP_ADDRESS_MAX);
}
void TransportLocal::HandleTopicRx(const char* pTopic, const char* pPayload)
{
    log_info("HandleTopicRx. topic: %s. payload: %s", pTopic, pPayload);
    if (strcmp(pTopic, ADD_TOPIC))
    {
        Device* pDevice = _deviceList.GetDevice(pPayload);
        if (pDevice == NULL)
        {
            PersistLocal persistData;
            memset(&persistData, 0x00, sizeof(persistData));
            strncat(persistData.name, pPayload, sizeof(persistData.name) - 1);
            strncat(persistData.room, "Bridge", sizeof(persistData.room) - 1);
            pDevice = TransportLocal::Private::NewDevice(DEVICE_INVALID, &persistData);
            _persistList.Upsert(pDevice->GetIndex(), &persistData);
            log_info("Added device: %s", pPayload);
        }
        _deviceList.Upsert(pPayload, pDevice);
    }
    else if (strcmp(pTopic, REM_TOPIC))
    {
        _deviceList.Remove(pPayload);
        log_info("Removed device: %s", pPayload);
    }
    else
    {
        log_warn("Invalid MQTT topic: %s", pTopic);
        // TODO: log something about an invalid MQTT message
    }
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
TransportLocal::TransportLocal(const char* pIpAddress)
{

}
TransportLocal::~TransportLocal(void)
{
}
/**************************************************************************
 *                                  Protected Functions
 **************************************************************************/
void TransportLocal::Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    //Do something based on the command coming from Google.
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
void TransportLocal::Private::PingAddHardcodedIpAddress(const char* pName, const char* pIpAddress)
{
    PersistLocal persistData;
    memset(&persistData, 0x00, sizeof(persistData));
    strncat(persistData.name, pName, sizeof(persistData.name) - 1);
    strncat(persistData.room, "Bridge", sizeof(persistData.room) - 1);
    strncat(persistData.ipAddress, pIpAddress, sizeof(persistData.ipAddress) - 1);
    Device* pDevice = TransportLocal::Private::NewDevice(DEVICE_INVALID, &persistData);
    _deviceList.Upsert(persistData.name, pDevice);
}
void TransportLocal::Private::NewDeviceOnPwr(int index, void* pPersist)
{
    Device* pDevice = NewDevice((uint16_t)index, (PersistLocal*)pPersist);
    _deviceList.Upsert(((PersistLocal*)pPersist)->name, pDevice);
}
Device* TransportLocal::Private::NewDevice(uint16_t index, PersistLocal* pPersist)
{
    TransportLayer* pTransport = new TransportLocal(pPersist->ipAddress);
    return new DevicePing(pPersist->name, pPersist->room, pTransport, index, pPersist->ipAddress);
}