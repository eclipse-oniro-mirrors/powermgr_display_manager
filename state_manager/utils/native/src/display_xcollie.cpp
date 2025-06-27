/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * limitations under the License.
 */

#include "display_log.h"
#include "display_xcollie.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace DisplayPowerMgr {
DisplayXCollie::DisplayXCollie(const std::string &logTag, bool isRecovery)
{
    logTag_ = logTag;
    isCanceled_.store(false, std::memory_order_release);
    const int DFX_DELAY_S = 60;
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG;
    if (isRecovery) {
        flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    }
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(logTag_, DFX_DELAY_S, nullptr, nullptr, flag);
    if (id_ == HiviewDFX::INVALID_ID) {
        DISPLAY_HILOGE(COMP_SVC, "Start DisplayXCollie SetTimer fail, tag:%{public}s, timeout(s):%{public}u",
            logTag_.c_str(), DFX_DELAY_S);
        return;
    }
    DISPLAY_HILOGD(COMP_SVC, "Start DisplayXCollie, id:%{public}d, tag:%{public}s, timeout(s):%{public}u", id_,
        logTag_.c_str(), DFX_DELAY_S);
}

DisplayXCollie::~DisplayXCollie()
{
    CancelDisplayXCollie();
}

void DisplayXCollie::CancelDisplayXCollie()
{
    if (isCanceled_.load(std::memory_order_acquire)) {
        return;
    }
    if (id_ != HiviewDFX::INVALID_ID) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        id_ = HiviewDFX::INVALID_ID;
    }
    isCanceled_.store(true, std::memory_order_release);
    DISPLAY_HILOGD(COMP_SVC, "Cancel DisplayXCollie, id:%{public}d, tag:%{public}s", id_, logTag_.c_str());
}
} // namespace DisplayPowerMgr
} // namespace OHOS