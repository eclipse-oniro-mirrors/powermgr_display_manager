/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POWERMGR_BRIGHTNESS_H
#define POWERMGR_BRIGHTNESS_H

#include <map>
#include <string>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <cstddef>
#include <functional>
#include "errors.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "running_lock.h"

namespace OHOS {
namespace DisplayPowerMgr {
class Brightness {
public:
    explicit Brightness(napi_env env, std::shared_ptr<PowerMgr::RunningLock> runningLock = nullptr);
    void GetValue();
    void SetValue(napi_value& number);
    void SystemSetValue();
    void GetMode();
    void SetMode();
    void SetKeepScreenOn();
    napi_value GetCallbackInfo(napi_callback_info& info, napi_valuetype checkType);
    bool CreateCallbackRef(napi_value& options);
    void CreateValueRef(napi_value& options, const std::string& valName, napi_valuetype checkType);

    napi_async_work asyncWork = nullptr;

    static const std::string BRIGHTNESS_VALUE;
    static const std::string BRIGHTNESS_MODE;
    static const std::string KEEP_SCREENON;
private:
    class Result {
    public:
        void Error(int32_t code, const std::string& msg);
        void GetError(napi_env env, napi_value* error, size_t& size);
        napi_value GetResult(napi_env env);
        inline void SetResult(const std::string& key, int32_t value)
        {
            mapResult_.emplace(key, value);
        }
        inline bool IsError()
        {
            return !msg_.empty() && (code_ != ERR_OK);
        }
    private:
        int32_t code_ { ERR_OK };
        std::string msg_;
        std::map<std::string, int32_t> mapResult_;
    };

    class BrightnessInfo {
    public:
        uint32_t GetBrightness();
        bool SetBrightness(int32_t value);
        int32_t GetAutoMode();
        bool SetAutoMode(bool mode);
        void ScreenOn(bool keep, std::shared_ptr<PowerMgr::RunningLock>& runningLock);
    };

    void ExecuteCallback();
    bool CheckValueType(napi_value& value, napi_valuetype checkType);
    napi_value GetOptions(napi_value& options, const std::string& name, napi_valuetype checkType);
    void CallFunction(napi_ref& callbackRef, size_t argc, napi_value* response);

    napi_env env_;
    Result result_;
    BrightnessInfo brightnessInfo_;

    napi_ref successRef_ = nullptr;
    napi_ref failRef_ = nullptr;
    napi_ref completeRef_ = nullptr;
    napi_ref napiValRef_ = nullptr;
    std::shared_ptr<PowerMgr::RunningLock> runningLock_ = nullptr;
};
} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // POWERMGR_BRIGHTNESS_H
