#include "BasicCluster.h"
#include "EndpointApi.h"
#include <lib/support/ZclString.h>
#include <string.h>
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (2u)
using namespace ::chip;
using namespace ::chip::app::Clusters;

BasicCluster::BasicCluster(void)
{
    _id = BridgedDeviceBasicInformation::Id;
}

void BasicCluster::SetReachable(bool reachable, uint16_t index)
{
    _reachable = reachable;
    EndpointReportChange(index, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
}

void BasicCluster::SetName(const char * name, uint16_t index)
{
    _name[0] = '\0';
    strncat(_name, name, sizeof(_name)-1);
    EndpointReportChange(index, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
}

EmberAfStatus BasicCluster::Write(chip::AttributeId attributeId, uint8_t * buffer)
{
    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus BasicCluster::Read(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    using namespace BridgedDeviceBasicInformation::Attributes;
    // TODO: add debug
    //     ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%" PRIu32 ", maxReadLength=%u", attributeId,
    //                     maxReadLength);

    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    if (attributeId == Reachable::Id)
    {
        *buffer = _reachable ? 1 : 0;
    }
    else if (attributeId == NodeLabel::Id)
    {
        if (maxReadLength == 32)
        {
            MutableByteSpan zclNameSpan(buffer, maxReadLength);
            MakeZclCharString(zclNameSpan, _name); // TODO: get this from the info cluster
        }
        else
        {
            log_error("basic cluster NodeLabel size %u", maxReadLength);
        }
    }
    else if (attributeId == ClusterRevision::Id)
    {
        if (maxReadLength == 2)
        {
            uint16_t rev = ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION;
            memcpy(buffer, &rev, sizeof(rev));
        }
        else
        {
            log_error("basic cluster ClusterRevision size %u", maxReadLength);
        }
    }
    else
    {
        log_warn("our basic cluster doesn't support attribute %04X", attributeId);
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    return status;
}