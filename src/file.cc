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

#include "config.h"

#include <cerrno>
#include <cstring>

#if HAVE_MALLOC_H
#include <malloc.h>
#else
#include <cstdlib>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include "env.h"
#include "file.h"
#include "util.h"

#define REFRESH_FILE_PROPS \
    do { \
        BACON_FREE(mProp); \
        mProp = new FilePropImpl(mName.c_str()); \
    } while (0)

#define PATH_SEPARATORS "/\\"

BACON_NAMESPACE_BEGIN

using std::string;

class File::FilePropImpl {
public:
    FilePropImpl(const char *name);
    bool exists() const;
    bool isDir() const;
    bool isFile() const;
    size_t sizeInBytes() const;
private:
#if HAVE_SYS_STAT_H
    struct stat mStatBuf;
#endif
    bool mIsDir;
    bool mIsFile;
    size_t mSizeInBytes;
};

File::FilePropImpl::FilePropImpl(const char *name)
    : mIsDir(false)
    , mIsFile(false)
    , mSizeInBytes(0)
{
#if HAVE_SYS_STAT_H
    memset(&mStatBuf, 0, sizeof (struct stat));

    if (!stat(name, &mStatBuf)) {
        if (S_ISDIR(mStatBuf.st_mode))
            mIsDir = true;
        else if (S_ISREG(mStatBuf.st_mode))
            mIsFile = true;
        else
            BACON_LOGW("`%s' is neither a file nor directory", name);
        if (mIsDir || mIsFile)
            mSizeInBytes = mStatBuf.st_size;
    } else {
        BACON_LOGW("stat: `%s': %s", name, strerror(errno));
    }
#endif
}

bool File::FilePropImpl::exists() const
{
    if (mIsDir || mIsFile)
        return true;
    return false;
}

bool File::FilePropImpl::isDir() const
{
    return mIsDir;
}

bool File::FilePropImpl::isFile() const
{
    return mIsFile;
}

size_t File::FilePropImpl::sizeInBytes() const
{
    return mSizeInBytes;
}

File::File(const string &name)
    : mStream(NULL)
    , mName(name)
    , mProp(new File::FilePropImpl(name.c_str()))
{}

File::~File()
{
    close();
    BACON_FREE(mProp);
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

bool File::open(const char *mode /*= "r"*/)
{
    mStream = fopen(mName.c_str(), mode);

    if (!mStream)
        return false;

    REFRESH_FILE_PROPS;
    return true;
}

bool File::close()
{
    bool ret = true;

    if (mStream && (ret = !fclose(mStream)))
        mStream = NULL;
    REFRESH_FILE_PROPS;
    return ret;
}

bool File::dispose()
{
    bool ret = false;

    if (exists())
        ret = !remove(mName.c_str());
    REFRESH_FILE_PROPS;
    return ret;
}

bool File::makeDir()
{
    bool ret = true;

    if (!exists())
        ret =
#if HAVE_MKDIR
            !mkdir(mName.c_str(), S_IRWXU)
#else
#ifdef _WIN32
            CreateDirectory(mName.c_str(), NULL)
#endif
#endif
        ;
    REFRESH_FILE_PROPS;
    return ret;
}

bool File::makeDirs()
{
    bool ret = true;
    char path[FILENAME_MAX];
    char *p = NULL;
    char *pn = NULL;

    strcpy(path, dirName().c_str());
    pn = strchr(path, env::dirSeparator());

    if (pn++) {
        while ((p = strchr(pn, env::dirSeparator()))) {
            *p = '\0';
            File f(path);
            if (!f.makeDir()) {
                ret = false;
                break;
            }
            *p = env::dirSeparator();
            pn = p + 1;
        }
    }

    if (ret && !makeDir())
        ret = false;
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
    return util::bytesToReadable(12, mProp->sizeInBytes(), true);
}

void File::change(const string &name)
{
    close();
    mName = name;
    REFRESH_FILE_PROPS;
}

void File::writeLine(const string &line)
{
    if (isOpen())
        fprintf(mStream, "%s\n", line.c_str());
}

string File::readLine()
{
    string line("");
    bool comment = false;

    if (isOpen()) {
        int ch;
        while ((ch = fgetc(mStream)) != EOF) {
            if (ch == (int)'#') {
                if (!comment)
                    comment = true;
            }
            if (ch == (int)'\n') {
                if (comment) {
                    comment = false;
                    continue;
                }
                if (line.empty())
                    continue;
                break;
            }
            if (!comment)
                line += (char)ch;
        }
    }
    return line;
}

BACON_NAMESPACE_END
