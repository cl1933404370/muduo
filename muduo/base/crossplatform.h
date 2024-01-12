#ifndef MUDUO_BASE_CROSSPLATFORM_H
#define MUDUO_BASE_CROSSPLATFORM_H
namespace muduo
{
    #ifdef __GNUC__
    #define LIKELY(x) __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
    #else
    #define LIKELY(x) (x)
    #define UNLIKELY(x) (x)
    #endif

    #define  pid_t int
}
#endif