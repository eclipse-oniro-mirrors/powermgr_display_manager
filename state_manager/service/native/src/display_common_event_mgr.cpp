/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "display_common_event_mgr.h"

#include <string>

#include "display_system_ability.h"
#include "display_power_mgr_service.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"


namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr const char *SETTING_URI_PROXY =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char *SETTINGS_DATA_EXT_URI = "datashare_ext";
}
using CommonEventSubscriber = OHOS::EventFwk::CommonEventSubscriber;
using CommonEventSubscribeInfo = OHOS::EventFwk::CommonEventSubscribeInfo;
using CommonEventData = OHOS::EventFwk::CommonEventData;

DisplayCommonEventManager::DisplayCommonEventManager(const CommonEventSubscribeInfo &subscribeInfo)
    : CommonEventSubscriber(subscribeInfo)
{
}

void DisplayCommonEventManager::OnReceiveEvent(const CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action == "usual.event.DATA_SHARE_READY") {
        DISPLAY_HILOGI(COMP_SVC, "on receive data_share ready.");
        if (SetKvDataReady()) {
            RegisterSettingObservers();
        }
    }
}

bool DisplayCommonEventManager::CheckIfSettingsDataReady()
{
    if (isDataShareReady_) {
        return true;
    }
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DISPLAY_HILOGE(COMP_SVC, "sa manager is null");
        return false;
    }
    sptr<IRemoteObject> remoteObj = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    if (remoteObj == nullptr) {
        DISPLAY_HILOGE(COMP_SVC, "remote obj is null");
        return false;
    }
    return CreateDataShareHelper(remoteObj);
}

bool DisplayCommonEventManager::CreateDataShareHelper(sptr<IRemoteObject> remoteObj)
{
    std::pair<int, std::shared_ptr<DataShare::DataShareHelper>> ret =
        DataShare::DataShareHelper::Create(remoteObj, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    DISPLAY_HILOGI(COMP_SVC, "create data_share helper, ret=%{public}d", ret.first);
    if (ret.first == DataShare::E_OK) {
        DISPLAY_HILOGI(COMP_SVC, "create data_share helper success");
        auto helper = ret.second;
        if (helper != nullptr) {
            bool releaseRet = helper->Release();
            DISPLAY_HILOGI(COMP_SVC, "release data_share helper, releaseRet=%{public}d", releaseRet);
        }
        isDataShareReady_ = true;
        return true;
    } else {
        DISPLAY_HILOGE(COMP_SVC, "create data_share helper failed");
        isDataShareReady_ = false;
        return false;
    }
}

bool DisplayCommonEventManager::SetKvDataReady()
{
    std::lock_guard<std::mutex> lock(lock_);
    if (isKvDataReady_) {
        return false;
    }
    isKvDataReady_ = true;
    return true;
}

void DisplayCommonEventManager::RegisterSettingObservers()
{
    DISPLAY_HILOGI(COMP_SVC, "Register setting observer");
    auto service = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    service->RegisterSettingObservers();
}
} // namespace DisplayPowerMgr
} // namespace OHOS
