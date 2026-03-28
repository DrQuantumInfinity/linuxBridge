/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    Licensed under the Apache License, Version 2.0
 */

// Stub implementations for cluster server callbacks used by dynamic endpoints.
// The bridge uses external attribute storage, so these init/shutdown callbacks
// are called when dynamic endpoints are registered but don't need to do anything
// beyond what the SDK's default implementations provide.

#include <stdint.h>

using EndpointId = uint16_t;

void emberAfColorControlClusterServerInitCallback(EndpointId endpoint) {}
void MatterColorControlClusterServerShutdownCallback(EndpointId endpoint) {}
