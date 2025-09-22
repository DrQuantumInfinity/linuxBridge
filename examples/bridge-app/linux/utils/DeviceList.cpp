
#include "DeviceList.h"


/**************************************************************************
 *                                  Constants
 **************************************************************************/
#define REMOVE_TIMER_1_DAY_MS       (24*60*60*1000)
/**************************************************************************
 *                                  Macros
 **************************************************************************/
#define STR(uint8Ptr, len) (std::string (reinterpret_cast<char const*>(uint8Ptr), len))
/**************************************************************************
 *                                  Types
 **************************************************************************/
/**************************************************************************
 *                                  Prototypes
 **************************************************************************/
/**************************************************************************
 *                                  Variables
 **************************************************************************/
/**************************************************************************
 *                                  Global Functions
 **************************************************************************/
DeviceList::DeviceList(void)
{
    _map.clear();
}
void DeviceList::Upsert(std::string key, Device* pDevice)
{
    std::pair<MAPPING::iterator, bool> mapRet;

    mapRet = _map.insert(make_pair(key, new MappedDevice(pDevice, REMOVE_TIMER_1_DAY_MS)));
    if (mapRet.second == false)
    {
        //mapRet.first->second.pDevice = pDevice; //Already assigned
        mapRet.first->second->_timer.SetFromNow(REMOVE_TIMER_1_DAY_MS);
    }
}
void DeviceList::Upsert(const uint8_t* pKey, uint32_t len, Device* pDevice)
{
    Upsert(STR(pKey, len), pDevice);
}
void DeviceList::Remove(std::string key)
{
    _map.erase(key);
}
void DeviceList::Remove(const uint8_t* pKey, uint32_t len)
{
    Remove(STR(pKey, len));
}
Device* DeviceList::GetDevice(std::string key)
{
    MAPPING::iterator it = _map.find(key);
    if (it != _map.end())
    {
        return it->second->_pDevice;
    }
    return NULL;
}
Device* DeviceList::GetDevice(const uint8_t* pKey, uint32_t len)
{
    return GetDevice(STR(pKey, len));
}
Device* DeviceList::GetFirstDevice(void)
{
    _currentDeviceIterator = _map.begin();
    return _currentDeviceIterator->second->_pDevice;
}
Device* DeviceList::GetNextDevice(void)
{
    MAPPING::iterator nextDeviceIterator = _currentDeviceIterator;
    nextDeviceIterator++;
    if (nextDeviceIterator != _map.end())
    {
        _currentDeviceIterator++;
        return _currentDeviceIterator->second->_pDevice;
    }
    else 
    {
        return NULL;
    }
}
Device* DeviceList::GetAndRemoveExpiredDevice(std::string key)
{
    for (MAPPING::iterator it = _map.begin(); it != _map.end(); it++)
    {
        if (it->second->_timer.HasElapsed())
        {
            Remove(key);
            return it->second->_pDevice;
        }
    }
    return NULL;
}
Device* DeviceList::GetAndRemoveExpiredDevice(const uint8_t* pKey, uint32_t len)
{
    return GetAndRemoveExpiredDevice(STR(pKey, len));
}
/**************************************************************************
 *                                  Private Functions
 **************************************************************************/
