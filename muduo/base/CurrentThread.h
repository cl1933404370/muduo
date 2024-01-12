// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include "muduo/base/Types.h"
#include "muduo/base/crossplatform.h"
#include <boost/stacktrace.hpp>

namespace muduo
{
namespace CurrentThread
{
  // internal
  extern thread_local  int t_cachedTid;
  extern thread_local  char t_tidString[32];
  extern thread_local  int t_tidStringLength;
  extern thread_local  const char* t_threadName;
  void cacheTid();

  inline int tid()
  {
    if (UNLIKELY(t_cachedTid == 0))
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread();

  void sleepUsec(int64_t usec);  // for testing

  string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace muduo

#endif  // MUDUO_BASE_CURRENTTHREAD_H
