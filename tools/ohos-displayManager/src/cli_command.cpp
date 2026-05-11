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

#include "cli_command.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <cstdlib>
#include "cJSON.h"

#include "display_mgr_errors.h"

#ifdef CLI_UT_TEST
#include "mock_display_power_mgr_client.h"
#else
#include "display_power_mgr_client.h"
#endif

using CommandHandler = std::function<int(int, char**)>;

namespace OHOS {
namespace DisplayPowerMgr {

static const uint32_t BRIGHTNESS_MAX_VALUE = 255;
static const uint32_t DECIMAL_BASE = 10;
static const int32_t ARG_OFFSET = 2;
static const int32_t SUBCMD_HELP_ARGC = 3;
static const char* const UNKNOWN_ARG_HELP_SUFFIX =
    ". Use 'ohos-displayManager set-brightness --help' for usage information.";
static const char* const FALLBACK_SUGGESTION =
    "Run 'ohos-displayManager --help' to see available commands and usage.";

int OutputSuccess(const std::string& jsonData)
{
    cJSON* response = cJSON_CreateObject();
    if (response == nullptr) {
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to create JSON response.");
    }
    if (cJSON_AddStringToObject(response, "type", "result") == nullptr ||
        cJSON_AddStringToObject(response, "status", "success") == nullptr) {
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to create JSON response.");
    }
    cJSON* data = cJSON_Parse(jsonData.c_str());
    if (data == nullptr) {
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    if (!cJSON_AddItemToObject(response, "data", data)) {
        cJSON_Delete(data);
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    char* out = cJSON_PrintUnformatted(response);
    if (out == nullptr) {
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    std::cout << out << std::endl;
    cJSON_free(out);
    cJSON_Delete(response);
    return 0;
}

int OutputError(const std::string& code, const std::string& message)
{
    cJSON* response = cJSON_CreateObject();
    if (response == nullptr) {
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    if (cJSON_AddStringToObject(response, "type", "result") == nullptr ||
        cJSON_AddStringToObject(response, "status", "failed") == nullptr ||
        cJSON_AddStringToObject(response, "data", "") == nullptr ||
        cJSON_AddStringToObject(response, "errCode", code.c_str()) == nullptr ||
        cJSON_AddStringToObject(response, "errMsg", message.c_str()) == nullptr ||
        cJSON_AddStringToObject(response, "suggestion", FALLBACK_SUGGESTION) == nullptr) {
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    char* out = cJSON_PrintUnformatted(response);
    if (out == nullptr) {
        cJSON_Delete(response);
        return OutputFallbackError("ERR_INTERNAL_ERROR", "Failed to format JSON response.");
    }
    std::cout << out << std::endl;
    cJSON_free(out);
    cJSON_Delete(response);
    return 1;
}

int OutputFallbackError(const std::string& code, const std::string& message)
{
    std::cout << R"({"type":"result","status":"failed","data":"","errCode":")" << code <<
              R"(","errMsg":")" << message <<
              R"(","suggestion":")" << FALLBACK_SUGGESTION <<
              R"("})" << std::endl;
    return 1;
}

SetBrightnessArgs ParseSetBrightnessArgs(int argc, char** argv)
{
    SetBrightnessArgs args;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--value") == 0 && i + 1 < argc) {
            if (args.hasValue) {
                args.unknownArg = "--value";
                break;
            }
            args.hasValue = true;
            char* endPtr = nullptr;
            args.value = static_cast<uint32_t>(std::strtoul(argv[i + 1], &endPtr, DECIMAL_BASE));
            if (endPtr == argv[i + 1] || *endPtr != '\0') {
                args.unknownArg = argv[i + 1];
            }
            i++;
        } else if (strcmp(argv[i], "--continuous") == 0) {
            if (args.hasContinuous) {
                args.unknownArg = "--continuous";
                break;
            }
            args.hasContinuous = true;
            args.continuous = true;
        } else {
            args.unknownArg = argv[i];
            break;
        }
    }
    return args;
}

int CmdSetBrightness(int argc, char** argv)
{
    SetBrightnessArgs args = ParseSetBrightnessArgs(argc, argv);

    if (!args.unknownArg.empty()) {
        return OutputError("ERR_UNKNOWN_ARG",
            "Unknown or invalid argument: " + args.unknownArg +
            ". Supported arguments: --value <brightness>, --continuous" +
            UNKNOWN_ARG_HELP_SUFFIX);
    }

    if (!args.hasValue) {
        return OutputError("ERR_ARG_MISSING",
            "Missing required parameter: --value is required to specify brightness level."
                " Example: ohos-displayManager set-brightness --value 128");
    }

    if (args.value > BRIGHTNESS_MAX_VALUE) {
        return OutputError("ERR_ARG_OUT_OF_RANGE",
            "Brightness value " + std::to_string(args.value) +
                " is out of range, must be between 0 and 255. Example: --value 128");
    }

#ifdef CLI_UT_TEST
    auto& client = MockDisplayPowerMgrClient::GetInstance();
#else
    auto& client = DisplayPowerMgrClient::GetInstance();
#endif
    bool result = client.SetBrightness(args.value, 0, args.continuous);

    if (!result) {
        DisplayErrors err = client.GetError();
        std::string errCodeStr;
        std::string errMsg;

        switch (err) {
            case DisplayErrors::ERR_PERMISSION_DENIED:
                errCodeStr = "ERR_PERMISSION_DENIED";
                errMsg = "Permission denied: system application identity is required to set brightness.";
                break;
            case DisplayErrors::ERR_SYSTEM_API_DENIED:
                errCodeStr = "ERR_SYSTEM_API_DENIED";
                errMsg = "System API access denied: system-level permission is required.";
                break;
            case DisplayErrors::ERR_PARAM_INVALID:
                errCodeStr = "ERR_ARG_INVALID";
                errMsg = "Invalid parameter: brightness value is not valid. Please check the brightness value (0-255).";
                break;
            case DisplayErrors::ERR_CONNECTION_FAIL:
                errCodeStr = "ERR_INTERNAL_ERROR";
                errMsg = "Failed to connect to display manager service."
                    " Please check if the service is running and try again later.";
                break;
            default:
                errCodeStr = "ERR_INTERNAL_ERROR";
                errMsg = "Set brightness failed with internal error code: " +
                    std::to_string(static_cast<int32_t>(err)) +
                    ". Please check system logs for details or try again later.";
                break;
        }
        return OutputError(errCodeStr, errMsg);
    }

    cJSON* data = cJSON_CreateObject();
    if (data == nullptr) {
        return OutputError("ERR_INTERNAL_ERROR", "Failed to create JSON data for brightness result.");
    }
    if (cJSON_AddNumberToObject(data, "value", args.value) == nullptr ||
        cJSON_AddBoolToObject(data, "continuous", args.continuous) == nullptr ||
        cJSON_AddStringToObject(data, "message", "Brightness set successfully") == nullptr) {
        cJSON_Delete(data);
        return OutputError("ERR_INTERNAL_ERROR", "Failed to build JSON data for brightness result.");
    }
    char* dataStr = cJSON_PrintUnformatted(data);
    if (dataStr == nullptr) {
        cJSON_Delete(data);
        return OutputError("ERR_INTERNAL_ERROR", "Failed to format JSON data for brightness result.");
    }
    std::string dataJson(dataStr);
    cJSON_free(dataStr);
    cJSON_Delete(data);
    return OutputSuccess(dataJson);
}

void PrintHelp(const char* prog)
{
    std::cout << prog << " - Display brightness management CLI tool for adjusting screen brightness" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << prog << " <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  --help                  Display this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  set-brightness          Set display screen brightness" << std::endl;
}

void PrintSubCommandHelp(const char* prog)
{
    std::cout << prog << " set-brightness - Set the brightness of a display device" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  " << prog << " set-brightness --value <brightness> [--continuous]" << std::endl;
    std::cout << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  --value <brightness>    Brightness value (required, range: 0-255)" << std::endl;
    std::cout << "  --continuous            Continuous adjustment mode (optional, default: false)" << std::endl;
    std::cout << "  --help                  Display this help message" << std::endl;
}

int DispatchCommand(int argc, char** argv)
{
    if (argc < ARG_OFFSET) {
        return OutputError("ERR_NO_COMMAND", "No command specified");
    }

    if (strcmp(argv[1], "--help") == 0) {
        if (argc > ARG_OFFSET) {
            return OutputError("ERR_UNKNOWN_ARG",
                "Unexpected argument after --help: " + std::string(argv[ARG_OFFSET]) +
                ". '--help' does not accept additional arguments."
                " Usage: ohos-displayManager --help");
        }
        PrintHelp(argv[0]);
        return 0;
    }

    static std::unordered_map<std::string, CommandHandler> commands = {
        {"set-brightness", CmdSetBrightness},
    };

    std::string cmdName = argv[1];

    auto it = commands.find(cmdName);
    if (it == commands.end()) {
        return OutputError("ERR_UNKNOWN_COMMAND",
            "Unknown command: " + cmdName +
            ". Use '--help' to see available commands.");
    }

    for (int i = ARG_OFFSET; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            if (argc > SUBCMD_HELP_ARGC) {
                return OutputError("ERR_UNKNOWN_ARG",
                    "Unexpected arguments with --help."
                    " '--help' does not accept additional arguments."
                    " Usage: ohos-displayManager " + cmdName + " --help");
            }
            PrintSubCommandHelp(argv[0]);
            return 0;
        }
    }

    return it->second(argc - ARG_OFFSET, argv + ARG_OFFSET);
}

}  // namespace DisplayPowerMgr
}  // namespace OHOS
