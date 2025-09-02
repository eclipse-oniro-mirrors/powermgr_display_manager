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

#ifndef DISPLAY_MANAGER_TEST_BASE_H
#define DISPLAY_MANAGER_TEST_BASE_H

#include <iostream>
#include <chrono>
#include <string>

#include "gtest/gtest.h"
#include "display_log.h"

#define DT_FAILURE_TAG "DT_FAILURE_BRIGHTNESS"

namespace OHOS {
namespace PowerMgr {
using DisplayPowerMgr::LABEL_TEST;
using DisplayPowerMgr::DISPLAY_LABEL;

inline void PrintTimestamp(void)
{
    auto tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::string formatTime = (std::ostringstream() << std::put_time(std::localtime(&tm), "%Y-%m-%d %H:%M:%S")).str();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    DISPLAY_HILOGI(LABEL_TEST, "%{public}s, BrtTimestamp:%{public}lld", formatTime.c_str(), timestamp);
    std::cout << formatTime << ", BrtTimestamp:" << timestamp << std::endl;
}

class TimestampListener : public testing::EmptyTestEventListener {
public:
    void OnTestPartResult(const testing::TestPartResult &result) override
    {
        if (result.passed()) {
            return;
        }
        // Also output DT test failures to hilog for easier analysis
        DISPLAY_HILOGI(LABEL_TEST, "[%{public}s]\n%{public}s:%{public}d: Failure\n%{public}s\n",
            DT_FAILURE_TAG, result.file_name(), result.line_number(), result.summary());
        PrintTimestamp();
    }
};

inline void InitBeforeTest(testing::UnitTest& instance)
{
    static bool listenerRegistered = false;
    if (listenerRegistered) {
        return; // Only needs to be registered once for the entire process
    }
    listenerRegistered = true;
    instance.listeners().Append(new TimestampListener());
    std::cout << std::endl;
    PrintTimestamp();
    std::cout << std::endl;
}

class TestBase : public testing::Test {
public:
    TestBase(void)
    {
        auto instance = testing::UnitTest::GetInstance();
        if (instance == nullptr || instance->current_test_info() == nullptr) {
            return;
        }
        InitBeforeTest(*instance);
        const testing::TestInfo *info = instance->current_test_info();
        mName = info->test_suite_name() + std::string(".") + info->name();
        mLocation = info->file() + std::string(":") + std::to_string(info->line());
        DISPLAY_HILOGI(LABEL_TEST, "%{public}s: instance created!", mName.c_str());
    }

    ~TestBase(void)
    {
        DISPLAY_HILOGI(LABEL_TEST, "%{public}s: instance destroyed!", mName.c_str());
    }

protected:
    std::string mName{};
    std::string mLocation{};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DISPLAY_MANAGER_TEST_BASE_H
