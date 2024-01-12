// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/FileUtil.h"
#include "muduo/base/Logging.h"

#include <cassert>
#include <cerrno>
#include <fcntl.h>
#include <fstream>
#include <cstdio>
#include <sys/stat.h>
#include <io.h>


using namespace muduo;
#if _MSC_VER
#define S_ISREG(m) (((m) & 0170000) == (0100000))
#define S_ISDIR(m) (((m) & 0170000) == (0040000))
#endif

FileUtil::AppendFile::AppendFile(StringArg filename)
    : fp_(fopen(filename.c_str(), "a+")), // 'e' for O_CLOEXEC
      writtenBytes_(0)
{
    assert(fp_);
    setvbuf(fp_, buffer_, _IOFBF, sizeof buffer_);
    // posix_fadvise POSIX_FADV_DONTNEED ?
}

FileUtil::AppendFile::~AppendFile()
{
    fclose(fp_);
}

void FileUtil::AppendFile::append(const char* logline, const size_t len)
{
    size_t written = 0;

    while (written != len)
    {
        const size_t remain = len - written;
        const size_t n = write(logline + written, remain);
        if (n != remain)
        {
            if (const int err = ferror(fp_))
            {
                fprintf(stderr, "AppendFile::append() failed %s\n", strerror_tl(err));
                break;
            }
        }
        written += n;
    }

    writtenBytes_ += written;
}

void FileUtil::AppendFile::flush()
{
    fflush(fp_);
}

size_t FileUtil::AppendFile::write(const char* logline, size_t len) const
{
    return ::_fwrite_nolock(logline, 1, len, fp_);
}

FileUtil::ReadSmallFile::ReadSmallFile(StringArg filename)
    :fd_(_open(filename.c_str(),_O_RDONLY)),
     err_(0)
{
    buf_[0] = '\0';
	if (fd_<0)
	{
		err_ = errno;
	}
}

FileUtil::ReadSmallFile::~ReadSmallFile()
{
    if (fd_ > 0)
	{
		::_close(fd_);
	}
}

// return errno
template <typename String>
int FileUtil::ReadSmallFile::readToString(int maxSize,
                                          String* content,
                                          int64_t* fileSize,
                                          int64_t* modifyTime,
                                          int64_t* createTime)
{
    //static_assert(sizeof(off_t) == 8, "_FILE_OFFSET_BITS = 64");
    assert(content != NULL);
    int err = err_;
    if (fd_ >= 0)
    {
        content->clear();

        if (fileSize)
        {
            struct stat statbuf;
            if (::fstat(fd_, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    *fileSize = statbuf.st_size;
                    content->reserve(static_cast<int>(std::min(implicit_cast<int64_t>(maxSize), *fileSize)));
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    err = EISDIR;
                }
                if (modifyTime)
                {
                    *modifyTime = statbuf.st_mtime;
                }
                if (createTime)
                {
                    *createTime = statbuf.st_ctime;
                }
            }
            else
            {
                err = errno;
            }
        }

        while (content->size() < implicit_cast<size_t>(maxSize))
        {
            //todo
            const size_t toRead = std::min(implicit_cast<size_t>(maxSize) - content->size(), sizeof(buf_));
            if (const int n = ::_read(fd_,buf_,toRead); n > 0)
            {
                content->append(buf_, n);
            }
            else
            {
                if (n < 0)
                {
                    err = errno;
                }
                break;
            }
        }
    }
    return err;
}

int FileUtil::ReadSmallFile::readToBuffer(int* size)
{
    int err = err_;
	if (fd_ > 0)
	{
        if (const int n = ::_read(fd_, buf_, sizeof(buf_) - 1); n >= 0)
		{
			if (size)
			{
				*size = static_cast<int>(n);
			}
			buf_[n] = '\0';
		}
		else
		{
			err = errno;
		}
	}
	return err;
}

template int FileUtil::readFile(StringArg filename,
                                int maxSize,
                                string* content,
                                int64_t*, int64_t*, int64_t*);

template int FileUtil::ReadSmallFile::readToString(
    int maxSize,
    string* content,
    int64_t*, int64_t*, int64_t*);
