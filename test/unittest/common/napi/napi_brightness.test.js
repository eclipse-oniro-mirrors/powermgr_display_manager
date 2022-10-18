/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

import brightness from '@ohos.brightness';
import { describe, it, expect } from 'deccjsunit/index';

describe('PowerMgrDisplayUnitTest', function () {
    /**
     * @tc.number PowerMgrDisplayUnitTest001
     * @tc.name set_value_success
     * @tc.desc Set brightness success
     */
    it('set_value_success', 0, function () {
        try {
            brightness.setValue(100);
        } catch (error) {
            console.info('set_value_success:' + error);
            expect().assertFail();
        }
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest002
     * @tc.name set_value_success_invalid
     * @tc.desc Set brightness success
     */
    it('set_value_success_invalid', 0, function () {
        try {
            brightness.setValue('');
        } catch (error) {
            console.info('set_value_success_invalid code: ' + error.code + ' msg:' + error.message);
            // 401: Invalid input parameter
            expect(error.code === 401).assertTrue();
        }
    });
});