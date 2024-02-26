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

class MAPPED_DEVICE
{
public:
    Device* pDevice;
    TimerTick timer;
protected:

private:
};

typedef std::map<std::string, MAPPED_DEVICE*> MAPPING;
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
    Device* GetAndRemoveExpiredDevice(std::string key);
    Device* GetAndRemoveExpiredDevice(const uint8_t* pKey, uint32_t len);

protected:

private:
    MAPPING _map;
};