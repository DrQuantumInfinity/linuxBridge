
#include "transportMqtt.h"

#include "Log.h"
//Devices
#include "DeviceLightLevel.h"
#include "DeviceButton.h"
#include "DeviceLightRGB.h"

#include "mqttWrapper.h"
#include "timer.h"
#include "string"

using namespace ::chip;
using namespace ::chip::app::Clusters;
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
typedef enum
{
    MQTT_FEIT_CMD_ONOFF,
    MQTT_FEIT_CMD_LEVEL
}MQTT_FEIT_CMD;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
struct TransportMqtt::Private
{
    static MQTT_TYPE GetDeviceType(const char* pTopic);
    static Device* AddNewDevice(MQTT_TYPE type, const char* pName);

    static void GoogleSend(MQTT_TYPE type, const char* pTopic, const char* pPayload, Device* pDevice);
    static void GoogleSendLightLevel(const char* pTopic, const char* pPayload, DeviceLightLevel* pDevice);
    static void GoogleSendOutlet(const char* pTopic, const char* pPayload, DeviceButton* pDevice);
    static void GoogleSendLightRgb(const char* pTopic, const char* pPayload, DeviceLightRGB* pDevice);
    
    static void MqttSend(TransportMqtt& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId);
    static void MqttSendLightLevel(TransportMqtt& self, const DeviceLightLevel* pDevice, ClusterId clusterId, AttributeId attributeId);
    static void MqttSendOutlet(TransportMqtt& self, const DeviceButton* pDevice, ClusterId clusterId, AttributeId attributeId);
    static void MqttSendLightRgb(TransportMqtt& self, const DeviceLightRGB* pDevice, ClusterId clusterId, AttributeId attributeId);
};

vector<string> split(const string& str, const string& delim);
/**************************************************************************
 *                                  Variables
 **************************************************************************/
static const char *pMqttDeviceTypes[] = {
    [MQTT_DIMMER_SWITCH_FEIT] = "DimmerFeit/",
    [MQTT_OUTLET_GORDON] = "OutletGordon/",
    [MQTT_LAMP_RGB] = "RgbLampGordon/"
};
static std::string MqttFeitCommands[2]      = { 
    [MQTT_FEIT_CMD_ONOFF] = "0",
    [MQTT_FEIT_CMD_LEVEL] = "1"
 };
// static const char *pMqttFeitCommands[] = {
//     [MQTT_FEIT_CMD_ONOFF] = "0/",
//     [MQTT_FEIT_CMD_LEVEL] = "1/"
// };
static uint32_t mqttDeviceTopicLengths[MQTT_TYPE_COUNT];
DeviceList TransportMqtt::_deviceList; //static variables in a class need to be independently initialized. C++ is dumb
mqtt_inst* TransportMqtt::_mqttInst; 
/**************************************************************************
 *                                  Static Functions
 **************************************************************************/
