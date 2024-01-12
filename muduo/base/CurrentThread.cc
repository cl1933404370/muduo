// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/CurrentThread.h"
#include <sstream>
#include <stdlib.h>

namespace muduo
{
namespace CurrentThread
{
thread_local int t_cachedTid = 0;
thread_local char t_tidString[32];
thread_local int t_tidStringLength = 6;
thread_local const char* t_threadName = "unknown";
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");

string stackTrace(bool demangle)
{
  boost::stacktrace::stacktrace st;
  std::ostringstream oss;
  oss << st;
  return oss.str();
}

}  // namespace CurrentThread
}  // namespace muduo
