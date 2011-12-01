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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "bacon-actions.h"
#include "bacon-cmd.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-prefs.h"
#include "bacon-util.h"

using std::string;
using std::vector;

string gProgramName;
int gRomHistory = BACON_ROM_HISTORY_DEFAULT;
bool gRomHistExplicit = false;

namespace bacon {
namespace {

bool dlReq = false;
bool wantsSpec = false;

const string configOptions[] = {
    "-b", "-l", "-c",
#ifdef _WIN32
    "/b", "/l", "/c",
#endif
    "--basedir", "--logpath", "--cmserver",
    ""
};

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

bool isConfigCmd(const string &arg)
{
    for (size_t i = 0; !configOptions[i].empty(); ++i) {
        if (arg.find(configOptions[i]) != string::npos)
            return true;
    }
    return false;
}

bool isBasicCmd(const string &arg)
{
    for (size_t i = 0; !basicOptions[i].empty(); i++) {
        if (arg == basicOptions[i])
            return true;
    }
    return false;
}

bool isActionCmd(const string &arg)
{
    for (size_t i = 0; !actionOptions[i].empty(); i++) {
        if (arg == actionOptions[i])
            return true;
    }
    return false;
}

bool isRomHistCmd(const string &arg)
{
    return arg.find("-H") != string::npos ||
        arg.find("--rom-history") != string::npos;
}

void setRomHistory(const string &val)
{
    int hist = util::stringToInt(val);

    if (hist <= 0 || hist > BACON_ROM_HISTORY_MAX)
        return;
    wantsSpec = true;
    gRomHistory = hist;
}

void properProgramName(char *execName)
{
    char *lastSlash = NULL;

    if (!execName) {
        gProgramName = PACKAGE_NAME;
        return;
    }

    lastSlash = strrchr(execName, env::dirSeparator());

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

} /* namespace */

class Cmd::ConfigCmdUtilImpl {
public:
    ConfigCmdUtilImpl()
    {
        for (prefs::Key key = 0; key < KEY_TOTAL; ++key)
            mVals[key] = "";
    }

    void addNew(const string &arg, const string &val)
    {
        if (arg == "-b" || arg == "--basedir"
#ifdef _WIN32
                || arg == "/b"
#endif
           )
            mVals[KEY_BASE_DIR] = val;
        else if (arg == "-l" || arg == "--logpath"
#ifdef _WIN32
                || arg == "/l"
#endif
           )
            mVals[KEY_LOG_PATH] = val;
        else if (arg == "-c" || arg == "--cmserver"
#ifdef _WIN32
                || arg == "/c"
#endif
           )
            mVals[KEY_CM_ROOT_SERVER] = val;
    }

    void set()
    {
        for (prefs::Key key = 0; key < KEY_TOTAL; ++key) {
            if (!mVals[key].empty())
                prefs::override(key, mVals[key]);
        }
    }

private:
    string mVals[KEY_TOTAL];
};

class Cmd::BasicCmdUtilImpl {
public:
    BasicCmdUtilImpl()
        : mFun(showUsage)
    {}