void TransportMqtt::Init(void)
{
    for (uint32_t type = 0; type < (uint32_t)MQTT_TYPE_COUNT; type++)
    {
        mqttDeviceTopicLengths[type] = strlen(pMqttDeviceTypes[type]) + 12;
    }

    TransportMqtt::_mqttInst = mqtt_wrap_init("192.168.0.128", TransportMqtt::HandleTopicRx);
    for(int i = 0; i<MQTT_TYPE_COUNT; i++)
    {
        char topicBuf[30];
        sprintf(topicBuf, "%s#", pMqttDeviceTypes[i]);
        mqtt_wrap_add_sub(TransportMqtt::_mqttInst, topicBuf);
    }
    mqtt_wrap_loopstart(TransportMqtt::_mqttInst);
}
void TransportMqtt::HandleTopicRx(const char* pTopic, const char* pPayload)
{
    log_info("HandleTopicRx. topic: %s. payload: %s", pTopic, pPayload);
    MQTT_TYPE type = Private::GetDeviceType(pTopic);
    if (type < MQTT_TYPE_COUNT)
    {
        char name[MQTT_MAX_DEVICE_NAME_LENGTH];
        memcpy(name, pTopic, mqttDeviceTopicLengths[type]);
        name[mqttDeviceTopicLengths[type]] = '\0';
        
        Device* pDevice = Private::AddNewDevice(type, name);
        if (pDevice)
        {
            _deviceList.Upsert(name, pDevice);
            Private::GoogleSend(type, pTopic, pPayload, pDevice);
        }
    }
    else
    {
        log_warn("Invalid MQTT topic: %s", pTopic);
        //TODO: log something about an invalid MQTT message
    }
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
TransportMqtt::TransportMqtt(MQTT_TYPE type, const char* pMacAddr)
{
    _type = type;
    memcpy(_macAddr, pMacAddr, sizeof(_macAddr));
    _macAddr[12] = '\0';
}
TransportMqtt::~TransportMqtt(void)
{
    
}
/**************************************************************************
 *                                  Protected Functions
 **************************************************************************/
void TransportMqtt::Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    Private::MqttSend(*this, pDevice, clusterId, attributeMetadata->attributeId);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
Device* TransportMqtt::Private::AddNewDevice(MQTT_TYPE type, const char* pName)
{
    Device* pDevice = _deviceList.GetDevice(pName);
    if (pDevice == NULL)
    {
        char room[10] = "Bridge";
        TransportLayer* pTransport = new TransportMqtt(type, &pName[strlen(pMqttDeviceTypes[type])]);
        switch(type)
        {
            case MQTT_DIMMER_SWITCH_FEIT:   pDevice = new DeviceLightLevel(pName, room, pTransport); break;
            case MQTT_OUTLET_GORDON:        pDevice = new DeviceButton(pName, room, pTransport);     break;
            case MQTT_LAMP_RGB:             pDevice = new DeviceLightRGB(pName, room, pTransport);   break;
            default:                        /*Support this type!*/                                  break;
        }
    }
    return pDevice;
}

MQTT_TYPE TransportMqtt::Private::GetDeviceType(const char* pTopic)
{
    //MQTT device identifiers must match the pattern "DEVICE-MACADDRESS/"
    uint32_t type = (uint32_t)MQTT_TYPE_COUNT;

    const char* pSlash = strchr(pTopic, '/');
    if (pSlash)
    {
        pSlash++;
        pSlash = strchr(pSlash, '/');
        if(pSlash)
        {
            // pSlash--;
            uint32_t deviceNameLength = (uint64_t)pSlash - (uint64_t)pTopic;
            type = 0;
            while (type < (uint32_t)MQTT_TYPE_COUNT)
            {
                if (deviceNameLength == mqttDeviceTopicLengths[type] &&
                    memcmp(pMqttDeviceTypes[type], pTopic, strlen(pMqttDeviceTypes[type])) == STR_CMP_MATCH)
                {
                    //Could also verify that the last 12 characters are ascii-hex...
                    break;
                }
                type++;
            }
        }
    }
    return (MQTT_TYPE)type;
}


//Send to Google functions
void TransportMqtt::Private::GoogleSend(MQTT_TYPE type, const char* pTopic, const char* pPayload, Device* pDevice)
{
    switch (type)
    {
        case MQTT_DIMMER_SWITCH_FEIT:   Private::GoogleSendLightLevel(pTopic, pPayload, (DeviceLightLevel*)pDevice);    break;
        case MQTT_OUTLET_GORDON:        Private::GoogleSendOutlet(pTopic, pPayload, (DeviceButton*)pDevice);            break;
        case MQTT_LAMP_RGB:             Private::GoogleSendLightRgb(pTopic, pPayload, (DeviceLightRGB*)pDevice);        break;
        default:                        /*Support this type!*/                                                          break;
    }
}
void TransportMqtt::Private::GoogleSendLightLevel(const char* pTopic, const char* pPayload, DeviceLightLevel* pDevice)
{
    log_info("topic: %s", pTopic);
    string topic (pTopic);
    vector<string> splitTopic = split(topic, '/');
    
    if(splitTopic[2].compare(MqttFeitCommands[MQTT_FEIT_CMD_ONOFF])==0){

    }
    else if(splitTopic[2].compare(MqttFeitCommands[MQTT_FEIT_CMD_LEVEL])==0){

    }

    const char* pSlash = strchr(pTopic, '/');
    if (pSlash)
    {
        log_info("mac: %s", pSlash);
        pSlash++;
        pSlash = strchr(pSlash, '/');
        if (pSlash)
        {  
            const char* commandCode = pSlash + 1;
            
            log_info("cmd: %s", pSlash);
            pSlash++;
            pSlash = strchr(pSlash, '/');
            if (pSlash)
            {
                log_info("set?: %s", pSlash);
                
            }
        }
    }
    // pDevice->setLevel();
    //  pDevice->SetOn(true/*or false...*/);
}
void TransportMqtt::Private::GoogleSendOutlet(const char* pTopic, const char* pPayload, DeviceButton* pDevice)
{
    /*Parse the topic for which value is changing*/
    pDevice->SetOn(true/*or false...*/);
}
void TransportMqtt::Private::GoogleSendLightRgb(const char* pTopic, const char* pPayload, DeviceLightRGB* pDevice)
{
    /*Parse the topic for which value is changing*/
    //pDevice->SetOn(pData->data.lightRgb.onOff);
    //pDevice->SetLevel(pData->data.lightRgb.brightness*100/255);
    //pDevice->SetColourHS(pData->data.lightRgb.hue*100/255, pData->data.lightRgb.saturation*100/255);
}


//Send to MQTT device functions
void TransportMqtt::Private::MqttSend(TransportMqtt& self, const Device* pDevice, ClusterId clusterId, AttributeId attributeId)
{
    log_info("MqttSend");
    switch (self._type)
    {
        case MQTT_DIMMER_SWITCH_FEIT:   Private::MqttSendLightLevel(self, (const DeviceLightLevel*)pDevice, clusterId, attributeId);    break;
        case MQTT_OUTLET_GORDON:        Private::MqttSendOutlet(self, (const DeviceButton*)pDevice, clusterId, attributeId);            break;
        case MQTT_LAMP_RGB:             Private::MqttSendLightRgb(self, (const DeviceLightRGB*)pDevice, clusterId, attributeId);        break;
        default:                        /*Support this type!*/                                                                              break;
    }
}
void TransportMqtt::Private::MqttSendLightLevel(TransportMqtt& self, const DeviceLightLevel* pDevice, ClusterId clusterId, AttributeId attributeId)
{
    char topic[30];
    char message[30];
    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        sprintf(topic, "%s%s/1/set", pMqttDeviceTypes[self._type], self._macAddr);
        sprintf(message, "%u", pDevice->onOffCluster._isOn ? 1 : 0);
    }
    else if (clusterId == LevelControl::Id && attributeId == LevelControl::Attributes::CurrentLevel::Id)
    {
        sprintf(topic, "%s%s/2/set", pMqttDeviceTypes[self._type], self._macAddr);
        sprintf(message, "%u", pDevice->levelControlCluster._level*10);
    }
    else{
        log_error("MqttSendLightLevel unsupported cluster/attribute %u/%u", clusterId, attributeId);
        return;
    }
    log_info("MqttSendLightLevel %s = %s", topic, message);
    mqtt_wrap_publish(TransportMqtt::_mqttInst, topic, message);
}
void TransportMqtt::Private::MqttSendOutlet(TransportMqtt& self, const DeviceButton* pDevice, ClusterId clusterId, AttributeId attributeId)
{

}
void TransportMqtt::Private::MqttSendLightRgb(TransportMqtt& self, const DeviceLightRGB* pDevice, ClusterId clusterId, AttributeId attributeId)
{
    //Read the cluster info and compose an MQTT update. 
    //Might need to look at the cluster/attribute data to determine what kind of update to post...

    /*
    self._data.data.lightRgb.onOff      = pDevice->onOffCluster._isOn;
    self._data.data.lightRgb.brightness = pDevice->levelControlCluster._level;
    self._data.data.lightRgb.hue        = pDevice->colourCluster._hue;
    self._data.data.lightRgb.saturation = pDevice->colourCluster._sat;
    self._data.data.lightRgb.mode       = ESP_NOW_DATA_LIGHT_RGB_MODE_STATIC;
    self._data.type                     = ESP_NOW_DEVICE_TYPE_LIGHT_RGB;

    SerialTransmit(&self._data, offsetof(ESP_NOW_DATA, data) + sizeof(ESP_NOW_DATA_LIGHT_RGB));
    */
}
/*
void TransportEspNow::DeviceLightRgb(const DeviceLightRGB* pDevice)
{    
    _data.data.lightRgb.onOff      = pDevice->onOffCluster._isOn;
    _data.data.lightRpDevicegb.brightness = pDevice->levelControlCluster._level;
    _data.data.lightRgb.hue        = pDevice->colourCluster._hue;
    _data.data.lightRgb.saturation = pDevice->colourCluster._sat;
    _data.data.lightRgb.mode       = ESP_NOW_DATA_LIGHT_RGB_MODE_STATIC;
    _data.type                     = ESP_NOW_DEVICE_TYPE_LIGHT_RGB;

    SerialTransmit(&_data, offsetof(ESP_NOW_DATA, data) + sizeof(ESP_NOW_DATA_LIGHT_RGB));
}
void DeviceLight::sendEspNowMessage()
{
    _espNowData.data.lightOnOff.onOff = onOffCluster._isOn;
    SerialTransmit(&_espNowData, sizeof(_espNowData));
}
void DeviceLightLevel::sendEspNowMessage()
{
    _espNowData.data.lightDimmer.onOff      = onOffCluster._isOn;
    _espNowData.data.lightDimmer.brightness = levelControlCluster._level;
    SerialTransmit(&_espNowData, sizeof(_espNowData));
}
void DeviceLightTemp::sendEspNowMessage()
{
    _espNowData.data.lightTempRgb.onOff = onOffCluster._isOn;
    _espNowData.data.lightTempRgb.brightness = levelControlCluster._level;
    _espNowData.data.lightTempRgb.hue = colourCluster._hue;
    _espNowData.data.lightTempRgb.saturation = colourCluster._sat;
    _espNowData.data.lightTempRgb.tempKelvin = 1000'000 / colourCluster._temp;
    SerialTransmit(&_espNowData, sizeof(_espNowData));
}
*/

vector<string> split(const string& str, const string& delim)
{
    vector<string> result;
    size_t start = 0;

    for (size_t found = str.find(delim); found != string::npos; found = str.find(delim, start))
    {
        result.emplace_back(str.begin() + start, str.begin() + found);
        start = found + delim.size();
    }
    if (start != str.size())
        result.emplace_back(str.begin() + start, str.end());
    return result;      
}