// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/ProcessInfo.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/FileUtil.h"

#include <algorithm>

#include <assert.h>
#include <iostream>
#include <ostream>
#include <process.h>
#include <stdio.h> // snprintf
#include <stdlib.h>
#if _MSC_VER
#include <windows.h>
#include <sddl.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <processthreadsapi.h>
#include <winternl.h>
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")
#else
#include <dirent.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/times.h>
#endif


namespace muduo
{
    namespace detail
    {
        thread_local int t_numOpenedFiles = 0;

        int fdDirFilter(const struct dirent* d)
        {
            if (::isdigit(d->d_name[0]))
            {
                ++t_numOpenedFiles;
            }
            return 0;
        }

        thread_local std::vector<pid_t>* t_pids = nullptr;

        int taskDirFilter(const struct dirent* d)
        {
            if (::isdigit(d->d_name[0]))
            {
                t_pids->push_back(atoi(d->d_name));
            }
            return 0;
        }

        int scanDir(const char* dirpath, int (*filter)(const struct dirent*))
        {
            struct dirent** namelist = NULL;
            int result = ::scandir(dirpath, &namelist, filter, alphasort);
            assert(namelist == NULL);
            return result;
        }

        Timestamp g_startTime = Timestamp::now();
        // assume those won't change during the life time of a process.
        int g_clockTicks = static_cast<int>(::sysconf(_SC_CLK_TCK));
        int g_pageSize = static_cast<int>(::sysconf(_SC_PAGE_SIZE));
    } // namespace detail
} // namespace muduo

using namespace muduo;
using namespace muduo::detail;

pid_t ProcessInfo::pid()
{
    return ::getpid();
}

string ProcessInfo::pidString()
{
    char buf[32];
    snprintf(buf, sizeof buf, "%d", pid());
    return buf;
}

uid_t ProcessInfo::uid()
{
    return ::getuid();
}

string ProcessInfo::username()
{
    struct passwd pwd;
    struct passwd* result = NULL;
    char buf[8192];
    auto name = "unknownuser";

    getpwuid_r(uid(), &pwd, buf, sizeof buf, &result);
    if (result)
    {
        name = pwd.pw_name;
    }
    return name;
}

uid_t ProcessInfo::euid()
{
    return ::geteuid();
}

Timestamp ProcessInfo::startTime()
{
    return g_startTime;
}

int ProcessInfo::clockTicksPerSecond()
{
    return g_clockTicks;
}

int ProcessInfo::pageSize()
{
    return g_pageSize;
}

bool ProcessInfo::isDebugBuild()
{
#ifdef NDEBUG
  return false;
#else
    return true;
#endif
}

string ProcessInfo::hostname()
{
    // HOST_NAME_MAX 64
    // _POSIX_HOST_NAME_MAX 255
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0)
    {
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    }
    else
    {
        return "unknownhost";
    }
}

string ProcessInfo::procname()
{
    return procname(procStat()).as_string();
}

StringPiece ProcessInfo::procname(const string& stat)
{
    StringPiece name;
    size_t lp = stat.find('(');
    size_t rp = stat.rfind(')');
    if (lp != string::npos && rp != string::npos && lp < rp)
    {
        name.set(stat.data() + lp + 1, static_cast<int>(rp - lp - 1));
    }
    return name;
}

string ProcessInfo::procStatus()
{
    string result;
    FileUtil::readFile("/proc/self/status", 65536, &result);
    return result;
}

string ProcessInfo::procStat()
{
    string result;
    FileUtil::readFile("/proc/self/stat", 65536, &result);
    return result;
}

string ProcessInfo::threadStat()
{
    char buf[64];
    snprintf(buf, sizeof buf, "/proc/self/task/%d/stat", CurrentThread::tid());
    string result;
    FileUtil::readFile(buf, 65536, &result);
    return result;
}

string ProcessInfo::exePath()
{
    string result;
    char buf[1024];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof buf);
    if (n > 0)
    {
        result.assign(buf, n);
    }
    return result;
}

