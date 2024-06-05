/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DISPLAY_POWER_MGR_IPC_INTERFACE_DODE_H
#define DISPLAY_POWER_MGR_IPC_INTERFACE_DODE_H

/* SAID: 3308 */
namespace OHOS {
namespace PowerMgr {
enum class DisplayPowerMgrInterfaceCode {
    SET_DISPLAY_STATE = 0,
    GET_DISPLAY_STATE,
    GET_DISPLAY_IDS,
    GET_MAIN_DISPLAY_ID,
    SET_BRIGHTNESS,
    DISCOUNT_BRIGHTNESS,
    OVERRIDE_BRIGHTNESS,
    OVERRIDE_DISPLAY_OFF_DELAY,
    RESTORE_BRIGHTNESS,
    GET_BRIGHTNESS,
    GET_DEFAULT_BRIGHTNESS,
    GET_MAX_BRIGHTNESS,
    GET_MIN_BRIGHTNESS,
    ADJUST_BRIGHTNESS,
    AUTO_ADJUST_BRIGHTNESS,
    IS_AUTO_ADJUST_BRIGHTNESS,
    REGISTER_CALLBACK,
    BOOST_BRIGHTNESS,
    CANCEL_BOOST_BRIGHTNESS,
    GET_DEVICE_BRIGHTNESS,
    SET_COORDINATED,
    SET_APS_LIGHT_AND_BRIGHTNESS_THRESOLD = 21,
    SET_MAX_BRIGHTNESS,
    SET_MAX_BRIGHTNESS_NIT
};
} // space PowerMgr
} // namespace OHOS

#endif // DISPLAY_POWER_MGR_IPC_INTERFACE_DODE_H