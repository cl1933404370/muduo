// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/Condition.h"

#include <errno.h>

// returns true if time out, false otherwise.
bool muduo::Condition::waitForSeconds(double seconds)
{
    MutexLock::UnassignGuard ug(mutex_);
    std::unique_lock lck(mutex_.getstdMutex());
    pcond_.wait_for(lck, std::chrono::duration<double>(seconds));
}

