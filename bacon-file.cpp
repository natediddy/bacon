/*
 * This file is part of bacon.
 *
 * bacon is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bacon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bacon.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>

#include "bacon-file.h"

#define REFRESH_FILE_PROPS                   \
  do {                                       \
    if (mProp) {                             \
      delete mProp;                          \
      mProp = 0;                             \
    }                                        \
    mProp = new FilePropImpl(mName.c_str()); \
  } while (0)

#define PATH_SEPARATORS "/\\"

#define KB_FACTOR 1024L
#define MB_FACTOR (KB_FACTOR * KB_FACTOR)
#define GB_FACTOR (KB_FACTOR * MB_FACTOR)
#define TB_FACTOR (KB_FACTOR * GB_FACTOR)

#define DIVDBL(x) (((double)bytes) / ((double)(x)))
#define DIVLNG(x) (bytes / ((long)(x)))

#define SIZE_STR_BUF_MAX 12

using std::string;

namespace bacon
{
  class File::FilePropImpl {
  public:
    FilePropImpl(const char * name)
      : mIsDir(false)
      , mIsFile(false)
      , mSizeInBytes(0)
    {
      memset(&mStatBuf, 0, sizeof (struct stat));

      if (!stat(name, &mStatBuf)) {
        if (S_ISDIR(mStatBuf.st_mode))
          mIsDir = true;
        else if (S_ISREG(mStatBuf.st_mode))
          mIsFile = true;
        if (mIsDir || mIsFile)
          mSizeInBytes = mStatBuf.st_size;
      }
    }

    ~FilePropImpl()
    {}

    bool exists() const
    {
      if (mIsDir || mIsFile)
        return true;
      return false;
    }

    bool isDir() const
    {
      return mIsDir;
    }

    bool isFile() const
    {
      return mIsFile;
    }

    size_t sizeInBytes() const
    {
      return mSizeInBytes;
    }

  private:
    struct stat mStatBuf;
    bool mIsDir;
    bool mIsFile;
    size_t mSizeInBytes;
  };

  File::File(const string & filename)
    : mStream(0)
    , mName(filename)
    , mProp(new File::FilePropImpl(filename.c_str()))
  {}

  File::~File()
  {
    close();

    if (mProp) {
      delete mProp;
      mProp = 0;
    }
  }

  string File::name() const
  {
    return mName;
  }

  string File::dirName() const
  {
    return mName.substr(0, mName.find_last_of(PATH_SEPARATORS));
  }

  string File::baseName() const
  {
    return mName.substr(mName.find_last_of(PATH_SEPARATORS) + 1);
  }

  bool File::isOpen() const
  {
    return !!mStream;
  }

  bool File::open(const char * mode)
  {
    mStream = fopen(mName.c_str(), mode);

    if (!mStream) {
      return false;
    }
    REFRESH_FILE_PROPS;
    return true;
  }

  bool File::close()
  {
    bool ret = true;

    if (mStream && (ret = !fclose(mStream))) {
      mStream = 0;
    }
    REFRESH_FILE_PROPS;
    return ret;
  }

  bool File::dispose()
  {
    bool ret = false;

    if (exists()) {
      ret = !remove(mName.c_str());
    }
    REFRESH_FILE_PROPS;
    return ret;
  }

  bool File::makeDir()
  {
    bool ret = true;

    if (!exists()) {
      ret = !mkdir(mName.c_str(), S_IRWXU);
    }
    REFRESH_FILE_PROPS;
    return ret;
  }

  bool File::exists() const
  {
    return mProp && mProp->exists();
  }

  bool File::isDir() const
  {
    return mProp && mProp->isDir();
  }

  bool File::isFile() const
  {
    return mProp && mProp->isFile();
  }

  size_t File::sizeInBytes() const
  {
    return mProp && mProp->sizeInBytes();
  }

  string File::sizeString() const
  {
    string result;
    string abbr;
    char buf[SIZE_STR_BUF_MAX];
    double final;
    long bytes = 0L;

    if (mProp) {
      bytes = (long)mProp->sizeInBytes();
    }

    if (!bytes) {
      result += "0 bytes";
      return result;
    }

    if (DIVLNG(TB_FACTOR)) {
      final = DIVDBL(TB_FACTOR);
      abbr = "TB";
    } else if (DIVLNG(GB_FACTOR)) {
      final = DIVDBL(GB_FACTOR);
      abbr = "GB";
    } else if (DIVLNG(MB_FACTOR)) {
      final = DIVDBL(MB_FACTOR);
      abbr = "MB";
    } else if (DIVLNG(KB_FACTOR)) {
      final = DIVDBL(KB_FACTOR);
      abbr = "KB";
    } else {
      final = (double)bytes;
      abbr = "bytes";
    }

    snprintf(buf, SIZE_STR_BUF_MAX, "%.1f", final);
    result = buf;
    result += " ";
    result += abbr;
    return result;
  }
}

