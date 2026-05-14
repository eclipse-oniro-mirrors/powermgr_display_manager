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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iostream>

#include "cli_command.h"
#include "mock_display_power_mgr_client.h"

using namespace testing;
using namespace OHOS::DisplayPowerMgr;

// argc values for subcommand args (CmdSetBrightness / ParseSetBrightnessArgs)
static constexpr int ARGC_VALUE_ONLY = 2;              // --value <n>
static constexpr int ARGC_VALUE_AND_FLAG = 3;           // --value <n> + one more flag
// --value <n> + --value <n> or --value <n> --continuous --continuous
static constexpr int ARGC_VALUE_AND_DUPLICATE = 4;

// argc values for top-level dispatch (DispatchCommand)
static constexpr int ARGC_PROG_AND_CMD = 2;             // prog + <cmd|--help>
static constexpr int ARGC_PROG_CMD_HELP = 3;            // prog + <cmd> + --help
static constexpr int ARGC_PROG_CMD_VALUE_PAIR = 4;      // prog + <cmd> + --value <n>

class CliCommandTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        mock_ = &MockDisplayPowerMgrClient::GetInstance();
        savedCout_ = std::cout.rdbuf();
        savedCerr_ = std::cerr.rdbuf();
        std::cout.rdbuf(coutBuf_.rdbuf());
        std::cerr.rdbuf(cerrBuf_.rdbuf());
    }

    void TearDown() override
    {
        std::cout.rdbuf(savedCout_);
        std::cerr.rdbuf(savedCerr_);
    }

    std::string GetOutput()
    {
        return coutBuf_.str();
    }

    std::string GetError()
    {
        return cerrBuf_.str();
    }

    MockDisplayPowerMgrClient* mock_;
    std::streambuf* savedCout_;
    std::streambuf* savedCerr_;
    std::stringstream coutBuf_;
    std::stringstream cerrBuf_;
};

// ==================== ParseSetBrightnessArgs Tests ====================

TEST_F(CliCommandTest, ParseArgs_ValueOnly)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_ONLY, argv);
    EXPECT_TRUE(args.hasValue);
    EXPECT_EQ(args.value, 128u);
    EXPECT_FALSE(args.continuous);
    EXPECT_TRUE(args.unknownArg.empty());
}

TEST_F(CliCommandTest, ParseArgs_ValueWithContinuous)
{
    char arg1[] = "--value";
    char arg2[] = "200";
    char arg3[] = "--continuous";
    char* argv[] = {arg1, arg2, arg3};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_AND_FLAG, argv);
    EXPECT_TRUE(args.hasValue);
    EXPECT_EQ(args.value, 200u);
    EXPECT_TRUE(args.continuous);
    EXPECT_TRUE(args.unknownArg.empty());
}

TEST_F(CliCommandTest, ParseArgs_NoValue)
{
    char arg1[] = "--continuous";
    char* argv[] = {arg1};
    auto args = ParseSetBrightnessArgs(1, argv);
    EXPECT_FALSE(args.hasValue);
    EXPECT_TRUE(args.continuous);
    EXPECT_TRUE(args.unknownArg.empty());
}

TEST_F(CliCommandTest, ParseArgs_UnknownFlag)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char arg3[] = "--unknown";
    char* argv[] = {arg1, arg2, arg3};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_AND_FLAG, argv);
    EXPECT_TRUE(args.hasValue);
    EXPECT_EQ(args.unknownArg, "--unknown");
}

TEST_F(CliCommandTest, ParseArgs_PartialNumericValue)
{
    char arg1[] = "--value";
    char arg2[] = "128abc";
    char* argv[] = {arg1, arg2};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_ONLY, argv);
    EXPECT_TRUE(args.hasValue);
    EXPECT_EQ(args.value, 128u);
    EXPECT_EQ(args.unknownArg, "128abc");
}

TEST_F(CliCommandTest, ParseArgs_EmptyArgs)
{
    char** argv = nullptr;
    auto args = ParseSetBrightnessArgs(0, argv);
    EXPECT_FALSE(args.hasValue);
    EXPECT_FALSE(args.continuous);
    EXPECT_TRUE(args.unknownArg.empty());
}

TEST_F(CliCommandTest, ParseArgs_DuplicateValue)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char arg3[] = "--value";
    char arg4[] = "200";
    char* argv[] = {arg1, arg2, arg3, arg4};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_AND_DUPLICATE, argv);
    EXPECT_EQ(args.unknownArg, "--value");
}

TEST_F(CliCommandTest, ParseArgs_DuplicateContinuous)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char arg3[] = "--continuous";
    char arg4[] = "--continuous";
    char* argv[] = {arg1, arg2, arg3, arg4};
    auto args = ParseSetBrightnessArgs(ARGC_VALUE_AND_DUPLICATE, argv);
    EXPECT_EQ(args.unknownArg, "--continuous");
}

