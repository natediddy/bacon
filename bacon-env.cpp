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
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>

#include "bacon-env.h"
#include "bacon-file.h"

#ifdef _WIN32
#define HOME_VAR_STR "USERPROFILE"
#define PATH_SEP_CHAR '\\'
#else
#define HOME_VAR_STR "HOME"
#define PATH_SEP_CHAR '/'
#endif

#define BACON_APP_DIR "Bacon"

using std::string;
using std::vector;

namespace bacon
{
  namespace env
  {
    string userHomeDir()
    {
      return string(getenv(HOME_VAR_STR));
    }

    string appDir()
    {
      string p[] = {
        userHomeDir(), BACON_APP_DIR, ""
      };
      return pathJoin(p);
    }

    string pathJoin(const string pathv[])
    {
      string result;

      for (int i = 0; !pathv[i].empty(); i++) {
        result += pathv[i];
        if (!pathv[i + 1].empty()) {
          result += PATH_SEP_CHAR;
        }
      }
      return result;
    }

    char dirSeparator()
    {
      return PATH_SEP_CHAR;
    }

    bool createAppDir()
    {
      File d(appDir());
      return d.makeDir();
    }
  }
}

