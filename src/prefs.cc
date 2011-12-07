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

#include "conf.h"
#include "env.h"
#include "file.h"
#include "prefs.h"
#include "util.h"

#define BASE_DIR_KEY_STRING       "BACON_BASE_DIR"
#define LOG_PATH_KEY_STRING       "BACON_LOG_PATH"
#define CM_ROOT_SERVER_KEY_STRING "BACON_CM_ROOT_SERVER"

BACON_NAMESPACE_BEGIN

using std::string;

extern string gProgramName;

namespace prefs {

namespace {

string values[BACON_PREFS_KEY_TOTAL];

string keyToString(const Key key)
{
    string result;

    switch (key) {
    case BACON_PREFS_KEY_BASE_DIR:
        result = BASE_DIR_KEY_STRING;
        break;
    case BACON_PREFS_KEY_LOG_PATH:
        result = LOG_PATH_KEY_STRING;
        break;
    case BACON_PREFS_KEY_CM_ROOT_SERVER:
        result = CM_ROOT_SERVER_KEY_STRING;
        break;
    default:
        result = "";
    }
    return result;
}

} /* namespace */

void init()
{
    Conf conf;

    for (Key key = 0; key < BACON_PREFS_KEY_TOTAL; ++key)
        values[key] = conf.valueOf(keyToString(key));
}

void override(const Key key, const string &val)
{
    values[key] = val;
}

string get(const Key key)
{
    return values[key];
}

bool check()
{
    bool result = true;

    for (Key key = 0; key < BACON_PREFS_KEY_TOTAL; ++key) {
        if (key == BACON_PREFS_KEY_BASE_DIR) {
            File p(get(key));
            if (!p.exists()) {
                if (!p.makeDirs()) {
                    fprintf(stderr, "%s: error: failed to create directory "
                            "`%s'\n", gProgramName.c_str(), p.name().c_str());
                    if (result)
                        result = false;
                }
            }
        } else if (key == BACON_PREFS_KEY_LOG_PATH) {
            if (values[key].empty()) {
                string p[] = {
                    values[BACON_PREFS_KEY_BASE_DIR], ".log", "bacon.log", ""
                };
                values[key] = env::pathJoin(p);
            }
            File p(get(key));
            p.change(p.dirName());
            if (!p.exists()) {
                if (!p.makeDirs()) {
                    fprintf(stderr, "%s: error: failed to create directory "
                            "`%s'\n", gProgramName.c_str(), p.name().c_str());
                    if (result)
                        result = false;
                }
            } else if (p.isFile()) {
                fprintf(stderr, "%s: error: `%s' is a file, needs to be a "
                        "directory\n", gProgramName.c_str(),
                        p.name().c_str());
                if (result)
                    result = false;
            }
        } else if (key == BACON_PREFS_KEY_CM_ROOT_SERVER) {
            string url(get(key));
            if (!util::isValidUrl(url)) {
                fprintf(stderr, "%s: error: `%s' is not a valid URL\n",
                        gProgramName.c_str(), url.c_str());
                if (result)
                    result = false;
            }
        }
    }
    return result;
}

} /* namespace prefs */

BACON_NAMESPACE_END

