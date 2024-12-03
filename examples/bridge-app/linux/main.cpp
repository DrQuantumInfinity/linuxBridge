/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <AppMain.h>

#include "Device.h"
#include "main.h" 

#include <vector>

#include "Log.h"
#include "uart.h"
#include "SerialFramerEspNow.h"
#include "EndpointApi.h"
#include "transportEspNow.h"
#include "transportMqtt.h"
#include "timer.h"


std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId)
{
    std::vector<EndpointListInfo> infoList;
    return infoList;
}

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId)
{
    std::vector<Action *> actionsDummy;
    return actionsDummy;
}

void ApplicationInit()
{
    log_set_level(1);
    // chip::Server::GetInstance().ScheduleFactoryReset();
    EndpointApiInit();
    TransportEspNow::Init();
/*    gpioInitialise();
    gpioSetMode(2, PI_OUTPUT);
    gpioSetMode(3, PI_OUTPUT);
    gpioWrite(3, 1);
    gpioWrite(2, 0);
    TimerSleepMs(10);
    gpioWrite(2, 1);*/

    SerialInit();
    TransportMqtt::Init();
    TransportMqtt::HandleTopicRx("WifiDimmerFeit-112233445566/1/set", "1");
}

void ApplicationShutdown() {}

void UartRxCallback(UART_HANDLE uartHandle, void* pData, uint32_t dataLen)
{
    char* pMsg = (char*)pData;
    log_warn("uart got ---- %s", pMsg);
}

int main(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();
    return 0;
}
