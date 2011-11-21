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

#include "bacon-config.h"
#include "bacon-env.h"
#include "bacon-log.h"
#include "bacon-prefs.h"
#include "bacon-util.h"

#define SYSTEM_WIDE_CONF_FILE "bacon.conf"
#define USER_CONF_FILE        ".baconrc"

using std::string;

namespace DEFAULTS
{
  /* These are the values used if these 3 conditions are met:
       1) the system wide config file doesn't exist
       2) a user provided config file doesn't exist
       3) shell variables with the same name do not exist */

  string systemConfig()
  {
    string p[] = {
      SYSCONFDIR, SYSTEM_WIDE_CONF_FILE, ""
    };
    return bacon::env::pathJoin(p);
  }

  string baseDir()
  {
    string p[] = {
      bacon::env::userHomeDir(), "Bacon", ""
    };
    return bacon::env::pathJoin(p);
  }

  string cmRootUrl()
  {
    return string("http://get.cm");
  }
}

namespace bacon
{
  Config::Config()
    : File(DEFAULTS::systemConfig())
    , mUser(NULL)
  {
    string p[] = {
      env::userHomeDir(), USER_CONF_FILE, ""
    };

    mUser = new File(env::pathJoin(p));

    if (!mUser->isFile()) {
#ifdef _WIN32
      p[1] = "_bacon";
      mUser->change(env::pathJoin(p));
      if (!mUser->isFile()) {
#endif
        delete mUser;
        mUser = NULL;
#ifdef _WIN32
      }
#endif
    }
  }

  Config::~Config()
  {
    if (mUser) {
      delete mUser;
      mUser = NULL;
    }
  }

  string Config::userDefined(const string &key)
  {
    if (!mUser) {
      return string("");
    }

    string value("");

    if (mUser->open()) {
      for (string ln = mUser->readLine(); !ln.empty(); ln = mUser->readLine())
      {
        size_t pos = ln.find(key);

        if (pos != string::npos) {

          while (ln[pos] != '=') {
            ++pos;
          }

          for (pos += 1; pos < ln.size(); ++pos) {
            value += ln[pos];
          }

          if (!value.empty()) {
            break;
          }
        }
      }
    }

    value = util::convertShellSymbols(value);
    return value;
  }

  string Config::valueOf(const string &key)
  {
    string value;

    if (!(value = env::variableValue(key)).empty()) {
      return value;
    } else if (!(value = userDefined(key)).empty()) {
      return value;
    } else {
      value = "";
    }

    if (open()) {

      for (string ln = readLine(); !ln.empty(); ln = readLine()) {
        size_t pos = ln.find(key);

        if (pos != string::npos) {

          while (ln[pos] != '=') {
            ++pos;
          }

          for (pos += 1; pos < ln.size(); ++pos) {
            value += ln[pos];
          }

          if (!value.empty()) {
            break;
          }
        }
      }

    } else {

      if (key == "BACON_BASE_DIR") {
        return DEFAULTS::baseDir();
      } else if (key == "BACON_CM_ROOT_SERVER") {
        return DEFAULTS::cmRootUrl();
      }
    }

    value = util::convertShellSymbols(value);
    return value;
  }
}

