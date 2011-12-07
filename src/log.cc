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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_MALLOC_H   /* for free() */
#include <malloc.h>
#else
#include <cstdlib>
#endif

#include "env.h"
#include "prefs.h"
#include "util.h"

#define LOG_FILENAME       "bacon.log"
#define BANNER_DATE_FORMAT "%m/%d/%Y %I:%M:%S%p"

BACON_NAMESPACE_BEGIN

using std::string;

namespace log {

namespace {

#if HAVE_UNISTD_H
int myPid = 0;
#endif
FILE *stream = NULL;

enum BannerType {
    HEADER,
    FOOTER
};

void writeBanner(const BannerType &type)
{
    if (!stream)
        return;

    if (type == HEADER)
        fputs("========================================\n", stream);

    fputs("*** BACON LOGGER ", stream);

    if (type == HEADER)
        fprintf(stream, "START (v%s) ", PACKAGE_VERSION);
    else if (type == FOOTER)
        fprintf(stream, "END (v%s) ", PACKAGE_VERSION);

#if HAVE_UNISTD_H
    fprintf(stream, "(pid:%d) ", myPid);
#endif
    fprintf(stream, "%s ***\n",
            util::timeString(BANNER_DATE_FORMAT).c_str());
}

} /* namespace */

bool _isActive()
{
    return !!stream;
}

void activate()
{
#if HAVE_UNISTD_H
    myPid = (int)getpid();
#endif
    if (!stream)
        stream = fopen(prefs::get(BACON_PREFS_KEY_LOG_PATH).c_str(), "a+");
    writeBanner(HEADER);
}

void deactivate()
{
    writeBanner(FOOTER);
    if (stream) {
        fclose(stream);
        stream = NULL;
    }
}

void _write(char *prefix, const char *msg, ...)
{
    va_list args;

    va_start(args, msg);

    if (prefix && *prefix)
        fprintf(stream, "%s: ", prefix);

    vfprintf(stream, msg, args);
    fputc('\n', stream);
    va_end(args);
    BACON_FREE_ARRAY(prefix);
}

char *_prefix(const char *tag, const char *file, const int line)
{
    char *buf = NULL;
    size_t size = 0;

    if (tag)
        size += strlen(tag) + 1;

    if (file)
        size += strlen(file) + 1;

    size += 20;
    buf = new char[size];

    if (!buf)
        return NULL;

#if HAVE_UNISTD_H
    snprintf(buf, size, "%s:(%d):%s:%d", tag, myPid, file, line);
#else
    snprintf(buf, size, "%s:%s:%d", tag, file, line);
#endif
    return buf;
}

} /* namespace log */

BACON_NAMESPACE_END

