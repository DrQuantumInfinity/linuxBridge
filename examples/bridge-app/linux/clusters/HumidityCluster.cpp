#include "HumidityCluster.h"
#include "EndpointApi.h"

#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

void HumidityCluster::UpdateHumidity(float humidity, uint16_t index)
{
    _humidity = (uint16_t) (humidity * 100);
    EndpointReportChange(index, RelativeHumidityMeasurement::Id, RelativeHumidityMeasurement::Attributes::MeasuredValue::Id);
}
EmberAfStatus HumidityCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
    case RelativeHumidityMeasurement::Attributes::MeasuredValue::Id:
        memcpy(&_humidity, buffer, sizeof(_humidity));
        break;
    default:
        status = EMBER_ZCL_STATUS_FAILURE;
        break;
    }
    return status;
}

EmberAfStatus HumidityCluster::Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    if (attributeId == RelativeHumidityMeasurement::Attributes::MeasuredValue::Id && maxReadLength == 2)
    {
        memcpy(buffer, &_humidity, maxReadLength);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }
    return status;
}