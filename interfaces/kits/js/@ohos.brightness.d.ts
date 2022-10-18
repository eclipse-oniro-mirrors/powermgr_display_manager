/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

import {BusinessError} from './basic';

/**
 * Provides interfaces to control the power of display.
 *
 * @syscap SystemCapability.PowerManager.DisplayPowerManager
 * @systemapi
 * @since 7
 */
declare namespace brightness {
    /**
     * Sets the screen brightness.
     *
     * @param {number} value Brightness value, ranging from 0 to 255.
     * @throws {BusinessError} 201 - If the permission is denied.
     * @throws {BusinessError} 401 - If the value is not valid.
     * @throws {BusinessError} 4700101 - If connecting to the service failed.
     * @systemapi
     * @since 7
     */
    function setValue(value: number): void;
}
export default brightness;