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

#include "env.h"

#ifdef _WIN32
#define HOME_VAR_STR "USERPROFILE"
#define VAR_SYMBOL    '%'
#define PATH_SEP_CHAR '\\'
#else
#define HOME_VAR_STR "HOME"
#define VAR_SYMBOL    '$'
#define PATH_SEP_CHAR '/'
#endif

using std::string;

namespace bacon {
namespace env {

string variableValue(const string &key)
{
    string result("");
    const char *val = getenv(key.c_str());

    if (val)
        result += val;
    return result;
}

string variableValue(const char *key)
{
    string result("");
    const char *val = getenv(key);

    if (val)
        result += val;
    return val;
}

string userHomeDir()
{
    return variableValue(HOME_VAR_STR);
}

string pathJoin(const string arr[])
{
    string result;

    for (int i = 0; !arr[i].empty(); i++) {
        result += arr[i];
        if (!arr[i + 1].empty())
            result += PATH_SEP_CHAR;
    }
    return result;
}

char variableSymbol()
{
    return VAR_SYMBOL;
}

char dirSeparator()
{
    return PATH_SEP_CHAR;
}

} /* namespace env */
} /* namespace bacon */

