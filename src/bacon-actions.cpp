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
#include <string>

#include "bacon-actions.h"
#include "bacon-device.h"
#include "bacon-devicelist.h"
#include "bacon-env.h"
#include "bacon-htmldoc.h"
#include "bacon-htmlparser.h"
#include "bacon-log.h"
#include "bacon-md5.h"
#include "bacon-rom.h"
#include "bacon-stats.h"
#include "bacon-util.h"

using std::string;
using std::vector;

extern string gProgramName;
extern int gRomHistory;
extern bool gRomHistExplicit;

namespace bacon {
namespace {

void alignPrintNumber(const size_t n)
{
    const size_t p = n + 1;
    int spaces;

    if (p < 10)
        spaces = 3;
    else if (p < 100)
        spaces = 2;
    else if (p < 1000)
        spaces = 1;
    else
        spaces = 0;

    for (int i = 0; i < spaces; ++i)
        fputc(' ', stdout);
    fprintf(stdout, "%zu) ", p);
}

bool validDevice(const string &id)
{
    DeviceList deviceList;

    deviceList.prep();
    return deviceList.hasMatch(id);
}

bool romExists(const string &path,
               const string &deviceId,
               const string &deviceType)
{
    File fp(path);
    bool ret = false;

    if (fp.isFile()) {
        Md5 md5(path, deviceId, deviceType);
        ret = md5.verify();
    } else if (fp.isDir()) {
        LOGW("`%s' is a directory, not a file!", fp.name().c_str());
    }
    return ret;
}

enum Action {
    SHOWONLY,
    DOWNLOAD_ST,
    DOWNLOAD_NI,
    DOWNLOAD_RC
};

string actionToString(const Action &action)
{
    switch (action) {
    case DOWNLOAD_ST:
        return string("stable");
    case DOWNLOAD_NI:
        return string("nightly");
    case DOWNLOAD_RC:
        return string("rc");
    }
    return string("");
}

bool performRomDownload(bool *err,
                        const string &romName,
                        const string &type,
                        const Device *device)
{
    string _p[] = {
        device->romDir(), romName, ""
    };
    string romPath = env::pathJoin(_p);

    if (!romExists(romPath, device->id(), type)) {
        Rom rom(romName, romPath);
        if (rom.fetch()) {
            fputs("\nVerifying file integrity...", stdout);
            Md5 md5(romPath, device->id(), type);
            if (!md5.verify()) {
                fputs(" FAIL\n", stdout);
                fprintf(stderr, "%s: `%s' may be corrupt!\n",
                    gProgramName.c_str(), romPath.c_str());
            } else {
                fputs(" PASS\n", stdout);
                fprintf(stdout, "New ROM located at:\n\t%s\n",
                    romPath.c_str());
            }
        } else {
            fprintf(stderr, "\n%s: error: failed to fetch %s\n",
                gProgramName.c_str(), romName.c_str());
            if (!*err)
                *err = true;
        }
    } else {
        fprintf(stdout, "%s: `%s' exists and has already been "
            "downloaded\n", gProgramName.c_str(), romName.c_str());
    }
}

bool prepRomDownload(const string &type, const Device *device)
{
    HtmlDoc *doc = new HtmlDoc(device->id(), type);
    bool err = false;

    if (doc->fetch()) {
        HtmlParser parser(doc->content());
        vector<string> romNames = parser.latestRomsForDevice();
        BACON_FREE(doc);
        for (size_t i = 0; i < romNames.size(); ++i) {
            if (gRomHistExplicit) {
                if (i == (gRomHistory - 1)) {
                    performRomDownload(&err, romNames[i], type, device);
                    break;
                }
                continue;
            }
            performRomDownload(&err, romNames[i], type, device);
            break;
        }
    } else if (!err) {
        err = true;
    }

    BACON_FREE(doc);
    return err;
}

bool performShowOnly(const Device *device)
{
    string types[3] = {
        "stable", "nightly", "rc"
    };

    fprintf(stdout, "%s:\n", device->id().c_str());
    for (size_t i = 0; i < 3; ++i) {
        Stats stats(device, types[i]);
        fprintf(stdout, "  %s:\n", types[i].c_str());
        if (stats.init()) {
            vector<string> romNames = stats.romNames();
            for (size_t j = 0; j < romNames.size(); ++j) {
                fputc('\t', stdout);
                if (types[i] == "nightly")
                    fputs(util::nightlyBuildNo(romNames[j]).c_str(), stdout);
                else
                    fputs(util::romVersionNo(romNames[j]).c_str(), stdout);
                fputs(" -- ", stdout);
                if (stats.existsLocally(j)) {
                    if (stats.isValid(j))
                        fputs("already downloaded", stdout);
                    else
                        fputs("partially downloaded or corrupt", stdout);
                } else {
                    fputs("not downloaded", stdout);
                }
                fputc('\n', stdout);
            }
        } else {
            fputs("\t(none found)\n", stdout);
        }
    }
}

void downloadAction(bool *err, const Action &action, const Device *device)
{
    if (action != SHOWONLY)
        *err = prepRomDownload(actionToString(action), device);
    else
        *err = performShowOnly(device);
}

int perform(const Action &action, const vector<Device *> &devices)
{
    bool err = false;

    if (!devices.size()) {
        fprintf(stderr, "%s: error: no device(s) given\n",
            gProgramName.c_str());
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < devices.size(); i++) {
        if (!validDevice(devices[i]->id())) {
            fprintf(stderr, "%s: warning: unknown device `%s', skipping...\n",
                gProgramName.c_str(),
            util::toUpperCase(devices[i]->id()).c_str());
            continue;
        } else if (action != SHOWONLY) {
            devices[i]->createRomDir();
        }
        if (action == SHOWONLY)
            fputs("Showing ", stdout);
        else
            fputs("Downloading ", stdout);
        fputs("the ", stdout);
        if (action != SHOWONLY && gRomHistExplicit)
            fprintf(stdout, "%s to ",
                util::properNumber(gRomHistory).c_str());
        else if (action == SHOWONLY)
            fprintf(stdout, "%d ", gRomHistory);
        fputs("latest ", stdout);
        if (action != SHOWONLY)
            fprintf(stdout, "%s ", actionToString(action).c_str());
        fputs("ROM", stdout);
        if (action == SHOWONLY)
            fputs("s from all categories", stdout);
        fprintf(stdout, " for %s...\n",
            util::toUpperCase(devices[i]->id()).c_str());
        downloadAction(&err, action, devices[i]);
    }

    if (err)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

class ListCompareData {
public:
    ListCompareData(DeviceList &deviceList, const vector<string> &oldList);
    void printDiff();
private:
    int mDiffCount;
    vector<string> mDiff;
};

ListCompareData::ListCompareData(DeviceList &deviceList,
                                 const vector<string> &oldList)
{
    vector<string> newList = deviceList.rawList();

    mDiffCount = (int)newList.size() - (int)oldList.size();
    if (mDiffCount <= 0)
        return;

    for (size_t i = 0; i < newList.size(); ++i) {
        bool noMatch = true;
        for (size_t j = 0; j < oldList.size(); ++j) {
            if (newList[i] == oldList[j]) {
                noMatch = false;
                break;
            }
        }
        if (noMatch)
            mDiff.push_back(newList[i]);
    }
}

void ListCompareData::printDiff()
{
    if (mDiffCount <= 0) {
        fputs("No new devices since last update\n", stdout);
        return;
    }

    fputs("There ", stdout);
    if (mDiffCount > 1)
        fputs("are ", stdout);
    else
        fputs("is ", stdout);

    fprintf(stdout, "%d new device", mDiffCount);
    if (mDiffCount > 1)
        fputc('s', stdout);

    fputs(":\n", stdout);
    for (size_t i = 0; i < mDiff.size(); ++i) {
        alignPrintNumber(i);
        fprintf(stdout, "%s\n", mDiff[i].c_str());
    }
}

} /* namespace */

int showUsage()
{
    fprintf(stdout, "Usage: %s [option] [DEVICE] ...\n",
        gProgramName.c_str());
    return EXIT_FAILURE;
}

int showHelp()
{
    showUsage();

    fprintf(stdout,
        "Options:\n"
        "  -S, --show               show stats for latest builds for DEVICE(s)"
          "\n"
        "  -s, --stable             download latest stable build for "
          "DEVICE(s)\n"
        "  -n, --nightly            download latest nightly build for "
          "DEVICE(s)\n"
        "  -r, --release-candidate  download latest release candidate for "
          "DEVICE(s)\n"
        "  -d, --devices            display all available DEVICEs and exit\n"
        "  -u, --update-devices     update the current DEVICE list and exit\n"
        "  -h, -?, --help           display this message and exit\n"
        "  -v, --version            display version information and exit\n"
        "  -H INTEGER, --rom-history=INTEGER\n"
        "                           set the number of latest ROMs to search "
          "for\n"
        "  -b BASEDIR, --basedir=BASEDIR\n"
        "                           set alternate base directory\n"
        "  -l LOGPATH, --logpath=LOGPATH\n"
        "                           write log to alternate location\n"
        "  -c ROOTURL, --cmserver=ROOTURL\n"
        "                           use alternate root url to connect with\n"
        "\nMore than one DEVICE can be given, but only ONE option per "
          "DEVICE list.\n");

    return EXIT_SUCCESS;
}

int showVersion()
{
    fprintf(stdout, "%s %s\n"
#ifdef BACON_MACHTYPE
        "Built for %s\n"
#endif
        "Copyright (C) 2011 Nathan Forbes\n"
        "This is free software; see the source for copying conditions.\n"
        "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
        "PARTICULAR PURPOSE.\n",
        PACKAGE_NAME, PACKAGE_VERSION
#ifdef BACON_MACHTYPE
        , BACON_MACHTYPE
#endif
        );

    return EXIT_SUCCESS;
}

int showDevices()
{
    DeviceList deviceList;

    if (!deviceList.exists()) {
        if (!deviceList.update()) {
            fprintf(stderr, "%s: error: could not retrieve device list, "
                "do you have an internet connection?\n", gProgramName.c_str());
            return EXIT_FAILURE;
        }
    } else {
        deviceList.getLocal();
    }

    size_t total = deviceList.size();

    fprintf(stdout, "There are currently %zu supported devices:\n", total);

    for (size_t i = 0; i < total; ++i) {
        alignPrintNumber(i);
        fputs(deviceList[i].c_str(), stdout);
        if (deviceList[i] == BACON_PSEUDO_RANDOM_DEVICE_ID)
            fputs("    -> (chooses a device at random)", stdout);
        else if (deviceList[i] == BACON_PSEUDO_ALL_DEVICE_ID)
            fputs("       -> (performs action on all devices)", stdout);
        fputc('\n', stdout);
    }
    return EXIT_SUCCESS;
}

int updateDeviceList()
{
    DeviceList deviceList;
    vector<string> oldList;

    if (deviceList.exists()) {
        deviceList.getLocal();
        oldList = deviceList.rawList();
    }

    string lastUpdated(deviceList.lastUpdate());

    fputs("Updating device list...", stdout);
    if (!lastUpdated.empty())
        fprintf(stdout, " (last updated %s)", lastUpdated.c_str());
    fputc('\n', stdout);

    if (!deviceList.update()) {
        fprintf(stderr, "%s: error: could not retrieve devices list, "
                "do you have an internet connection?\n",
                gProgramName.c_str());
        return EXIT_FAILURE;
    }

    ListCompareData cmp(deviceList, oldList);
    cmp.printDiff();

    fputs("Finished\n", stdout);
    return EXIT_SUCCESS;
}

int showAllRoms(const vector<Device *> &devices)
{
    return perform(SHOWONLY, devices);
}

int downloadLatestStableRom(const vector<Device *> &devices)
{
    return perform(DOWNLOAD_ST, devices);
}

int downloadLatestNightlyRom(const vector<Device *> &devices)
{
    return perform(DOWNLOAD_NI, devices);
}

int downloadLatestRcRom(const vector<Device *> &devices)
{
    return perform(DOWNLOAD_RC, devices);
}

} /* namespace bacon */

