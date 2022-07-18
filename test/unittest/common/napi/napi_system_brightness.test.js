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

import brightness from "@system.brightness";
import power from '@ohos.power';

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index';

const INPUT_ERROR_CODE_CODE = 202;
const SET_VALUE_MSG = "value is not an available number";
const SET_MODE_MSG = "value is not an available number";
const SET_KEEP_SCREEN_ON_MSG = "value is not an available boolean";

function sleep(time){
    return new Promise((resolve) => setTimeout(resolve, time));
}

describe('PowerMgrDisplayUnitTest', function () {
    beforeAll(function() {
        /*
         * @tc.setup: setup invoked before all test cases
         */
        console.info('PowerMgrDisplayUnitTest beforeAll called');
    })

    afterAll(function() {
        /*
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('PowerMgrDisplayUnitTest afterAll called');
    })

    beforeEach(function() {
        /*
         * @tc.setup: setup invoked before each test case
         */
        console.info('PowerMgrDisplayUnitTest beforeEach called');
    })

    afterEach(function() {
        /*
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('PowerMgrDisplayUnitTest afterEach called');
    })

    /**
     * @tc.number PowerMgrDisplayUnitTest001
     * @tc.name get_value_success
     * @tc.desc Get brightness success
     */
    it('get_value_success', 0, function () {
        let currValue = 100;
        let setValue = 120;
        brightness.getValue({
            success: (data) => {
                currValue = data.value;
                let value = (data.value > 0) && (data.value <= 255);
                expect(value).assertTrue();
            }
        })
        brightness.setValue({
            value: setValue
        });
        brightness.getValue({
            success: (data) => {
                expect(setValue === data.value).assertTrue();
            },
            fail: (data, code) => {
                console.log("get_value_success, data: " + data + ", code: " + code)
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        })

        brightness.setValue({
            value: currValue
        });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest002
     * @tc.name get_value_success_not_must_test
     * @tc.desc Get brightness
     */
    it('get_status_test_success_not_must', 0, function () {
        brightness.getValue({
            fail: (data, code) => {
                console.log("get_status_test_success_not_must, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest003
     * @tc.name set_value_success_all
     * @tc.desc Set brightness success
     */
    it('set_value_success_all', 0, function () {
        let setValue = 200;
        let currValue = 100;
        brightness.getValue({
            success: (data) => {
                currValue = data.value;
            }
        })

        brightness.setValue({
            value: setValue,
            success: () => {
                brightness.getValue({
                    success: (data) => {
                        expect(data.value === setValue).assertTrue();
                    }
                })
            },
            fail: (data, code) => {
                console.log("set_value_success_all, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        })

        brightness.setValue({
            value: currValue
        });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest004
     * @tc.name set_value_success_value
     * @tc.desc Set brightness success
     */
    it('set_value_success_value', 0, function () {
        let setValue = 50;
        let currValue = 100;
        brightness.getValue({
            success: (data) => {
                currValue = data.value;
            }
        })
        brightness.setValue({ value: setValue });
        brightness.getValue({
            success: (data) => {
                console.log("set_value_success_value, brightness: " + data.value);
                expect(data.value === setValue).assertTrue();
            }
        });
        brightness.setValue({ value: currValue });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest005
     * @tc.name set_value_string
     * @tc.desc Set brightness fail
     */
    it('set_value_string', 0, function () {
        let setValue = "50";
        brightness.setValue({
            value: setValue,
            success: () => {
                console.log("set_value_string success");
                expect().assertFail();
            },
            fail: (data, code) => {
                console.log("set_value_string, data: " + data + ", code: " + code);
                expect(code === NPUT_ERROR_CODE).assertTrue();
                expect(data === SET_VALUE_MSG).assertTrue();
            }
        });
    });

     /**
     * @tc.number PowerMgrDisplayUnitTest006
     * @tc.name set_value_min_value
     * @tc.desc Set brightness min value
     */
    it('set_value_min_value', 0, function () {
        let setValue = 0;
        let currValue = 100;
        brightness.getValue({
            success: (data) => {
                currValue = data.value;
            }
        });
        if (false) {
            brightness.setValue({
                value: setValue,
                success: () => {
                    console.log("set_value_min_value success");
                    brightness.getValue({
                        success: (data) => {
                            console.log("set_value_min_value: value: " + data.value);
                            expect(1 === data.value).assertTrue();
                        }
                    });
                },
                fail: (data, code) => {
                    console.log("set_value_min_value, data: " + data + ", code: " + code);
                    expect().assertFail();
                }
            });
        }

        brightness.setValue({ value: currValue });
    });

     /**
     * @tc.number PowerMgrDisplayUnitTest007
     * @tc.name set_value_max_value
     * @tc.desc Set brightness max value
     */
    it('set_value_max_value', 0, function () {
        let setValue = 500;
        let currValue = 100;
        brightness.getValue({
            success: (data) => {
                console.log("set_value_max_value: get value: " + data.value);
                currValue = data.value;
            }
        });
        brightness.setValue({
            value: setValue,
            success: () => {
                console.log("set_value_max_value success");
                brightness.getValue({
                    success: (data) => {
                        console.log("set_value_max_value: value: " + data.value);
                        expect(255 === data.value).assertTrue();
                    }
                });
            },
            fail: (data, code) => {
                console.log("set_value_max_value, data: " + data + ", code: " + code);
                expect().assertFail();
            }
        });

        brightness.setValue({ value: currValue });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest008
     * @tc.name get_mode_success
     * @tc.desc Get mode success
     */
    it('get_mode_success', 0, function () {
        let modeVal = 0;
        let exec = true;
        brightness.getMode({
            success: (data) => {
                console.log("get_mode_success: get mode: " + data.mode);
                modeVal = data.mode;
            }
        });
        brightness.setMode({
            mode: modeVal ? 0 : 1,
            fail: (data, code) => {
                console.log("get_mode_success, data: " + data + ", code: " + code);
                exec = false;
            }
        });
        if (!exec) {
            return;
        }
        brightness.getMode({
            success: (data) => {
                expect(data.mode === (modeVal ? 0 : 1)).assertTrue();
            },
            fail: (data, code) => {
                console.log("get_mode_success, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });

        brightness.setMode({ mode: modeVal });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest009
     * @tc.name get_mode_success_null
     * @tc.desc Get mode success is null
     */
    it('get_mode_success_null', 0, function () {
        brightness.getMode({
            fail: (data, code) => {
                console.log("get_mode_success_null, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest0010
     * @tc.name set_mode_success
     * @tc.desc set mode success
     */
    it('set_mode_success', 0, function () {
        let modeVal = 0;
        brightness.getMode({
            success: (data) => {
                modeVal = data.mode;
            }
        });

        brightness.setMode({
            mode: modeVal ? 0 : 1,
            success: () => {
                console.log("set_mode_success success");
                brightness.getMode({
                    success: (data) => {
                        console.log("set_mode_success, data: " + data.mode);
                        expect(data.mode === (modeVal ? 0 : 1)).assertTrue();
                    }
                });
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });

        brightness.setMode({ mode: modeVal });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest011
     * @tc.name set_mode_string
     * @tc.desc set mode string
     */
    it('set_mode_fail', 0, function () {
        brightness.setMode({
            mode: "0",
            success: () => {
                expect().assertFail();
            },
            fail: (data, code) => {
                console.log("set_mode_fail, data: " + data + ", code: " + code);
                expect(code === INPUT_ERROR_CODE_CODE).assertTrue();
                expect(data === SET_MODE_MSG).assertTrue();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest012
     * @tc.name set_keep_screen_on_true
     * @tc.desc set keep screen on true
     */
    it('set_keep_screen_on_true', 0, async function () {
        let sleepTime = 35 * 1000;
        brightness.setKeepScreenOn({
            keepScreenOn: true,
            success: () => {
                expect().assertTrue();
            },
            fail: (data, code) => {
                console.log("set_keep_screen_on, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });

        await sleep(sleepTime);
        power.isScreenOn().then(screenOn => {
            console.info('The current screenOn is ' + screenOn);
            expect(screenOn).assertTrue();
        }).catch(error => {
            console.log('isScreenOn error: ' + error);
        })
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest013
     * @tc.name set_keep_screen_on_false
     * @tc.desc set keep screen on false
     */
    it('set_keep_screen_on_false', 0, async function () {
        let sleepTime = 35 * 1000;
        brightness.setKeepScreenOn({
            keepScreenOn: false,
            success: () => {
                expect().assertTrue();
            },
            fail: (data, code) => {
                console.log("set_keep_screen_on_false, data: " + data + ", code: " + code);
                expect().assertFail();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });

        await sleep(sleepTime);
        power.isScreenOn().then(screenOn => {
            console.info('set_keep_screen_on_false The current screenOn is ' + screenOn);
            expect(screenOn).assertFalse();
        }).catch(error => {
            console.log('set_keep_screen_on_false isScreenOn error: ' + error);
        })
    });

    /**
     * @tc.number PowerMgrDisplayUnitTest014
     * @tc.name set_keep_screen_on_not_bool
     * @tc.desc set keep screen on fail
     */
    it('set_keep_screen_on_not_bool', 0, function () {
        brightness.setKeepScreenOn({
            keepScreenOn: "0",
            success: () => {
                expect().assertFail();
            },
            fail: (data, code) => {
                console.log("set_keep_screen_on_not_bool, data: " + data + ", code: " + code);
                expect(data === SET_KEEP_SCREEN_ON_MSG).assertTrue();
                expect(code === INPUT_ERROR_CODE_CODE).assertTrue();
            },
            complete: () => {
                console.log("The device information is obtained successfully. Procedure");
            }
        });
    });
});