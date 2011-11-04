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

#include <cstdio>

#include "bacon-util.h"

#define KB_FACTOR 1024L
#define MB_FACTOR (KB_FACTOR * KB_FACTOR)
#define GB_FACTOR (KB_FACTOR * MB_FACTOR)
#define TB_FACTOR (KB_FACTOR * GB_FACTOR)

#define DIVDBL(x) (((double)bytes) / ((double)(x)))
#define DIVLNG(x) (bytes / ((long)(x)))

using std::string;

namespace bacon
{
  namespace util
  {
    string toUpperCase(const string & str)
    {
      char c;
      string ret = "";

      for (string::size_type i = 0; i < str.size(); i++) {
        c = str[i];
        if (str[i] >= 'a' && str[i] <= 'z') {
          c = (str[i] - 'a' + 'A');
        }
        ret += c;
      }
      return ret;
    }

    string bytesToReadable(const size_t & maxSize,
                           const long & bytes,
                           const bool precision)
    {
      string result;
      string abbr;
      char buf[maxSize];
      double final;

      if (!bytes) {
        result += "0B";
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
        abbr = "B";
      }

      if (precision) {
        snprintf(buf, maxSize, "%.1f", final);
      } else {
        snprintf(buf, maxSize, "%.0f", final);
      }
      result = buf;
      result += " ";
      result += abbr;
      return result;
    }
  }
}

