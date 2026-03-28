//NOTE: This is a fake transport layer. There's nothing being transported 
//since there are no external devices to communicate with. The device is 
//local to the RaspberryPi.

#include "transportPing.h"

#include "Log.h"
// Devices
#include "DevicePing.h"

#include "mqttWrapper.h"
#include "string"
#include "timer.h"
#include <fstream>
#include <cstring>

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
#define PING_CONFIG_FILE "pingDevices.conf"
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
}PersistPing;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
struct TransportPing::Private {
    static void NewDeviceOnPwr(int index, void* pPersist);
    static Device* NewDevice(uint16_t index, PersistPing* persistPing);

    static void StartThread(void);
    static void Run(void* pArgs);
    static void PingAddHardcodedIpAddress(const char* pName, const char* pIpAddress);

    static void PingSend(TransportPing& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId, uint8_t* buffer);
};

/**************************************************************************
 *                                  Variables
 **************************************************************************/
DeviceList TransportPing::_deviceList; // static variables in a class need to be independently initialized. C++ is dumb
mqtt_inst* TransportPing::_mqttInst;
PersistDevList TransportPing::_persistList = PersistDevList(sizeof(PersistPing), "pingPersist.bin");
/**************************************************************************
 *                                  Static Functions
 **************************************************************************/
void TransportPing::Init(void)
{

    // TransportPing::_mqttInst = mqtt_wrap_init("localhost", TransportPing::HandleTopicRx);
    // TransportPing::_mqttInst = mqtt_wrap_init("192.168.0.217", TransportPing::HandleTopicRx);
    TransportPing::_mqttInst = mqtt_wrap_init("0.0.0.0", TransportPing::HandleTopicRx);

    mqtt_wrap_add_sub(TransportPing::_mqttInst, ADD_TOPIC);
    mqtt_wrap_add_sub(TransportPing::_mqttInst, REM_TOPIC);

    mqtt_wrap_loopstart(TransportPing::_mqttInst);

    //_persistList.Apply(TransportPing::Private::NewDeviceOnPwr);
    TransportPing::Private::StartThread();
}
void TransportPing::HandleTopicRx(const char* pTopic, const char* pPayload)
{
    log_info("HandleTopicRx. topic: %s. payload: %s", pTopic, pPayload);
    if (strcmp(pTopic, ADD_TOPIC) == 0)
    {
        Device* pDevice = _deviceList.GetDevice(pPayload);
        if (pDevice == NULL)
        {
            PersistPing persistData;
            memset(&persistData, 0x00, sizeof(persistData));
            strncat(persistData.name, pPayload, sizeof(persistData.name) - 1);
            strncat(persistData.room, "Bridge", sizeof(persistData.room) - 1);
            strcat(persistData.ipAddress, pPayload);
            pDevice = TransportPing::Private::NewDevice(DEVICE_INVALID, &persistData);
            if (pDevice)
            {
                _persistList.Upsert(pDevice->GetIndex(), &persistData);
            }
            log_info("Added device: %s", pPayload);
        }
        _deviceList.Upsert(pPayload, pDevice);
    }
    else if (strcmp(pTopic, REM_TOPIC) == 0)
    {
        Device* pDevice = _deviceList.GetDevice(pPayload);
        if (pDevice != NULL)
        {
            _deviceList.Remove(pPayload);
            _persistList.Remove(pDevice->GetIndex());
            log_info("Removed device: %s", pPayload);
        }
        else
        {
            log_info("Could not find device to remove: %s", pPayload);
        }
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
TransportPing::TransportPing(const char* pIpAddress)
{

}
TransportPing::~TransportPing(void)
{
}
/**************************************************************************
 *                                  Protected Functions
 **************************************************************************/
void TransportPing::Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    Private::PingSend(*this, pDevice, clusterId, attributeMetadata->attributeId, buffer);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
void TransportPing::Private::StartThread(void)
{
    pthread_t thread;
    pthread_create(&thread, NULL, (THREAD_PFN)TransportPing::Private::Run, NULL);
}
void TransportPing::Private::Run(void* pArgs)
{
    sleep(1); //Make sure persistent devices get added before config file ones.
    std::ifstream configFile(PING_CONFIG_FILE);
    if (configFile.is_open())
    {
        std::string line;
        while (std::getline(configFile, line))
        {
            if (line.empty() || line[0] == '#')
                continue;
            size_t comma = line.find(',');
            if (comma == std::string::npos)
                continue;
            std::string name = line.substr(0, comma);
            std::string ip = line.substr(comma + 1);
            std::string fullName = name + ": " + ip;
            TransportPing::Private::PingAddHardcodedIpAddress(fullName.c_str(), ip.c_str());
        }
        configFile.close();
    }
    else
    {
        log_warn("No %s found, skipping ping devices", PING_CONFIG_FILE);
    }

    DevicePing* pDevice;
    while (true)
    {
        pDevice = (DevicePing*)_deviceList.GetFirstDevice();
        while (pDevice != NULL)
        {
            pDevice->SendPing();
            pDevice = (DevicePing*)_deviceList.GetNextDevice();
        }
        sleep(5);
    }
}
void TransportPing::Private::PingAddHardcodedIpAddress(const char* pName, const char* pIpAddress)
{
    PersistPing persistData;
    memset(&persistData, 0x00, sizeof(persistData));
    strncat(persistData.name, pName, sizeof(persistData.name) - 1);
    strncat(persistData.room, "Bridge", sizeof(persistData.room) - 1);
    strncat(persistData.ipAddress, pIpAddress, sizeof(persistData.ipAddress) - 1);
    Device* pDevice = TransportPing::Private::NewDevice(DEVICE_INVALID, &persistData);
    _deviceList.Upsert(persistData.name, pDevice);
}
void TransportPing::Private::NewDeviceOnPwr(int index, void* pPersist)
{
    Device* pDevice = NewDevice((uint16_t)index, (PersistPing*)pPersist);
    if (pDevice)
    {
        _deviceList.Upsert(((PersistPing*)pPersist)->name, pDevice);
    }
}
Device* TransportPing::Private::NewDevice(uint16_t index, PersistPing* pPersist)
{
    log_info("NewDevice. name: %s. ip: %s", pPersist->name, pPersist->ipAddress);

    TransportLayer* pTransport = new TransportPing(pPersist->ipAddress);
    return new DevicePing(pPersist->name, pPersist->room, pTransport, index, pPersist->ipAddress);
}
// Send to PING device functions
void TransportPing::Private::PingSend(TransportPing& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId, uint8_t* buffer)
{
    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        //Do Nothing. 
    }
}