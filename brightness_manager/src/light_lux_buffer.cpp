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

#include "light_lux_buffer.h"

#include <cerrno>
#include <cstdlib>
#include <new>
#include <string>
#include <securec.h>
#include <securectype.h>

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {

namespace {
const unsigned int BUFFER_SIZE_INCREASE = 2;
const unsigned int LUX_BUFFER_SIZE_MAX = 512;
}

LightLuxBuffer::LightLuxBuffer(unsigned int initialCapacity)
{
    if (initialCapacity == 0 || initialCapacity >= LUX_BUFFER_SIZE_MAX) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "initialCapacity=%{public}d out of range, Reset to %{public}d",
            initialCapacity, LUX_BUFFER_SIZE_DEFAULT);
        mCapacity = LUX_BUFFER_SIZE_DEFAULT;
    } else {
        mCapacity = initialCapacity;
    }
}

LightLuxBuffer::~LightLuxBuffer()
{
    if (mBufferData != nullptr) {
        delete[] mBufferData;
        mBufferData = nullptr;
    }

    if (mBufferTime != nullptr) {
        delete[] mBufferTime;
        mBufferTime = nullptr;
    }
}

int LightLuxBuffer::LuxBufferCheck()
{
    if (mBufferData == nullptr) {
        mBufferData = new(std::nothrow) float[mCapacity];
        if (mBufferData == nullptr) {
            DISPLAY_HILOGE(FEAT_BRIGHTNESS, "new mBufferData failed!");
            return -1;
        }
    }
    if (mBufferTime == nullptr) {
        mBufferTime = new(std::nothrow) int64_t[mCapacity];
        if (mBufferTime == nullptr) {
            DISPLAY_HILOGE(FEAT_BRIGHTNESS, "new mBufferTime failed!");
            delete[] mBufferData;
            mBufferData = nullptr;
            return -1;
        }
    }
    return 0;
}

int LightLuxBuffer::CopyLuxBuffer(int newSize)
{
    auto newBufferData = new(std::nothrow) float[newSize];
    if (newBufferData == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "newBufferData id null");
        return -1;
    }
    auto newBufferTime = new(std::nothrow) int64_t[newSize];
    if (newBufferTime == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "newBufferTime id null");
        delete[] newBufferData;
        return -1;
    }
    unsigned int length = mCapacity - mStart;
    bool isError = false;
    isError = isError || (memcpy_s(newBufferData, newSize * sizeof(newBufferData[0]),
        mBufferData + mStart, length * sizeof(mBufferData[0])) != EOK);
    isError = isError || (memcpy_s(newBufferTime, newSize * sizeof(newBufferTime[0]),
        mBufferTime + mStart, length * sizeof(mBufferTime[0])) != EOK);
    if (mStart != 0) {
        isError = isError || (memcpy_s(newBufferData + length, (newSize - length) * sizeof(newBufferData[0]),
            mBufferData, mStart * sizeof(mBufferData[0])) != EOK);
        isError = isError || (memcpy_s(newBufferTime + length, (newSize - length) * sizeof(newBufferTime[0]),
            mBufferTime, mStart * sizeof(mBufferTime[0])) != EOK);
    }
    if (isError) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "memcpy_s error, mCapacity=%{public}d, mStart=%{public}d", mCapacity, mStart);
        delete[] newBufferData;
        delete[] newBufferTime;
        return -1;
    }
    delete[] mBufferData;
    mBufferData = newBufferData;
    delete[] mBufferTime;
    mBufferTime = newBufferTime;

    return 0;
}

void LightLuxBuffer::Push(const int64_t timestamp, const float data)
{
    if (LuxBufferCheck() != 0) {
        return;
    }

    unsigned int next = mEnd;
    if (mCount == mCapacity) {
        unsigned long newSize = mCapacity * static_cast<unsigned long>(BUFFER_SIZE_INCREASE);
        if (newSize > static_cast<unsigned long>(LUX_BUFFER_SIZE_MAX)) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "buffer size larger than max value %{public}d already, stop expand",
                LUX_BUFFER_SIZE_MAX);
            return;
        }
        if (mStart >= mCapacity) {
            DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mStart error! mStart(%{public}d) >= mCapacity(%{public}d)",
                mStart, mCapacity);
            return;
        }

        if (CopyLuxBuffer(newSize) != 0) {
            return;
        }

        next = mCapacity;
        mCapacity = static_cast<unsigned int>(newSize);
        mStart = 0;
    }

    if (mBufferTime != nullptr) {
        mBufferTime[next] = timestamp;
    }

    if (mBufferData != nullptr) {
        mBufferData[next] = data;
    }

    mEnd = next + 1;
    if (mEnd == mCapacity) {
        mEnd = 0;
    }
    mCount++;
}

void LightLuxBuffer::Prune(const int64_t horizon)
{
    if (mCount == 0 || mBufferTime == nullptr) {
        return;
    }

    while (mCount > 1) {
        unsigned int next = mStart + 1;
        if (next >= mCapacity) {
            next -= mCapacity;
        }
        if (mBufferTime[next] > horizon) {
            break;
        }
        mStart = next;
        mCount--;
    }
    if (mBufferTime[mStart] < horizon) {
        mBufferTime[mStart] = horizon;
    }
}

void LightLuxBuffer::Clear()
{
    mStart = 0;
    mEnd = 0;
    mCount = 0;
}

float LightLuxBuffer::GetData(const unsigned int index) const
{
    if (mBufferData == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mBufferData is nullptr.");
        return 0;
    }
    if (index >= mCount) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "error! index = %{public}d, out of range mCount = %{public}d", index, mCount);
        return 0;
    }
    return mBufferData[OffsetOf(index)];
}

int64_t LightLuxBuffer::GetTime(const unsigned int index) const
{
    if (mBufferTime == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mBufferTime is nullptr");
        return 0;
    }

    if (index >= mCount) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "error! index = %{public}d, out of range mCount = %{public}d", index, mCount);
        return 0;
    }
    return mBufferTime[OffsetOf(index)];
}

unsigned int LightLuxBuffer::GetSize() const
{
    return mCount;
}

unsigned int LightLuxBuffer::OffsetOf(const unsigned int index) const
{
    unsigned actualIndex = index + mStart;
    if (actualIndex >= mCapacity) {
        actualIndex -= mCapacity;
    }
    return actualIndex;
}

std::string LightLuxBuffer::ToString(unsigned int n)
{
    std::ostringstream result;
    if (n > mCount) {
        n = mCount;
    }
    result << "[";
    for (unsigned int i = mCount-n; i>=0 && i < mCount;i++) {
        result << GetData(i);
        result << "/";
        result << GetTime(i);
        result << ", ";
    }
    result << "]";
    return result.str();
}
} // namespace DisplayPowerMgr
} // namespace OHOS