// ==================== DispatchCommand Tests ====================

TEST_F(CliCommandTest, Dispatch_NoArgs_ReturnsError)
{
    char prog[] = "ohos-displayManager";
    char* argv[] = {prog};
    int ret = DispatchCommand(1, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_NO_COMMAND"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_HelpFlag)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "--help";
    char* argv[] = {prog, arg1};
    int ret = DispatchCommand(ARGC_PROG_AND_CMD, argv);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(GetOutput().find("Display brightness management CLI tool"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_SubCommandHelp)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "set-brightness";
    char arg2[] = "--help";
    char* argv[] = {prog, arg1, arg2};
    int ret = DispatchCommand(ARGC_PROG_CMD_HELP, argv);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(GetOutput().find("Set the brightness of a display device"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_UnknownCommand)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "unknown-cmd";
    char* argv[] = {prog, arg1};
    int ret = DispatchCommand(ARGC_PROG_AND_CMD, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_COMMAND"), std::string::npos);
    EXPECT_NE(output.find("Unknown command: unknown-cmd"), std::string::npos);
}

// ==================== Strict Validation Tests ====================

TEST_F(CliCommandTest, Dispatch_HelpWithExtraArgs_ReturnsError)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "--help";
    char arg2[] = "extra";
    char* argv[] = {prog, arg1, arg2};
    int ret = DispatchCommand(ARGC_PROG_CMD_HELP, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("--help"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_SubCommandHelpWithExtraArgs_ReturnsError)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "set-brightness";
    char arg2[] = "--help";
    char arg3[] = "extra";
    char* argv[] = {prog, arg1, arg2, arg3};
    int ret = DispatchCommand(ARGC_PROG_CMD_VALUE_PAIR, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_UnknownFlag_ReturnsError)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char arg3[] = "--unknown";
    char* argv[] = {arg1, arg2, arg3};
    int ret = CmdSetBrightness(ARGC_VALUE_AND_FLAG, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("--unknown"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_UnknownPositionalArg_ReturnsError)
{
    char arg1[] = "--value";
    char arg2[] = "128";
    char arg3[] = "extra";
    char* argv[] = {arg1, arg2, arg3};
    int ret = CmdSetBrightness(ARGC_VALUE_AND_FLAG, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("extra"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_TrailingValueFlag_ReturnsError)
{
    char arg1[] = "--value";
    char* argv[] = {arg1};
    int ret = CmdSetBrightness(1, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("--value"), std::string::npos);
}

// ==================== CmdSetBrightness Tests ====================

TEST_F(CliCommandTest, SetBrightness_MissingValue)
{
    char arg1[] = "--continuous";
    char* argv[] = {arg1};
    int ret = CmdSetBrightness(1, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_ARG_MISSING"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_OutOfRange_TooHigh)
{
    char arg1[] = "--value";
    char arg2[] = "300";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_ARG_OUT_OF_RANGE"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_Success)
{
    const uint32_t temp = 128;
    EXPECT_CALL(*mock_, SetBrightness(temp, 0, false))
        .WillOnce(Return(true));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 0);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"value\":128"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_SuccessWithContinuous)
{
    const uint32_t temp = 200;
    EXPECT_CALL(*mock_, SetBrightness(temp, 0, true))
        .WillOnce(Return(true));

    char arg1[] = "--value";
    char arg2[] = "200";
    char arg3[] = "--continuous";
    char* argv[] = {arg1, arg2, arg3};
    int ret = CmdSetBrightness(ARGC_VALUE_AND_FLAG, argv);
    EXPECT_EQ(ret, 0);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"continuous\":true"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_PermissionDenied)
{
    EXPECT_CALL(*mock_, SetBrightness(_, _, _))
        .WillOnce(Return(false));
    EXPECT_CALL(*mock_, GetError())
        .WillOnce(Return(DisplayErrors::ERR_PERMISSION_DENIED));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_PERMISSION_DENIED"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_SystemApiDenied)
{
    EXPECT_CALL(*mock_, SetBrightness(_, _, _))
        .WillOnce(Return(false));
    EXPECT_CALL(*mock_, GetError())
        .WillOnce(Return(DisplayErrors::ERR_SYSTEM_API_DENIED));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_SYSTEM_API_DENIED"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_ParamInvalid)
{
    EXPECT_CALL(*mock_, SetBrightness(_, _, _))
        .WillOnce(Return(false));
    EXPECT_CALL(*mock_, GetError())
        .WillOnce(Return(DisplayErrors::ERR_PARAM_INVALID));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_ARG_INVALID"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_ConnectionFail)
{
    EXPECT_CALL(*mock_, SetBrightness(_, _, _))
        .WillOnce(Return(false));
    EXPECT_CALL(*mock_, GetError())
        .WillOnce(Return(DisplayErrors::ERR_CONNECTION_FAIL));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_INTERNAL_ERROR"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_DefaultError)
{
    const int temp = 999;
    EXPECT_CALL(*mock_, SetBrightness(_, _, _))
        .WillOnce(Return(false));
    EXPECT_CALL(*mock_, GetError())
        .WillOnce(Return(static_cast<DisplayErrors>(temp)));

    char arg1[] = "--value";
    char arg2[] = "128";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_INTERNAL_ERROR"), std::string::npos);
    EXPECT_NE(output.find("internal error code: 999"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_NonNumericValue_ReturnsError)
{
    char arg1[] = "--value";
    char arg2[] = "abc";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("abc"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_ValueZero_Success)
{
    EXPECT_CALL(*mock_, SetBrightness(0, 0, false))
        .WillOnce(Return(true));

    char arg1[] = "--value";
    char arg2[] = "0";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(GetOutput().find("\"value\":0"), std::string::npos);
}

TEST_F(CliCommandTest, SetBrightness_Value255_Success)
{
    const uint32_t BRIGHTNESS_MAX_VALUE = 255;
    EXPECT_CALL(*mock_, SetBrightness(BRIGHTNESS_MAX_VALUE, 0, false))
        .WillOnce(Return(true));

    char arg1[] = "--value";
    char arg2[] = "255";
    char* argv[] = {arg1, arg2};
    int ret = CmdSetBrightness(ARGC_VALUE_ONLY, argv);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(GetOutput().find("\"value\":255"), std::string::npos);
}

// ==================== OutputSuccess / OutputError Tests ====================

TEST_F(CliCommandTest, OutputSuccess_Format)
{
    int ret = OutputSuccess("{\"key\":1}");
    EXPECT_EQ(ret, 0);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"data\""), std::string::npos);
}

TEST_F(CliCommandTest, OutputSuccess_InvalidJson_FallbackError)
{
    int ret = OutputSuccess("not valid json");
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("\"errCode\":\"ERR_INTERNAL_ERROR\""), std::string::npos);
    EXPECT_NE(output.find("\"errMsg\":\"Failed to format JSON response.\""), std::string::npos);
}

TEST_F(CliCommandTest, OutputError_Format)
{
    int ret = OutputError("ERR_TEST", "test message");
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("\"data\":\"\""), std::string::npos);
    EXPECT_NE(output.find("\"errCode\":\"ERR_TEST\""), std::string::npos);
    EXPECT_NE(output.find("\"errMsg\":\"test message\""), std::string::npos);
    EXPECT_NE(output.find("\"suggestion\""), std::string::npos);
}

TEST_F(CliCommandTest, OutputFallbackError_Format)
{
    int ret = OutputFallbackError("ERR_TEST", "fallback message");
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"failed\""), std::string::npos);
    EXPECT_NE(output.find("\"data\":\"\""), std::string::npos);
    EXPECT_NE(output.find("\"errCode\":\"ERR_TEST\""), std::string::npos);
    EXPECT_NE(output.find("\"errMsg\":\"fallback message\""), std::string::npos);
    EXPECT_NE(output.find("\"suggestion\""), std::string::npos);
}

// ==================== End-to-End Dispatch Tests ====================

TEST_F(CliCommandTest, Dispatch_SetBrightness_EndToEnd)
{
    const uint32_t temp = 100;
    EXPECT_CALL(*mock_, SetBrightness(temp, 0, false))
        .WillOnce(Return(true));

    char prog[] = "ohos-displayManager";
    char arg1[] = "set-brightness";
    char arg2[] = "--value";
    char arg3[] = "100";
    char* argv[] = {prog, arg1, arg2, arg3};
    int ret = DispatchCommand(ARGC_PROG_CMD_VALUE_PAIR, argv);
    EXPECT_EQ(ret, 0);
    std::string output = GetOutput();
    EXPECT_NE(output.find("\"type\":\"result\""), std::string::npos);
    EXPECT_NE(output.find("\"status\":\"success\""), std::string::npos);
    EXPECT_NE(output.find("\"value\":100"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_SetBrightness_NoSubcommandArgs)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "set-brightness";
    char* argv[] = {prog, arg1};
    int ret = DispatchCommand(ARGC_PROG_AND_CMD, argv);
    EXPECT_EQ(ret, 1);
    EXPECT_NE(GetOutput().find("ERR_ARG_MISSING"), std::string::npos);
}

TEST_F(CliCommandTest, Dispatch_SetBrightness_DuplicateSubcommand)
{
    char prog[] = "ohos-displayManager";
    char arg1[] = "set-brightness";
    char arg2[] = "set-brightness";
    char* argv[] = {prog, arg1, arg2};
    int ret = DispatchCommand(ARGC_PROG_CMD_HELP, argv);
    EXPECT_EQ(ret, 1);
    std::string output = GetOutput();
    EXPECT_NE(output.find("ERR_UNKNOWN_ARG"), std::string::npos);
    EXPECT_NE(output.find("set-brightness"), std::string::npos);
}
