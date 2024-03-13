#include "OnOffCluster.h"
#include "EndpointApi.h"

#include <lib/support/ZclString.h>
#define ZCL_ON_OFF_CLUSTER_REVISION (4u)
using namespace ::chip;
using namespace ::chip::app::Clusters;

void OnOffCluster::SetOn(bool on, uint16_t index)
{
    if (_isOn != on)
    {
        _isOn = on;
        EndpointReportChange(index, OnOff::Id, OnOff::Attributes::OnOff::Id);
    }
}

EmberAfStatus OnOffCluster::Write(chip::AttributeId attributeId, uint8_t* buffer)
{
    log_info("OnOffCluster Write called");
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    switch (attributeId)
    {
        case OnOff::Attributes::OnOff::Id:  _isOn = (bool)buffer[0];   break;
        default:                            status = EMBER_ZCL_STATUS_FAILURE;      break;
    }
    return status;
}

EmberAfStatus OnOffCluster::Read(chip::AttributeId attributeId, uint8_t* buffer, uint16_t maxReadLength){

    log_info("OnOffCluster Read called");
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        *buffer = _isOn ? 1 : 0;
    }
    else if ((attributeId == OnOff::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_ON_OFF_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }
    return status;
}