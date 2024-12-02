
#include "transportEspNow.h"

//Devices
#include "DeviceButton.h"
#include "DeviceLightRGB.h"
#include "SerialFramerEspNow.h"

using namespace ::chip;

/**************************************************************************
 *                                  Constants
 **************************************************************************/
 /**************************************************************************
  *                                  Macros
  **************************************************************************/
  /**************************************************************************
   *                                  Types
   **************************************************************************/
typedef struct {
    char name[32];
    char room[ENDPOINT_LOCATION_LENGTH];
    uint8_t macAddr[6];
    ESP_NOW_DEVICE_TYPE type;
}PersistEspNow;

/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
struct TransportEspNow::Private
{
    static void NewDeviceOnPwr(int index, void* pPersist);
    static Device* NewDevice(int index, PersistEspNow* pPersist);

    static void GoogleSend(const ESP_NOW_DATA* pData, Device* pDevice);
    static void GoogleSendButton(const ESP_NOW_DATA* pData, DeviceButton* pDevice);
    static void GoogleSendLightRgb(const ESP_NOW_DATA* pData, DeviceLightRGB* pDevice);

    static void EspNowSend(TransportEspNow& self, const Device* pDevice);
    static void EspNowSendLightRgb(TransportEspNow& self, const DeviceLightRGB* pDevice);
};
/**************************************************************************
 *                                  Variables
 **************************************************************************/
DeviceList TransportEspNow::_deviceList; //static variables in a class need to be independently initialized. C++ is dumb
PersistDevList TransportEspNow::_persistList = PersistDevList(sizeof(PersistEspNow), "espnowPersist.bin");
/**************************************************************************
 *                                  Static Functions
 **************************************************************************/

void TransportEspNow::Init(void)
{
    _persistList.Apply(TransportEspNow::Private::NewDeviceOnPwr);
}
void TransportEspNow::HandleSerialRx(const ESP_NOW_DATA* pData, uint32_t dataLength)
{
    char name[32];
    sprintf(name, "%s %02X:%02X:%02X:%02X:%02X:%02X", EspNowGetName(pData),
        pData->macAddr[0], pData->macAddr[1], pData->macAddr[2],
        pData->macAddr[3], pData->macAddr[4], pData->macAddr[5]);
    char room[10] = "Bridge";

    Device* pDevice = _deviceList.GetDevice(pData->macAddr, sizeof(pData->macAddr));

    if (pDevice == NULL)
    {
        PersistEspNow persistData;
        strncpy(persistData.name, name, sizeof(persistData.name));
        strncpy(persistData.room, "Bridge", sizeof(persistData.room));
        persistData.type = pData->type;
        pDevice = TransportEspNow::Private::NewDevice(-1, &persistData);
        _persistList.Upsert(pDevice->GetIndex(), &persistData);
    }

    // Device* pDevice = Private::AddNewDevice(pData, dataLength);
    if (pDevice)
    {
        _deviceList.Upsert(pData->macAddr, sizeof(pData->macAddr), pDevice);
        Private::GoogleSend(pData, pDevice);
    }
}
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
TransportEspNow::TransportEspNow(ESP_NOW_DEVICE_TYPE type, const uint8_t* pMacAddr)
{
    _data.type = type;
    memcpy(&_data.macAddr, pMacAddr, sizeof(_data.macAddr));
}
TransportEspNow::~TransportEspNow(void)
{

}
/**************************************************************************
 *                                  Protected Functions
 **************************************************************************/
void TransportEspNow::Send(const Device* pDevice, ClusterId clusterId, const EmberAfAttributeMetadata* attributeMetadata, uint8_t* buffer)
{
    Private::EspNowSend(*this, pDevice);
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/

void TransportEspNow::Private::NewDeviceOnPwr(int index, void* pPersist)
{
    NewDevice(index, (PersistEspNow*)pPersist);
}

Device* TransportEspNow::Private::NewDevice(int index, PersistEspNow* pPersist)
{
    Device* pDevice;
    TransportLayer* pTransport = new TransportEspNow(pPersist->type, pPersist->macAddr);
    switch (pPersist->type)
    {
        case ESP_NOW_DEVICE_TYPE_TOGGLE:    pDevice = new DeviceButton(pPersist->name, pPersist->room, pTransport);     break;
        case ESP_NOW_DEVICE_TYPE_LIGHT_RGB: pDevice = new DeviceLightRGB(pPersist->name, pPersist->room, pTransport);   break;
        default:                            /*Support this type!*/                                  break;
    }
    return pDevice;
}

//Send to Google functions
void TransportEspNow::Private::GoogleSend(const ESP_NOW_DATA* pData, Device* pDevice)
{
    switch (pData->type)
    {
        case ESP_NOW_DEVICE_TYPE_TOGGLE:    Private::GoogleSendButton(pData, (DeviceButton*)pDevice);       break;
        case ESP_NOW_DEVICE_TYPE_LIGHT_RGB: Private::GoogleSendLightRgb(pData, (DeviceLightRGB*)pDevice);   break;
        default:                            /*Support this type!*/                                          break;
    }
}
void TransportEspNow::Private::GoogleSendButton(const ESP_NOW_DATA* pData, DeviceButton* pDevice)
{
    pDevice->Toggle();
}
void TransportEspNow::Private::GoogleSendLightRgb(const ESP_NOW_DATA* pData, DeviceLightRGB* pDevice)
{
    /*
    bool onOff;
    ESP_NOW_DATA_LIGHT_RGB_MODE mode;
    uint8_t hue;
    uint8_t saturation;
    uint8_t brightness;
    */
    pDevice->SetOn(pData->data.lightRgb.onOff);
    pDevice->SetLevel(pData->data.lightRgb.brightness * 100 / 255);
    pDevice->SetColourHS(pData->data.lightRgb.hue * 360 / 255, pData->data.lightRgb.saturation * 100 / 255);
}


//Send to EspNow device functions
void TransportEspNow::Private::EspNowSend(TransportEspNow& self, const Device* pDevice)
{
    switch (self._data.type)
    {
        case ESP_NOW_DEVICE_TYPE_TOGGLE:    /*ESPNOW TOGGLEs are transmitters only*/                            break;
        case ESP_NOW_DEVICE_TYPE_LIGHT_RGB: /* Private::EspNowSendLightRgb(self, (const DeviceLightRGB*)pDevice); */  break;
        default:                            /*Support this type!*/                                              break;
    }
}
void TransportEspNow::Private::EspNowSendLightRgb(TransportEspNow& self, const DeviceLightRGB* pDevice)
{
    self._data.data.lightRgb.onOff = pDevice->onOffCluster._isOn;
    self._data.data.lightRgb.brightness = pDevice->levelControlCluster._level;
    self._data.data.lightRgb.hue = pDevice->colourCluster._hue;
    self._data.data.lightRgb.saturation = pDevice->colourCluster._sat;
    self._data.data.lightRgb.mode = ESP_NOW_DATA_LIGHT_RGB_MODE_STATIC;
    self._data.type = ESP_NOW_DEVICE_TYPE_LIGHT_RGB;

    SerialTransmit(&self._data, offsetof(ESP_NOW_DATA, data) + sizeof(ESP_NOW_DATA_LIGHT_RGB));
}
/*
void TransportEspNow::DeviceLightRgb(const DeviceLightRGB* pDevice)
{
    _data.data.lightRgb.onOff      = pDevice->onOffCluster._isOn;
    _data.data.lightRgb.brightness = pDevice->levelControlCluster._level;
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