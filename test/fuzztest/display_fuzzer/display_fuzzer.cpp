/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "display_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>

#include "display_power_mgr_client.h"
#define private   public
#define protected public
#include "display_power_mgr_service.h"
#undef private
#undef protected
#include "message_parcel.h"
#include "new"
#include "securec.h"

using namespace OHOS::PowerMgr;
using namespace OHOS::DisplayPowerMgr;
using namespace std;
using namespace OHOS;

namespace {
auto& g_displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();
const int32_t REWIND_READ_DATA = 0;
shared_ptr<DisplayPowerMgrService> g_fuzzService = nullptr;
} // namespace

static uint32_t GetUint32(const uint8_t* data, size_t size)
{
    uint32_t value = 0;
    if (size < sizeof(value)) {
        return value;
    }
    if (memcpy_s(&value, sizeof(value), data, sizeof(value)) != EOK) {
        return value;
    }
    return value;
}

static void SetDisplayState(const uint8_t* data, size_t size)
{
    uint32_t type = GetUint32(data, size);
    g_displayMgrClient.SetDisplayState(static_cast<OHOS::DisplayPowerMgr::DisplayState>(type), StateChangeReason(type));
}

static void GetDisplayState(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.GetDisplayState(value);
}

static void GetDisplayIds([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_displayMgrClient.GetDisplayIds();
}

static void GetMainDisplayId([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_displayMgrClient.GetMainDisplayId();
}

static void SetBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.SetBrightness(value, value);
}

static void AdjustBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.AdjustBrightness(value, value, value);
}

static void AutoAdjustBrightness([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
{
    g_displayMgrClient.AutoAdjustBrightness(true);
    g_displayMgrClient.AutoAdjustBrightness(false);
}

static void RegisterCallback(const uint8_t* data, size_t size)
{
    OHOS::sptr<OHOS::DisplayPowerMgr::IDisplayPowerCallback> callback;
    if (size < sizeof(callback)) {
        g_displayMgrClient.RegisterCallback(nullptr);
        return;
    }
    if ((memcpy_s(callback, sizeof(callback), data, sizeof(callback))) != EOK) {
        g_displayMgrClient.RegisterCallback(nullptr);
        return;
    }
    g_displayMgrClient.RegisterCallback(callback);
}

static void OverrideBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.OverrideBrightness(value, value);
}

static void RestoreBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.RestoreBrightness(value);
}

static void BoostBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.BoostBrightness(value, value);
}

static void CancelBoostBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.CancelBoostBrightness(value);
}

static void DiscountBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.DiscountBrightness(value, value);
}

static void GetBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.GetBrightness(value);
}

static void GetDeviceBrightness(const uint8_t* data, size_t size)
{
    uint32_t value = GetUint32(data, size);
    g_displayMgrClient.GetDeviceBrightness(value);
}

static void DisplayServiceStub(const uint8_t* data, size_t size)
{
    uint32_t code;
    if (size < sizeof(code)) {
        return;
    }
    if (memcpy_s(&code, sizeof(code), data, sizeof(code)) != EOK) {
        return;
    }

    MessageParcel datas;
    datas.WriteInterfaceToken(DisplayPowerMgrService::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(REWIND_READ_DATA);
    MessageParcel reply;
    MessageOption option;
    if (g_fuzzService == nullptr) {
        g_fuzzService = make_shared<DisplayPowerMgrService>();
        g_fuzzService->Init();
    }
    g_fuzzService->OnRemoteRequest(code, datas, reply, option);
}

static std::vector<std::function<void(const uint8_t*, size_t)>> fuzzFunc = {
    &SetDisplayState,
    &GetDisplayState,
    &GetDisplayIds,
    &GetMainDisplayId,
    &SetBrightness,
    &AdjustBrightness,
    &AutoAdjustBrightness,
    &RegisterCallback,
    &OverrideBrightness,
    &RestoreBrightness,
    &BoostBrightness,
    &CancelBoostBrightness,
    &DiscountBrightness,
    &GetBrightness,
    &GetDeviceBrightness
};

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(0, fuzzFunc.size() - 1);
    int32_t number = randomNum(engine);
    fuzzFunc[number](data, size);
    DisplayServiceStub(data, size);
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
