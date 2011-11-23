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
#include <ctime>

#include "bacon-env.h"
#include "bacon-util.h"

#define KB_FACTOR 1024L
#define MB_FACTOR (KB_FACTOR * KB_FACTOR)
#define GB_FACTOR (KB_FACTOR * MB_FACTOR)
#define TB_FACTOR (KB_FACTOR * GB_FACTOR)

#define DIVDBL(x) (((double)bytes) / ((double)(x)))
#define DIVLNG(x) (bytes / ((long)(x)))

using std::string;

namespace
{
  unsigned int rSeed;

  const string urlPrefixes[] = {
    "http://", "https://", "ftp://", "git://", "ssh://", ""
  };
}

namespace bacon
{
  namespace util
  {
    string toUpperCase(const string &str)
    {
      string ret("");

      for (size_t i = 0; i < str.size(); ++i)
      {
        if (str[i] >= 'a' && str[i] <= 'z')
          ret += (str[i] - 'a' + 'A');
        else
          ret += str[i];
      }
      return ret;
    }

    string toLowerCase(const string &str)
    {
      string ret("");

      for (size_t i = 0; i < str.size(); ++i)
      {
        if (str[i] >= 'A' && str[i] <= 'Z')
          ret += (str[i] - 'A' + 'a');
        else
          ret += str[i];
      }
      return ret;
    }

    string bytesToReadable(const size_t maxSize,
                           const long &bytes,
                           const bool precision)
    {
      if (!bytes)
        return string("0B");

      char abbr;
      char buf[maxSize];
      double final;

      if (DIVLNG(TB_FACTOR))
      {
        final = DIVDBL(TB_FACTOR);
        abbr = 'T';
      }
      else if (DIVLNG(GB_FACTOR))
      {
        final = DIVDBL(GB_FACTOR);
        abbr = 'G';
      }
      else if (DIVLNG(MB_FACTOR))
      {
        final = DIVDBL(MB_FACTOR);
        abbr = 'M';
      }
      else if (DIVLNG(KB_FACTOR))
      {
        final = DIVDBL(KB_FACTOR);
        abbr = 'K';
      }
      else
      {
        final = (double)bytes;
        abbr = 'B';
      }

      if (precision)
        snprintf(buf, maxSize, "%.1f%c", final, abbr);
      else
        snprintf(buf, maxSize, "%.0f%c", final, abbr);
      return string(buf);
    }

    void randomSeed()
    {
      rSeed = (unsigned int)time(0);
      random();
      random();
      random();
    }

    unsigned int random()
    {
      unsigned int i;

      i = rSeed = rSeed * 1765301923 + 12345;
      return (i << 16) | ((i >> 16) & 0xffff);
    }

    string timeString(const string &fmt)
    {
      time_t now;
      char buf[fmt.size() * 2];

      time(&now);
      strftime(buf, fmt.size() * 2, fmt.c_str(), localtime(&now));

      return string(buf);
    }

    bool isAlphaNumeric(const char c)
    {
      if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
          (c >= '0' && c <= '9'))
        return true;
      return false;
    }

    void strip(string &str)
    {
      for (size_t i = 0; i < str.size(); ++i)
      {
        if (str[i] == ' ')
          str.erase(i, 1);
      }
    }

    bool isValidUrl(string &url)
    {
      /* This is about all the URL parsing needed for bacon IMO... */

      strip(url);

      if (url[url.size() - 1] != '/')
        url += '/';

      for (size_t i = 0; !urlPrefixes[i].empty(); ++i)
      {
        if (url.substr(0, urlPrefixes[i].size()) == urlPrefixes[i])
          return true;
      }
      return false;
    }

    string convertShellSymbols(const string &val)
    {
      int totalVars = 0;

      for (size_t i = 0; i < val.size(); ++i)
      {
        if (val[i] == env::variableSymbol())
          ++totalVars;
#ifndef _WIN32
        else if (val[i] == '~')
          ++totalVars;
#endif
      }

      if (!totalVars)
        return string(val);

#ifdef _WIN32
      if (totalVars % 2 != 0)
      {
        LOGW("syntax error shell string `%s'", val.c_str());
        return string(val);
      }
      totalVars /= 2;
#endif

      string conv("");
      string var("");
      bool inVar = false;

      for (size_t i = 0; i < val.size(); ++i)
      {
        if (val[i] == env::variableSymbol())
        {
          if (!inVar)
            inVar = true;
#ifdef _WIN32
          else
            inVar = false;
#endif
          continue;
        }
        else if (val[i] == '~')
        {
          conv += env::userHomeDir();
          continue;
        }
        if (inVar)
        {
          if (isAlphaNumeric(val[i]) || val[i] == '_')
          {
            var += val[i];
            continue;
          }
          else
          {
            inVar = false;
            conv += env::variableValue(var);
            var = "";
          }
        }
        if (!inVar)
          conv += val[i];
      }

      if (conv.empty())
        return string(val);
      return conv;
    }
  }
}

