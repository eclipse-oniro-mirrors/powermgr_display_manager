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
#include <cJSON.h>
#include <gtest/gtest.h>
#include "brightness_config_parser.h"
#include "calculation_config_parser.h"
#include "config_parser_base.h"
#include "display_log.h"
#include "lux_filter_config_parser.h"
#include "lux_threshold_config_parser.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
using namespace std;

class BrightnessConfigParseTest : public Test {
public:
    void SetUp() override
    {
        root_ = cJSON_CreateObject();
    }

    void TearDown() override
    {
        cJSON_Delete(root_);
    }

    cJSON* root_;
};

namespace {
constexpr float POINT_1_X = 1.5f;
constexpr float POINT_1_Y = 2.3f;
constexpr float POINT_2_X = 4.0f;
constexpr float POINT_2_Y = 5.0f;
constexpr size_t NUMBER_ONE = 1;
constexpr size_t NUMBER_TWO = 2;
constexpr size_t DISPLAY_ID_ONE = 100;
constexpr size_t DISPLAY_ID_TWO = 101;
constexpr size_t SENSOR_ID_ONE = 200;
constexpr size_t SENSOR_ID_TWO = 201;
constexpr size_t DEFAULT_VALUE_NUMBER = -1;
constexpr float DEFAULT_VALUE_FLOAT = -1.0f;

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest001 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON* item1 = cJSON_CreateFloatArray((const float[]){POINT_1_X, POINT_1_Y}, NUMBER_TWO);
    cJSON* item2 = cJSON_CreateFloatArray((const float[]){POINT_2_X, POINT_2_Y}, NUMBER_TWO);
    cJSON_AddItemToArray(array, item1);
    cJSON_AddItemToArray(array, item2);
    cJSON_AddItemToObject(root_, "points", array);

    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);

    ASSERT_EQ(data.size(), NUMBER_TWO);
    EXPECT_EQ(data[0].x, POINT_1_X);
    EXPECT_EQ(data[0].y, POINT_1_Y);
    EXPECT_EQ(data[1].x, POINT_2_X);
    EXPECT_EQ(data[1].y, POINT_2_Y);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest001 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest002 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON* item = cJSON_CreateFloatArray((const float[]){POINT_1_X}, NUMBER_ONE);
    cJSON_AddItemToArray(array, item);
    cJSON_AddItemToObject(root_, "points", array);

    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest002 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest003 function start!");
    EXPECT_TRUE(root_);
    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest003 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest004 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToObject(root_, "points", array);
    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest004 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest005 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "points", cJSON_CreateString("not an array"));
    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest005 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest006 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateString("not array"));
    cJSON_AddItemToObject(root_, "points", array);

    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.ParsePointXy(root_, "points", data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest006 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest007 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();

    cJSON* item1 = cJSON_CreateObject();
    cJSON_AddItemToObject(item1, "displayMode", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(item1, "displayId", cJSON_CreateNumber(DISPLAY_ID_ONE));
    cJSON_AddItemToObject(item1, "sensorId", cJSON_CreateNumber(SENSOR_ID_ONE));
    cJSON_AddItemToArray(array, item1);

    cJSON* item2 = cJSON_CreateObject();
    cJSON_AddItemToObject(item2, "displayMode", cJSON_CreateNumber(NUMBER_ONE));
    cJSON_AddItemToObject(item2, "displayId", cJSON_CreateNumber(DISPLAY_ID_TWO));
    cJSON_AddItemToObject(item2, "sensorId", cJSON_CreateNumber(SENSOR_ID_TWO));
    cJSON_AddItemToArray(array, item2);

    cJSON_AddItemToObject(root_, "displayModeData", array);

    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");

    ASSERT_EQ(data.size(), NUMBER_TWO);
    EXPECT_EQ(data[0].displayId, DISPLAY_ID_ONE);
    EXPECT_EQ(data[0].sensorId, SENSOR_ID_ONE);
    EXPECT_EQ(data[1].displayId, DISPLAY_ID_TWO);
    EXPECT_EQ(data[1].sensorId, SENSOR_ID_TWO);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest007 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest008 function start!");
    EXPECT_TRUE(root_);
    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest008 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest009 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "displayModeData", cJSON_CreateString("not an array"));
    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest009 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest010, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest010 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToArray(array, cJSON_CreateNumber(DISPLAY_ID_ONE));
    cJSON_AddItemToObject(root_, "displayModeData", array);

    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest010 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest011 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();

    cJSON* item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "displayId", cJSON_CreateNumber(DISPLAY_ID_ONE));
    cJSON_AddItemToArray(array, item);
    cJSON_AddItemToObject(root_, "displayModeData", array);

    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");
    ASSERT_EQ(data.size(), NUMBER_ONE);
    EXPECT_EQ(data[0].displayId, DISPLAY_ID_ONE);
    EXPECT_EQ(data[0].sensorId, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest011 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest012 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();

    cJSON* item = cJSON_CreateObject();
    cJSON_AddItemToObject(item, "displayMode", cJSON_CreateNumber(0));
    cJSON_AddItemToArray(array, item);
    cJSON_AddItemToObject(root_, "displayModeData", array);

    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");

    ASSERT_EQ(data.size(), NUMBER_ONE);
    EXPECT_EQ(data[0].displayId, 0);
    EXPECT_EQ(data[0].sensorId, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest012 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest013 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON_AddItemToObject(root_, "displayModeData", array);

    std::unordered_map<int, ScreenData> data;
    ConfigParserBase parser;
    parser.ParseScreenData(root_, "displayModeData", data, "displayMode");
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest013 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest014 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "filterName", cJSON_CreateString("default"));
    cJSON_AddItemToObject(root_, "filterNoFilterNum", cJSON_CreateNumber(NUMBER_ONE));
    cJSON_AddItemToObject(root_, "filterNum", cJSON_CreateNumber(NUMBER_TWO));
    cJSON_AddItemToObject(root_, "filterMaxFuncLuxNum", cJSON_CreateNumber(DISPLAY_ID_ONE));
    cJSON_AddItemToObject(root_, "filterAlpha", cJSON_CreateNumber(POINT_1_X));
    cJSON_AddItemToObject(root_, "filterLuxTh", cJSON_CreateNumber(DISPLAY_ID_TWO));

    LuxFilterConfigParser parser;
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    parser.LuxFilterParseConfigParams(root_, data);

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data["default"].filterNoFilterNum, NUMBER_ONE);
    EXPECT_EQ(data["default"].filterNum, NUMBER_TWO);
    EXPECT_EQ(data["default"].filterMaxFuncLuxNum, DISPLAY_ID_ONE);
    EXPECT_FLOAT_EQ(data["default"].filterAlpha, POINT_1_X);
    EXPECT_EQ(data["default"].filterLuxTh, DISPLAY_ID_TWO);

    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest014 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest015 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "filterNum", cJSON_CreateNumber(NUMBER_ONE));
    LuxFilterConfigParser parser;
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    parser.LuxFilterParseConfigParams(root_, data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest015 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest016 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "filterName", cJSON_CreateNumber(NUMBER_ONE));
    LuxFilterConfigParser parser;
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    parser.LuxFilterParseConfigParams(root_, data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest016 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest017 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "filterName", cJSON_CreateString(""));
    LuxFilterConfigParser parser;
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    parser.LuxFilterParseConfigParams(root_, data);
    EXPECT_TRUE(data.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest017 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest018 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "filterName", cJSON_CreateString("default"));

    LuxFilterConfigParser parser;
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    parser.LuxFilterParseConfigParams(root_, data);

    ASSERT_EQ(data.size(), 1);
    EXPECT_EQ(data["default"].filterNoFilterNum, DEFAULT_VALUE_NUMBER);
    EXPECT_EQ(data["default"].filterNum, DEFAULT_VALUE_NUMBER);
    EXPECT_EQ(data["default"].filterMaxFuncLuxNum, DEFAULT_VALUE_NUMBER);
    EXPECT_FLOAT_EQ(data["default"].filterAlpha, DEFAULT_VALUE_FLOAT);
    EXPECT_EQ(data["default"].filterLuxTh, DEFAULT_VALUE_NUMBER);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest018 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest019 function start!");
    EXPECT_TRUE(root_);
    BrightnessConfig::Data data;
    BrightnessConfigParser::ParseConfig(data);
    EXPECT_FALSE(data.displayModeMap.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest019 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest020 function start!");
    EXPECT_TRUE(root_);
    CalculationConfig::Data data;
    CalculationConfigParser::ParseConfig(0, data);
    EXPECT_FALSE(data.defaultPoints.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest020 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest022 function start!");
    EXPECT_TRUE(root_);
    cJSON* array = cJSON_CreateArray();
    cJSON* item1 = cJSON_CreateArray();
    if (item1 != NULL) {
        cJSON_AddItemToArray(item1, cJSON_CreateString("POINT_1_X"));
        cJSON_AddItemToArray(item1, cJSON_CreateString("POINT_1_Y"));
    }
    cJSON_AddItemToArray(array, item1);
    cJSON_AddItemToObject(root_, "points", array);

    std::vector<PointXy> data;
    ConfigParserBase parser;
    parser.LoadConfigRoot(0, "brightness_config");
    parser.ParsePointXy(root_, "points", data);
    ASSERT_EQ(data.size(), NUMBER_ONE);
    EXPECT_EQ(data[0].x, DEFAULT_VALUE_FLOAT);
    EXPECT_EQ(data[0].y, DEFAULT_VALUE_FLOAT);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest022 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest023 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "modeName", cJSON_CreateNumber(NUMBER_ONE));

    LuxThresholdConfigParser parser;
    LuxThresholdConfig::Data data;
    parser.LuxThresholdParseConfigParams(root_, data);
    EXPECT_TRUE(data.modeArray.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest023 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest024 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "modeName", cJSON_CreateString("indoor"));
    cJSON_AddItemToObject(root_, "brightenDebounceTime", cJSON_CreateNumber(NUMBER_ONE));
    cJSON_AddItemToObject(root_, "darkenDebounceTime", cJSON_CreateNumber(NUMBER_TWO));

    LuxThresholdConfigParser parser;
    LuxThresholdConfig::Data data;
    parser.LuxThresholdParseConfigParams(root_, data);
    ASSERT_EQ(data.modeArray.size(), NUMBER_ONE);
    auto& mode = data.modeArray["indoor"];
    EXPECT_EQ(mode.brightenDebounceTime, NUMBER_ONE);
    EXPECT_EQ(mode.darkenDebounceTime, NUMBER_TWO);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest024 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest025 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "brightenDebounceTime", cJSON_CreateNumber(NUMBER_ONE));

    LuxThresholdConfigParser parser;
    LuxThresholdConfig::Data data;
    parser.LuxThresholdParseConfigParams(root_, data);
    EXPECT_TRUE(data.modeArray.empty());
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest025 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest027, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest027 function start!");
    EXPECT_TRUE(root_);
    BrightnessConfig::Data data;
    bool ret = BrightnessConfigParser::ParseConfigJsonRoot("", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest027 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest028, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest028 function start!");
    EXPECT_TRUE(root_);
    BrightnessConfig::Data data;
    bool ret = BrightnessConfigParser::ParseConfigJsonRoot("[]", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest028 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest029, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest029 function start!");
    EXPECT_TRUE(root_);
    BrightnessConfig::Data data;
    const std::string json = R"({"lid": {"action": 1, "delayMs": 2}})";
    bool ret = BrightnessConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest029 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest030, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest030 function start!");
    EXPECT_TRUE(root_);
    CalculationConfig::Data data;
    bool ret = CalculationConfigParser::ParseConfigJsonRoot(0, "", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest030 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest031, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest031 function start!");
    EXPECT_TRUE(root_);
    CalculationConfig::Data data;
    bool ret = CalculationConfigParser::ParseConfigJsonRoot(0, "[]", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest031 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest032, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest032 function start!");
    EXPECT_TRUE(root_);
    CalculationConfig::Data data;
    const std::string json = R"({"defaultBrightness": 2})";
    bool ret = CalculationConfigParser::ParseConfigJsonRoot(0, json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest032 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest033, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest033 function start!");
    EXPECT_TRUE(root_);
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    LuxFilterConfigParser::ParseConfig(0, data);
    bool ret = LuxFilterConfigParser::ParseConfigJsonRoot("", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest033 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest034, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest034 function start!");
    EXPECT_TRUE(root_);
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    const std::string json = R"({"test": 2})";
    bool ret = LuxFilterConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest034 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest035, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest035 function start!");
    EXPECT_TRUE(root_);
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    const std::string json = R"([{"name":"A", "value":123}, []])";
    bool ret = LuxFilterConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest035 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest036, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest036 function start!");
    EXPECT_TRUE(root_);
    std::unordered_map<std::string, LuxFilterConfig::Data> data;
    std::string json = R"([{"name":"A", "value":123}, {"name":"B", "value":456}])";
    bool ret = LuxFilterConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest036 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest038, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest038 function start!");
    EXPECT_TRUE(root_);
    LuxThresholdConfig::Data data;
    LuxThresholdConfigParser::ParseConfig(0, data);
    bool ret = LuxThresholdConfigParser::ParseConfigJsonRoot("", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest038 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest039, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest039 function start!");
    EXPECT_TRUE(root_);
    LuxThresholdConfig::Data data;
    bool ret = LuxThresholdConfigParser::ParseConfigJsonRoot("[]", data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest039 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest040, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest040 function start!");
    EXPECT_TRUE(root_);
    LuxThresholdConfig::Data data;
    const std::string json = R"({"isLevelEnable": "11", "thresholdMode": "12"})";
    bool ret = LuxThresholdConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest040 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest041, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest041 function start!");
    EXPECT_TRUE(root_);
    LuxThresholdConfig::Data data;
    std::string json = R"({"isLevelEnable": true, "thresholdMode": [{"name":"A", "value":123},
        {"name":"B", "value":456}]})";
    bool ret = LuxThresholdConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest041 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest042, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest042 function start!");
    EXPECT_TRUE(root_);
    LuxThresholdConfig::Data data;
    std::string json = R"({"isLevelEnable": true, "thresholdMode": [{"name":"A", "value":123}, []]})";
    bool ret = LuxThresholdConfigParser::ParseConfigJsonRoot(json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest042 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest043, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest043 function start!");
    EXPECT_TRUE(root_);
    cJSON_AddItemToObject(root_, "modeName", cJSON_CreateString("indoor"));

    LuxThresholdConfigParser parser;
    LuxThresholdConfig::Data data;
    parser.LuxThresholdParseConfigParams(root_, data);
    ASSERT_EQ(data.modeArray.size(), NUMBER_ONE);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest043 function end!");
}

HWTEST_F(BrightnessConfigParseTest, BrightnessConfigParseTest044, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest044 function start!");
    EXPECT_TRUE(root_);
    CalculationConfig::Data data;
    const std::string json = R"({"test": 2})";
    bool ret = CalculationConfigParser::ParseConfigJsonRoot(0, json, data);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessConfigParseTest044 function end!");
}
} // namespace
