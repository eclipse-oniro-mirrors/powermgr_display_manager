/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "screen_action.h"

#include "display_common.h"
#include "display_type.h"
#include "hilog_wrapper.h"
#include "window_manager_service_client.h"

namespace OHOS {
namespace DisplayPowerMgr {
ScreenAction::ScreenAction()
{
    DeviceFuncs *f = NULL;

    int32_t ret = DeviceInitialize(&f);
    if (ret != DISPLAY_SUCCESS) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to init device");
        return;
    }
    devIds_.push_back(0);
    hdiFuncs_ = DeviceFuncPtr(f);
    DISPLAY_HILOGI(MODULE_SERVICE, "Succeed to init");
}

std::vector<uint32_t> ScreenAction::GetDisplayIds()
{
    return devIds_;
}

DisplayState ScreenAction::GetPowerState(uint32_t devId)
{
    DisplayState ret = DisplayState::DISPLAY_UNKNOWN;

    auto wmsc = WindowManagerServiceClient::GetInstance();
    wmsc->Init();
    sptr<IWindowManagerService> wms = wmsc->GetService();
    if (wms == nullptr) {
        DISPLAY_HILOGE(MODULE_SERVICE, "FAILED to get service from WindowManager Client");
        return DisplayState::DISPLAY_UNKNOWN;
    }
    // auto promise = wms->GetDisplayPower(devId)->Await();
    // if (promise.wret != WM_OK) {
    //     DISPLAY_HILOGE(MODULE_SERVICE, "GetPowerState failed: %{public}d", promise.wret);
    //     return ret;
    // }

    // switch (promise.status) {
    //     case POWER_STATUS_ON:
    //         ret = DisplayState::DISPLAY_ON;
    //         break;
    //     case POWER_STATUS_STANDBY:
    //         ret = DisplayState::DISPLAY_DIM;
    //         break;
    //     case POWER_STATUS_SUSPEND:
    //         ret = DisplayState::DISPLAY_SUSPEND;
    //         break;
    //     case POWER_STATUS_OFF:
    //         ret = DisplayState::DISPLAY_OFF;
    //         break;
    //     default:
    //         break;
    // }

    return ret;
}

bool ScreenAction::SetPowerState(uint32_t devId, DisplayState state)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "SetDisplayPower: devId=%{public}d, state=%{public}d",
        devId, static_cast<uint32_t>(state));
    auto wmsc = WindowManagerServiceClient::GetInstance();
    wmsc->Init();
    sptr<IWindowManagerService> wms = wmsc->GetService();
    if (wms == nullptr) {
        DISPLAY_HILOGE(MODULE_SERVICE, "FAILED to get service from WindowManager Client");
        return false;
    }

    DispPowerStatus status = POWER_STATUS_BUTT;
    switch (state) {
        case DisplayState::DISPLAY_ON:
            status = POWER_STATUS_ON;
            break;
        case DisplayState::DISPLAY_DIM:
            status = POWER_STATUS_STANDBY;
            break;
        case DisplayState::DISPLAY_SUSPEND:
            status = POWER_STATUS_SUSPEND;
            break;
        case DisplayState::DISPLAY_OFF:
            status = POWER_STATUS_OFF;
            break;
        default:
            break;
    }

    // auto wret = wms->SetDisplayPower(devId, status)->Await();
    // if (wret != WM_OK) {
    //     DISPLAY_HILOGE(MODULE_SERVICE, "SetDisplayPower failed: %{public}d", wret);
    //     return false;
    // }

    return true;
}

uint32_t ScreenAction::GetBrightness(uint32_t devId)
{
    auto wmsc = WindowManagerServiceClient::GetInstance();
    wmsc->Init();
    sptr<IWindowManagerService> wms = wmsc->GetService();
    if (wms == nullptr) {
        DISPLAY_HILOGE(MODULE_SERVICE, "FAILED to get service from WindowManager Client");
        return 0;
    }
    // auto promise = wms->GetDisplayBacklight(devId)->Await();
    // if (promise.wret != WM_OK) {
    //     DISPLAY_HILOGE(MODULE_SERVICE, "GetBrightness failed: %{public}d", promise.wret);
    //     return 0;
    // }

    // return promise.level;
    return 0;
}

bool ScreenAction::SetBrightness(uint32_t devId, uint32_t value)
{
    auto wmsc = WindowManagerServiceClient::GetInstance();
    wmsc->Init();
    sptr<IWindowManagerService> wms = wmsc->GetService();
    if (wms == nullptr) {
        DISPLAY_HILOGE(MODULE_SERVICE, "FAILED to get service from WindowManager Client");
        return false;
    }
    // auto wret = wms->SetDisplayBacklight(devId, value)->Await();
    // if (wret != WM_OK) {
    //     DISPLAY_HILOGE(MODULE_SERVICE, "SetBrightness failed: %{public}d", wret);
    //     return false;
    // }

    return true;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
