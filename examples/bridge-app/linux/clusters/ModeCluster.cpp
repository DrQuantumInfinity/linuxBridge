// #include "OnOffCluster.h"
// #include "EndpointApi.h"

// #include "esp_log.h"
// #include <lib/support/ZclString.h>
// #define ZCL_ON_OFF_CLUSTER_REVISION (4u)
// static const char * TAG = "OnOffCluster";

// using namespace chip::app::Clusters::ModeSelect;

// void OnOffCluster::SetOn(bool on, uint16_t index)
// {
//     _isOn = on;
//     EndpointReportChange(index, OnOff::Id, OnOff::Attributes::OnOff::Id);
// }

// Status OnOffCluster::Write(chip::AttributeId attributeId, uint8_t* buffer)
// {
//     log_info("OnOffCluster Write called");
    //     Status status = Status::Success;
    //     switch (attributeId)
    //     {
        //         case OnOff::Attributes::OnOff::Id:  _isOn = (bool)buffer[0];   break;
        //         default:                            status = Status::Failure;      break;
    //     }
//     return status;
// }

// Status OnOffCluster::Read(chip::AttributeId attributeId, uint8_t* buffer, uint16_t maxReadLength){

    //     log_info("OnOffCluster Read called");
    //     Status status = Status::Success;
    //     if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    //     {
        //         *buffer = _isOn ? 1 : 0;
//     }
//     else if ((attributeId == OnOff::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    //     {
    //         uint16_t rev = ZCL_ON_OFF_CLUSTER_REVISION;
    //         memcpy(buffer, &rev, sizeof(rev));
    //     }
    //     else
    //     {
//         status = Status::Failure;
    //     }
//     return status;
// }