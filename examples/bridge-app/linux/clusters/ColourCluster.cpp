#include "ColourCluster.h"
#include "EndpointApi.h"

#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

// namespace ColorTemperatureMireds {
// static constexpr AttributeId Id = 0x00000007;
// } // namespace ColorTemperatureMireds

// namespace TemperatureMeasurement {
// static constexpr ClusterId Id = 0x00000402;
// } // namespace TemperatureMeasurement

// namespace CurrentHue {
// static constexpr AttributeId Id = 0x00000000;
// } // namespace CurrentHue

// namespace CurrentSaturation {

#define ZCL_COLOR_CLUSTER_REVISION (6u)
void ColourCluster::SetColourHS(uint8_t hue, uint8_t sat, uint16_t index)
{
    _hue = hue;
    _sat = sat;
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::CurrentHue::Id);
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::CurrentSaturation::Id);
}

void ColourCluster::SetColourTemp(uint16_t temp, uint16_t index)
{
    _temp = temp;
    EndpointReportChange(index, ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id);
}

EmberAfStatus ColourCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case ColorControl::Attributes::CurrentHue::Id:
        _hue = (uint8_t) buffer[0];
        break;
    case ColorControl::Attributes::CurrentSaturation::Id:
        _sat = (uint8_t) buffer[0];
        break;
    case ColorControl::Attributes::ColorTemperatureMireds::Id:
        memcpy(&_temp, buffer, sizeof(_temp));
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}

EmberAfStatus ColourCluster::Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    uint16_t rev         = ZCL_COLOR_CLUSTER_REVISION;
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case ColorControl::Attributes::ClusterRevision::Id:
        memcpy(buffer, &rev, sizeof(rev));
        break;
    case ColorControl::Attributes::CurrentHue::Id:
        buffer[0] = _hue;
        break;
    case ColorControl::Attributes::CurrentSaturation::Id:
        buffer[0] = _sat;
        break;
        //TODO set correctly
    case ColorControl::Attributes::FeatureMap::Id:
        buffer[0] = (1 << 0) | (1 << 1);
        break;
    case ColorControl::Attributes::ColorTemperatureMireds::Id:
        if (maxReadLength == 2)
        {
            memcpy(buffer, &_temp, maxReadLength);
        }
        else
        {
            status = EMBER_ZCL_STATUS_FAILURE;
        }
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}