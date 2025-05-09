/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import brightness from "@ohos.brightness";

function main() {
    try {
        console.info('brightness setValue start');
        brightness.setValue(200);
        console.info('brightness setValue end');
        console.info('brightness setValue continuous start');
        brightness.setValue(30, true);
        console.info('brightness.setValue continuous end');
    } catch (err) {
        console.error("test brightness err: " + err)
    }
}