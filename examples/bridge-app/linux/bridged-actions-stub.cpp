/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    Licensed under the Apache License, Version 2.0
 */

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/actions-server/actions-server.h>
#include <app/util/attribute-storage.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using Status = chip::Protocols::InteractionModel::Status;

namespace {

class BridgeActionsDelegateImpl : public Actions::Delegate
{
public:
    CHIP_ERROR ReadActionAtIndex(uint16_t index, ActionStructStorage & action) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    CHIP_ERROR ReadEndpointListAtIndex(uint16_t index, EndpointListStorage & epList) override
    {
        return CHIP_ERROR_PROVIDER_LIST_EXHAUSTED;
    }
    bool HaveActionWithId(uint16_t actionId, uint16_t & actionIndex) override { return false; }

    Status HandleInstantAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleInstantActionWithTransition(uint16_t actionId, uint16_t transitionTime, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleStartAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleStartActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleStopAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandlePauseAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandlePauseActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleResumeAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleEnableAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleEnableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleDisableAction(uint16_t actionId, Optional<uint32_t> invokeId) override { return Status::NotFound; }
    Status HandleDisableActionWithDuration(uint16_t actionId, uint32_t duration, Optional<uint32_t> invokeId) override { return Status::NotFound; }
};

BridgeActionsDelegateImpl gActionsDelegateImpl;
std::unique_ptr<Actions::ActionsServer> sActionsServer;

} // anonymous namespace

void emberAfActionsClusterInitCallback(EndpointId endpoint)
{
    if (endpoint != 1)
        return;
    if (!emberAfContainsServer(endpoint, Actions::Id))
        return;
    if (sActionsServer)
        return;

    sActionsServer = std::make_unique<Actions::ActionsServer>(endpoint, gActionsDelegateImpl);
    TEMPORARY_RETURN_IGNORED sActionsServer->Init();
}
