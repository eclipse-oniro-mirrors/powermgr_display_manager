/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef LIGHT_LUX_BUFFER_H
#define LIGHT_LUX_BUFFER_H

#include <cstdint>
#include <iostream>
#include <sstream>

namespace OHOS {
namespace DisplayPowerMgr {

class LightLuxBuffer {
public:
    explicit LightLuxBuffer(unsigned int initialCapacity = LUX_BUFFER_SIZE_DEFAULT);
    virtual ~LightLuxBuffer();
    LightLuxBuffer(const LightLuxBuffer&) = delete;
    LightLuxBuffer& operator=(const LightLuxBuffer&) = delete;
    LightLuxBuffer(LightLuxBuffer&&) = delete;
    LightLuxBuffer& operator=(LightLuxBuffer&&) = delete;

    void Push(const int64_t timestamp, const float data);
    void Prune(const int64_t horizon);
    void Clear();
    float GetData(const unsigned int index) const;
    int64_t GetTime(const unsigned int index) const;
    unsigned int GetSize() const;
    std::string ToString(unsigned int n);

private:
    unsigned int OffsetOf(const unsigned int index) const;
    int LuxBufferCheck();
    int CopyLuxBuffer(int newSize);

private:
    static const unsigned int LUX_BUFFER_SIZE_DEFAULT = 32;

    float* mBufferData {nullptr};
    int64_t* mBufferTime {nullptr};
    unsigned int mCapacity {0};
    unsigned int mStart {0};
    unsigned int mEnd {0};
    unsigned int mCount {0};
};
} // namespace BrightnessPowerMgr
} // namespace OHOS
#endif // LIGHT_LUX_RING_BUFFER_H