int ProcessInfo::openedFiles()
{
    t_numOpenedFiles = 0;
#if defined(_WIN32)
    using NtQuerySystemInformation_t = NTSTATUS(NTAPI *)(
        ULONG SystemInformationClass,
        PVOID SystemInformation,
        ULONG SystemInformationLength,
        PULONG ReturnLength
    );

    typedef struct _SYSTEM_HANDLE
    {
        ULONG ProcessId;
        BYTE ObjectTypeNumber;
        BYTE Flags;
        USHORT Handle;
        PVOID Object;
        ACCESS_MASK GrantedAccess;
    } SYSTEM_HANDLE, *PSYSTEM_HANDLE;
    typedef struct _SYSTEM_HANDLE_INFORMATION
    {
        ULONG HandleCount;
        SYSTEM_HANDLE Handles[1];
    } SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

#define SystemHandleInformation 16
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

    // Get a list of all open handles
    const auto ntQuerySystemInformation = reinterpret_cast<NtQuerySystemInformation_t>(GetProcAddress(
        GetModuleHandle("ntdll.dll"), "NtQuerySystemInformation"));
    if (ntQuerySystemInformation == nullptr)
    {
        std::cerr << "Failed to get NtQuerySystemInformation function." << '\n';
        return 1;
    }

    ULONG size = 0;
    NTSTATUS status = ntQuerySystemInformation(SystemHandleInformation, nullptr, 0, &size);
    if (status != STATUS_INFO_LENGTH_MISMATCH)
    {
        std::cerr << "Failed to get system handle information size." << '\n';
        return 1;
    }

    const auto handleInfo = static_cast<PSYSTEM_HANDLE_INFORMATION>(malloc(size));
    status = ntQuerySystemInformation(SystemHandleInformation, handleInfo, size, &size);
    if (!NT_SUCCESS(status))
    {
        std::cerr << "Failed to get system handle information." << '\n';
        return 1;
    }

    // Print the handles for the specified process
    const ULONG curProcessID = GetCurrentProcessId();
    for (ULONG i = 0; i < handleInfo->HandleCount; i++)
    {
        if (const SYSTEM_HANDLE handle = handleInfo->Handles[i]; handle.ProcessId == curProcessID)
        {
            std::cout << "Handle: " << handle.Handle << '\n';
        }
    }

    free(handleInfo);
#else
    scanDir("/proc/self/fd", fdDirFilter);
#endif
    return t_numOpenedFiles;
}

int ProcessInfo::maxOpenFiles()
{
#if defined(_WIN32)
    return _getmaxstdio();
#else
    struct rlimit rl;
    if (::getrlimit(RLIMIT_NOFILE, &rl))
    {
        return openedFiles();
    }
    else
    {
        return static_cast<int>(rl.rlim_cur);
    }
#endif
}

ProcessInfo::CpuTime ProcessInfo::cpuTime()
{
    ProcessInfo::CpuTime t;
#if defined(_WIN32)
    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetProcessTimes(GetCurrentProcess(), &creationTime, &exitTime, &kernelTime, &userTime))
    {
        ULARGE_INTEGER userSystemTime;
        userSystemTime.LowPart = userTime.dwLowDateTime;
        userSystemTime.HighPart = userTime.dwHighDateTime;
        t.userSeconds = static_cast<double>(userSystemTime.QuadPart) / 10000000;
        ULARGE_INTEGER kernelSystemTime;
        kernelSystemTime.LowPart = kernelTime.dwLowDateTime;
        kernelSystemTime.HighPart = kernelTime.dwHighDateTime;
        t.systemSeconds = static_cast<double>(kernelSystemTime.QuadPart) / 10000000;
    }
#else
    struct tms tms;
    if (::times(&tms) >= 0)
    {
        const double hz = static_cast<double>(clockTicksPerSecond());
        t.userSeconds = static_cast<double>(tms.tms_utime) / hz;
        t.systemSeconds = static_cast<double>(tms.tms_stime) / hz;
    }
#endif
    return t;
}

int ProcessInfo::numThreads()
{
    int result = 0;
#if defined(_WIN32)
    const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
    {
        // Handle error
        return -1;
    }

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    if (Thread32First(hSnapshot, &te))
    {
        do
        {
            if (te.th32OwnerProcessID == GetCurrentProcessId())
            {
                ++result;
            }
        }
        while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);
#else
  string status = procStatus();
  size_t pos = status.find("Threads:");
  if (pos != string::npos)
  {
    result = ::atoi(status.c_str() + pos + 8);
  }
#endif
    return result;
}

std::vector<pid_t> ProcessInfo::threads()
{
    std::vector<pid_t> result;
    t_pids = &result;
#if defined(_WIN32)
    if (const HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); hSnapshot != INVALID_HANDLE_VALUE)
    {
        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        if (Thread32First(hSnapshot, &te))
        {
            do
            {
                if (te.th32OwnerProcessID == GetCurrentProcessId())
                {
                    result.push_back(te.th32ThreadID);
                }
            }
            while (Thread32Next(hSnapshot, &te));
        }
        CloseHandle(hSnapshot);
    }
#else
  scanDir("/proc/self/task", taskDirFilter);
#endif
    t_pids = nullptr;
    std::ranges::sort(result);
    return result;
}
