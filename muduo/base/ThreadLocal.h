// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCAL_H
#define MUDUO_BASE_THREADLOCAL_H

#include "muduo/base/Mutex.h"
#include "muduo/base/noncopyable.h"

#include <thread>
#include <mutex>
#include <mutex>
#include <map>


namespace muduo
{
    template <typename T>
    class ThreadLocal : noncopyable
    {
    public:
        ThreadLocal() = default;
        ~ThreadLocal() = default;

        T& value()
        {
            const std::thread::id tid = std::this_thread::get_id();
            std::lock_guard lock(mutex_);
            if (pkey_.find(tid) == pkey_.end())
            {
                pkey_[tid] = T();
            }
            return pkey_[tid];
        }

    private:
        static void destructor(void* x)
        {
            T* obj = static_cast<T*>(x);
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            T_must_be_complete_type dummy; (void) dummy;
            delete obj;
        }

        std::map<std::thread::id, T> pkey_;
        std::mutex mutex_;
    };
} // namespace muduo

#endif  // MUDUO_BASE_THREADLOCAL_H
