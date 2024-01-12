// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include "muduo/base/Mutex.h"

namespace muduo
{

class Condition : noncopyable
{
 public:
  explicit Condition(MutexLock& mutex)
    : mutex_(mutex)
  {
    //MCHECK(pthread_cond_init(&pcond_, NULL));

  }

  ~Condition()
  {
    //MCHECK(pthread_cond_destroy(&pcond_));

  }

  void wait()
  {
    MutexLock::UnassignGuard ug(mutex_);
    std::unique_lock lck(mutex_.getstdMutex());
    pcond_.wait(lck);
  }

  // returns true if time out, false otherwise.
  bool waitForSeconds(double seconds);

  void notify()
  {
    pcond_.notify_all();
  }

  void notifyAll()
  {
    pcond_.notify_one();
  }

 private:
  MutexLock& mutex_;
  std::condition_variable pcond_;
};

}  // namespace muduo

#endif  // MUDUO_BASE_CONDITION_H