    BasicCmdUtilImpl(const string &basic)
    {
        if (basic == "-h" || basic == "-?" || basic == "--help"
#ifdef _WIN32
                || basic == "/h"
#endif
           )
            mFun = showHelp;
        else if (basic == "-v" || basic == "--version"
#ifdef _WIN32
                || basic == "/v"
#endif
           )
            mFun = showVersion;
        else if (basic == "-d" || basic == "--devices"
#ifdef _WIN32
                || basic == "/d"
#endif
           )
            mFun = showDevices;
        else if (basic == "-u" || basic == "--update-devices"
#ifdef _WIN32
                || basic == "/u"
#endif
           )
            mFun = updateDeviceList;
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
    ActionCmdUtilImpl(const string &action, vector<Device *> &devices)
        : mDevices(devices)
    {
        if (action == "-S" || action == "--show"
#ifdef _WIN32
                || action == "/S"
#endif
           )
            mFun = showAllRoms;
        else if (action == "-s" || action == "--stable"
#ifdef _WIN32
                || action == "/s"
#endif
           )
        {
            mFun = downloadLatestStableRom;
            dlReq = true;
        } else if (action == "-n" || action == "--nightly"
#ifdef _WIN32
                || action == "/n"
#endif
           )
        {
            mFun = downloadLatestNightlyRom;
            dlReq = true;
        } else if (action == "-r" || action == "--release-candidate"
#ifdef _WIN32
                || action == "/r"
#endif
           )
        {
            mFun = downloadLatestRcRom;
            dlReq = true;
        }
    }

    ~ActionCmdUtilImpl()
    {
        for (size_t i = 0; i < mDevices.size(); i++)
            BACON_FREE(mDevices[i]);
    }

    int perform() const
    {
        return (*mFun)(mDevices);
    }

private:
    vector<Device *> mDevices;
    int (*mFun)(const vector<Device *> &devices);
};

Cmd::Cmd(char ***argv)
    : mConf(NULL)
    , mBasic(NULL)
{
    properProgramName(**argv);
    (void)*++(*argv);

    do {
        if (!**argv)
            break;
        mArgs.push_back(string(**argv));
    } while (*(*argv)++);

    analyze();
}

Cmd::~Cmd()
{
    BACON_FREE(mBasic);
    for (size_t i = 0; i < mActions.size(); i++)
        BACON_FREE(mActions[i]);
}

void Cmd::analyze()
{
    size_t argCount = mArgs.size();

    if (!argCount) {
        mBasic = new BasicCmdUtilImpl;
        return;
    }

    for (size_t i = 0; i < argCount; ++i) {
        if (isConfigCmd(mArgs[i])) {
            if (!mConf)
                mConf = new ConfigCmdUtilImpl;
            if (mArgs[i][0] == '-' && mArgs[i][1] == '-') {
                mConf->addNew(mArgs[i].substr(0, mArgs[i].find_first_of('=')),
                        util::convertShellSymbols(mArgs[i].substr(
                                mArgs[i].find_first_of('=') + 1,
                                mArgs[i].size())));
            } else if (mArgs[i].size() > 2) {
                mConf->addNew(mArgs[i].substr(0, 2),
                        mArgs[i].substr(2, mArgs[i].size()));
            } else if ((i + 1) < argCount) {
                mConf->addNew(mArgs[i],
                        util::convertShellSymbols(mArgs[i + 1]));
                ++i;
            }
        } else if (isBasicCmd(mArgs[i])) {
            mBasic = new BasicCmdUtilImpl(mArgs[i]);
            break;
        } else if (isActionCmd(mArgs[i])) {
            vector<Device *> devices;
            size_t i_save = i;
            for (size_t j = i + 1; j < argCount; ++j) {
                if (mArgs[j][0] != '-') {
                    devices.push_back(new Device(mArgs[j]));
                    ++i;
                    continue;
                }
                break;
            }
            mActions.push_back(new ActionCmdUtilImpl(mArgs[i_save], devices));
        } else if (isRomHistCmd(mArgs[i])) {
            string histVal("");
            if (mArgs[i][0] == '-' && mArgs[i][1] == '-') {
                histVal = mArgs[i].substr(mArgs[i].find_first_of('=') + 1,
                        mArgs[i].size());
            } else if (mArgs[i].size() > 2) {
                histVal = mArgs[i].substr(2, mArgs[i].size());
            } else if ((i + 1) < argCount) {
                histVal = mArgs[i + 1];
                ++i;
            }
            setRomHistory(histVal);
        }
    }

    if (mConf)
        mConf->set();
}

int Cmd::exec() const
{
    int retval = EXIT_SUCCESS;
    bool error = false;

    if (mBasic) {
        retval = mBasic->perform();
    } else if (mActions.size()) {
        if (dlReq && wantsSpec && gRomHistory > 1)
            gRomHistExplicit = true;
        for(size_t i = 0; i < mActions.size(); i++) {
            if (mActions[i]->perform() == EXIT_FAILURE) {
                if (!error)
                    error = true;
            }
        }
        if (error)
            retval = EXIT_FAILURE;
    }
    return retval;
}

} /* namespace bacon */

