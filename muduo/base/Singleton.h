// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_SINGLETON_H
#define MUDUO_BASE_SINGLETON_H

#include "muduo/base/noncopyable.h"

#include <assert.h>
#include <stdlib.h> // atexit
#include <thread>
#include <mutex>

namespace muduo
{
    namespace detail
    {
        // This doesn't detect inherited member functions!
        // http://stackoverflow.com/questions/1966362/sfinae-to-check-for-inherited-member-functions
        template <typename T>
        struct has_no_destroy
        {
            template <typename C>
            static char test(decltype(&C::no_destroy));
            template <typename C>
            static int32_t test(...);
            const static bool value = sizeof(test<T>(0)) == 1;
        };
    } // namespace detail

    template <typename T>
    class Singleton : noncopyable
    {
    public:
        Singleton() = delete;
        ~Singleton() = delete;
        Singleton(const Singleton &) = delete;
        Singleton &operator=(const Singleton &) = delete;
        Singleton(Singleton &&) = delete;
        Singleton &operator=(Singleton &&) = delete;

        static T& instance()
        {
            std::call_once(ponce_, &Singleton::init);
            assert(value_ != NULL);
            return *value_;
        }

    private:
        static void init()
        {
            value_.reset(new T());
            if (!detail::has_no_destroy<T>::value)
            {
                ::atexit(destroy);
            }
        }

        static void destroy()
        {
            value_.swap(T{});
        }

        static std::once_flag ponce_;
        static std::unique_ptr<T> value_;
    };

    template <typename T>
    std::once_flag Singleton<T>::ponce_;

    template <typename T>
    std::unique_ptr<T> Singleton<T>::value_ = nullptr;
} // namespace muduo

#endif  // MUDUO_BASE_SINGLETON_H
