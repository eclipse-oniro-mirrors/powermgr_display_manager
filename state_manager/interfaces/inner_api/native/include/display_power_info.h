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

#ifndef DISPLAYMGR_DISPLAY_POWER_INFO_H
#define DISPLAYMGR_DISPLAY_POWER_INFO_H

namespace OHOS {
namespace DisplayPowerMgr {
/**
 * Display State
 */
enum class DisplayState : uint32_t {
    DISPLAY_OFF = 0,
    DISPLAY_DIM = 1,
    DISPLAY_ON = 2,
    DISPLAY_SUSPEND = 3,
    DISPLAY_DELAY_OFF = 4,
    DISPLAY_UNKNOWN = 5,
};

enum class StateChangeReason : uint32_t {
    STATE_CHANGE_REASON_INIT = 0,
    STATE_CHANGE_REASON_TIMEOUT = 1,
    STATE_CHANGE_REASON_RUNNING_LOCK = 2,
    STATE_CHANGE_REASON_BATTERY = 3,
    STATE_CHANGE_REASON_THERMAL = 4,
    STATE_CHANGE_REASON_WORK = 5,
    STATE_CHANGE_REASON_SYSTEM = 6,
    STATE_CHANGE_REASON_APPLICATION = 10,
    STATE_CHANGE_REASON_SETTINGS = 11,
    STATE_CHANGE_REASON_HARD_KEY = 12,
    STATE_CHANGE_REASON_TOUCH = 13,
    STATE_CHANGE_REASON_CABLE = 14,
    STATE_CHANGE_REASON_SENSOR = 15,
    STATE_CHANGE_REASON_LID = 16,
    STATE_CHANGE_REASON_CAMERA = 17,
    STATE_CHANGE_REASON_ACCESSIBILITY = 18,
    STATE_CHANGE_REASON_RESET = 19,
    STATE_CHANGE_REASON_POWER_KEY = 20,
    STATE_CHANGE_REASON_KEYBOARD = 21,
    STATE_CHANGE_REASON_MOUSE = 22,
    STATE_CHANGE_REASON_DOUBLE_CLICK = 23,
    STATE_CHANGE_REASON_PRE_PROCESS = 24,
    STATE_CHANGE_REASON_CANCEL_PRE_PROCESS = 25,
    STATE_CHANGE_REASON_REMOTE = 100,
    STATE_CHANGE_REASON_UNKNOWN = 1000,
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_POWER_INFO_H
