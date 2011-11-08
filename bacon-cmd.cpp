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
#include <cstring>

#include "bacon-actions.h"
#include "bacon-cmd.h"
#include "bacon-device.h"
#include "bacon-env.h"

using std::string;
using std::vector;

string gProgramName;

namespace
{
  const string basicOptions[] = {
    "-h", "-?", "-v", "-d", "-u",
#ifdef _WIN32
    "/h", "/?", "/v", "/d", "/u",
#endif
    "--help", "--version", "--devices", "--update-devices",
    ""
  };

  const string actionOptions[] = {
    "-S", "-s", "-n", "-r",
#ifdef _WIN32
    "/S", "/s", "/n", "/r",
#endif
    "--show", "--stable", "--nightly", "--release-candidate",
    ""
  };

  bool isBasicCmd(const string & arg)
  {
    for (size_t i = 0; !basicOptions[i].empty(); i++) {
      if (arg == basicOptions[i]) {
        return true;
      }
    }
    return false;
  }

  bool isActionCmd(const string & arg)
  {
    for (size_t i = 0; !actionOptions[i].empty(); i++) {
      if (arg == actionOptions[i]) {
        return true;
      }
    }
    return false;
  }

  void properProgramName(char * execName)
  {
    char * lastSlash = 0;

    if (!execName) {
      gProgramName = DEFAULT_PROGRAM_NAME;
      return;
    }

    lastSlash = strrchr(execName, bacon::env::dirSeparator());
    if (!lastSlash) {
#ifdef _WIN32
      lastSlash = strrchr(execName, '/');
      if (!lastSlash) {
#endif
      gProgramName = execName;
      return;
#ifdef _WIN32
      } 
#endif
    }

    (void)*++lastSlash;
    gProgramName = lastSlash;
  }
}

namespace bacon
{
  class Cmd::BasicCmdUtilImpl {
  public:
    BasicCmdUtilImpl()
      : mFun(showUsage)
    {}

    BasicCmdUtilImpl(const string & basic)
    {
      if (basic == "-h" || basic == "-?" || basic == "--help"
#ifdef _WIN32
          || basic == "/h"
#endif
         )
      {
        mFun = showHelp;
      } else if (basic == "-v" || basic == "--version"
#ifdef _WIN32
          || basic == "/v"
#endif
         )
      {
        mFun = showVersion;
      } else if (basic == "-d" || basic == "--devices"
#ifdef _WIN32
          || basic == "/d"
#endif
         )
      {
        mFun = showDevices;
      } else if (basic == "-u" || basic == "--update-devices"
#ifdef _WIN32
          || basic == "/u"
#endif
         )
      {
        mFun = updateDeviceList;
      }
    }

    int perform() const
    {
      return (*mFun)();
    }

  private:
    int (*mFun)();
  };

  class Cmd::ActionCmdUtilImpl {
  public:
    ActionCmdUtilImpl(const string & action, vector<Device *> & devices)
      : mDevices(devices)
    {
      if (action == "-S" || action == "--show"
#ifdef _WIN32
          || action == "/S"
#endif
         )
      {
        mFun = showAllRoms;
      } else if (action == "-s" || action == "--stable"
#ifdef _WIN32
          || action == "/s"
#endif
         )
      {
        mFun = downloadLatestStableRom;
      } else if (action == "-n" || action == "--nightly"
#ifdef _WIN32
          || action == "/n"
#endif
         )
      {
        mFun = downloadLatestNightlyRom;
      } else if (action == "-r" || action == "--release-candidate"
#ifdef _WIN32
          || action == "/r"
#endif
         )
      {
        mFun = downloadLatestRcRom;
      }
    }

    ~ActionCmdUtilImpl()
    {
      for (vector<Device *>::size_type i = 0; i < mDevices.size(); i++) {
        if (mDevices[i]) {
          delete mDevices[i];
          mDevices[i] = 0;
        }
      }
    }

    int perform() const
    {
      return (*mFun)(mDevices);
    }

  private:
    vector<Device *> mDevices;
    int (*mFun)(const vector<Device *> &);
  };

  Cmd::Cmd(char *** argv)
    : mBasic(0)
  {
    properProgramName(**argv);
    env::createAppDir();

    (void)*++(*argv);
    do {
      if (!**argv) {
        break;
      }
      mArgs.push_back(string(**argv));
    } while (*(*argv)++);

    analyze();
  }

  Cmd::~Cmd()
  {
    if (mBasic) {
      delete mBasic;
      mBasic = 0;
    }

    if (mActions.size()) {
      for (vector<ActionCmdUtilImpl *>::size_type i = 0;
           i < mActions.size();
           i++)
      {
        if (mActions[i]) {
          delete mActions[i];
          mActions[i] = 0;
        }
      }
    }
  }

  void Cmd::analyze()
  {
    vector<string>::size_type argCount = mArgs.size();

    if (!argCount) {
      mBasic = new BasicCmdUtilImpl;
      return;
    }

    for (vector<string>::size_type i = 0; i < argCount; i++) {
      if (isBasicCmd(mArgs[i])) {
        mBasic = new BasicCmdUtilImpl(mArgs[i]);
        break;
      } else if (isActionCmd(mArgs[i])) {
        vector<Device *> devices;
        vector<string>::size_type i_save = i;
        for (vector<string>::size_type j = i + 1; j < mArgs.size(); j++) {
          if (mArgs[j][0] != '-') {
            devices.push_back(new Device(mArgs[j]));
            i++;
            continue;
          }
          break;
        }
        mActions.push_back(new ActionCmdUtilImpl(mArgs[i_save], devices));
      } else {
        mBasic = new BasicCmdUtilImpl;
        break;
      }
    }
  }

  int Cmd::exec() const
  {
    int retval = EXIT_SUCCESS;
    bool hadError = false;

    if (mBasic) {
      retval = mBasic->perform();
    } else if (mActions.size()) {
      for(vector<ActionCmdUtilImpl*>::size_type i=0; i<mActions.size(); i++) {
        if (mActions[i]->perform() == EXIT_FAILURE) {
          if (!hadError) {
            hadError = true;
          }
        }
      }
      if (hadError) {
        retval = EXIT_FAILURE;
      }
    }
    return retval;
  }
}

