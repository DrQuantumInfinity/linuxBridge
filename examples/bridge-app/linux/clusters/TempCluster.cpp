#include "TempCluster.h"
#include "EndpointApi.h"

#include <app/util/attribute-storage.h>
using namespace ::chip;
using namespace ::chip::app::Clusters;

void TempCluster::UpdateTemp(float temp, uint16_t index)
{
    _temp = (int16_t) temp * 100;
    EndpointReportChange(index, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);
}
Status TempCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    Status status = Status::Success;
    switch (attributeId)
    {
    case TemperatureMeasurement::Attributes::MeasuredValue::Id:
        memcpy(&_temp, buffer, sizeof(_temp));
        break;
    default:
        status = Status::Failure;
        break;
    }
    return status;
}

Status TempCluster::Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    Status status = Status::Success;
    if (attributeId == TemperatureMeasurement::Attributes::MeasuredValue::Id && maxReadLength == 2)
    {
        memcpy(buffer, &_temp, maxReadLength);
    }
    else
    {
        status = Status::Failure;
    }
    return status;
}