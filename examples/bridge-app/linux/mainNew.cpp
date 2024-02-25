/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "DeviceCallbacks.h"
#include "SerialTask.h"
#include "MatterTask.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "EndpointApi.h"
#include <app/server/OnboardingCodesUtil.h>
#include <common/Esp32AppServer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/ESP32/ESP32Utils.h>

#include <app/server/Server.h>

#include "StaticModeManager.h"


#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
#include <platform/ESP32/ESP32DeviceInfoProvider.h>
#else
#include <DeviceInfoProviderImpl.h>
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER

namespace {
#if CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
chip::DeviceLayer::ESP32DeviceInfoProvider gExampleDeviceInfoProvider;
#else
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif // CONFIG_ENABLE_ESP32_DEVICE_INFO_PROVIDER
} // namespace

static const char * TAG = "app_main";

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::Credentials;

static AppDeviceCallbacks AppCallback;

static void InitServer(intptr_t context);
static void StartThreads(void);

extern "C" void app_main()
{
    // Initialize the ESP NVS layer.
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK)
    {
        log_error("nvs_flash_init() failed: %s", esp_err_to_name(err));
        return;
    }
    err = esp_event_loop_create_default();
    if (err != ESP_OK)
    {
        log_error("esp_event_loop_create_default()  failed: %s", esp_err_to_name(err));
        return;
    }

    CHIP_ERROR chip_err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    if (DeviceLayer::Internal::ESP32Utils::InitWiFiStack() != CHIP_NO_ERROR)
    {
        log_error("Failed to initialize the Wi-Fi stack");
        return;
    }
#endif

    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    // app::Clusters::ModeSelect::StaticSupportedModesManager manager = app::Clusters::ModeSelect::StaticSupportedModesManager::getStaticSupportedModesManagerInstance();

    // if (err != ESP_OK)
    // {
    //     log_error("Failed to initialize endpoint array for supported-modes, err:%" CHIP_ERROR_FORMAT, err.Format());
    // }

    chip_err = deviceMgr.Init(&AppCallback);
    if (chip_err != CHIP_NO_ERROR)
    {
        log_error("device.Init() failed: %" CHIP_ERROR_FORMAT, chip_err.Format());
        return;
    }

#if CONFIG_ENABLE_ESP32_DEVICE_INSTANCE_INFO_PROVIDER
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
#endif
    // SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    // chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    StartThreads();
}
static void InitServer(intptr_t context)
{
    // PrintOnboardingCodes(chip::RendezvousInformationFlags(CONFIG_RENDEZVOUS_MODE));
    // Esp32AppServer::Init(); // Init ZCL Data Model and CHIP App Server AND Initialize device attestation config
}
static void StartThreads(void)
{
    //Initialize utilities before starting threads
    EndpointApiInit();

    CHIP_ERROR chip_err;
    chip_err = SerialTaskStart();
    if (chip_err != CHIP_NO_ERROR)
    {
        log_error("SerialTaskStart() failed : %" CHIP_ERROR_FORMAT, chip_err.Format());
    }
    chip_err = MatterTaskStart();
    if (chip_err != CHIP_NO_ERROR)
    {
        log_error("MatterTaskStart() failed : %" CHIP_ERROR_FORMAT, chip_err.Format());
    }
}