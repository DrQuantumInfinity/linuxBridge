#pragma once
#include <stdint.h>
#include <map>
#include <vector>
#include <string>
#include "TimerTick.h"

/**************************************************************************
 *                                  Constants
 **************************************************************************/
/**************************************************************************
 *                                  Macros
 **************************************************************************/
/**************************************************************************
 *                                  Types
 **************************************************************************/
class Device; //Forward declare (awful shit. Should just be able to #include Device.h but things will break)

class MappedDevice
{
public:
    Device* _pDevice;
    TimerTick _timer;

    MappedDevice(Device* pDevice, uint32_t msFromNow)
    {
        _pDevice = pDevice;
        _timer.SetFromNow(msFromNow);
    }

protected:

private:
};

typedef std::map<std::string, MappedDevice*> MAPPING;
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/

class DeviceList
{
public:
    DeviceList(void);

    void Upsert(std::string key, Device* pDevice);
    void Upsert(const uint8_t* pKey, uint32_t len, Device* pDevice);
    void Remove(std::string key);
    void Remove(const uint8_t* pKey, uint32_t len);
    Device* GetDevice(std::string key);
    Device* GetDevice(const uint8_t* pKey, uint32_t len);
    Device* GetFirstDevice(void);
    Device* GetNextDevice(void);
    Device* GetAndRemoveExpiredDevice(std::string key);
    Device* GetAndRemoveExpiredDevice(const uint8_t* pKey, uint32_t len);

protected:

private:
    MAPPING _map;
    MAPPING::iterator _currentDeviceIterator;
};