/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISPLAY_MANAGER_CLI_COMMAND_H
#define OHOS_DISPLAY_MANAGER_CLI_COMMAND_H

#include <string>
#include <cstdint>

namespace OHOS {
namespace DisplayPowerMgr {

struct SetBrightnessArgs {
    bool hasValue = false;
    bool hasContinuous = false;
    uint32_t value = 0;
    bool continuous = false;
    std::string unknownArg;
};

int OutputSuccess(const std::string& jsonData);
int OutputError(const std::string& code, const std::string& message);
int OutputFallbackError(const std::string& code, const std::string& message);

SetBrightnessArgs ParseSetBrightnessArgs(int argc, char** argv);
int CmdSetBrightness(int argc, char** argv);

void PrintHelp(const char* prog);
void PrintSubCommandHelp(const char* prog);
int DispatchCommand(int argc, char** argv);

}  // namespace DisplayPowerMgr
}  // namespace OHOS

#endif  // OHOS_DISPLAY_MANAGER_CLI_COMMAND_H
