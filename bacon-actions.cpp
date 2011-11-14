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
#include "bacon-md5.h"
#include "bacon-rom.h"
#include "bacon-util.h"

using std::string;
using std::vector;

extern string gProgramName;

namespace
{
  using namespace bacon;

  bool validDevice(const string & id)
  {
    DeviceList deviceList;

    if (!deviceList.exists()) {
      deviceList.update();
    } else {
      deviceList.getLocal();
    }
    return deviceList.hasMatch(id);
  }

  bool romExists(const string & path,
                 const string & deviceId,
                 const string & deviceType)
  {
    File fp(path);
    bool ret = false;

    if (fp.isFile()) {
      Md5 md5(path, deviceId, deviceType);
      if (md5.verify()) {
        ret = true;
      }
    }
    return ret;
  }

  enum Action {
    SHOWONLY,
    DOWNLOAD_ST,
    DOWNLOAD_NI,
    DOWNLOAD_RC
  };

  void downloadAction(bool * hasError,
                      const Action & action,
                      const Device * device)
  {
    if (action != SHOWONLY) {
      string type;
      switch (action) {
      case DOWNLOAD_ST:
        type = "stable";
        break;
      case DOWNLOAD_NI:
        type = "nightly";
        break;
      case DOWNLOAD_RC:
        type = "rc";
        break;
      }
      HtmlDoc *doc = new HtmlDoc(device->id(), type);
      if (doc->fetch()) {
        HtmlParser parser(doc->content());
        string romName = parser.latestRomForDevice(device->id());
        delete doc;
        doc = 0;
        if (!romName.empty()) {
          string p[] = {
            device->romDir(), romName, ""
          };
          string romPath = env::pathJoin(p);
          if (!romExists(romPath, device->id(), type)) {
            Rom *rom = new Rom(romName, romPath);
            if (rom->fetch()) {
              fputs("\nVerifying file integrity...", stdout);
              Md5 *md5 = new Md5(romPath, device->id(), type);
              if (!md5->verify()) {
                fputs(" FAIL\n", stdout);
                fprintf(stderr, "%s: `%s' may be corrupt!\n",
                    romPath.c_str(), gProgramName.c_str());
              } else {
                fputs(" PASS\n", stdout);
                fprintf(stdout, "New ROM located at:\n\t%s\n",
                    romPath.c_str());
              }
              if (md5) {
                delete md5;
                md5 = 0;
              }
            } else {
              fprintf(stderr, "\n%s: error: failed to fetch %s\n",
                  gProgramName.c_str(), romName.c_str());
              if (!*hasError) {
                *hasError = true;
              }
            }
            if (rom) {
              delete rom;
              rom = 0;
            }
          } else {
            fprintf(stdout, "%s: `%s' exists and has already been "
                "downloaded\n", gProgramName.c_str(), romName.c_str());
          }
        }
      }
    } else {
      string types[3] = {
        "stable", "nightly", "rc"
      };
      fprintf(stdout, "%s:\n", device->id().c_str());
      for (size_t i = 0; i < 3; i++) {
        HtmlDoc *doc = new HtmlDoc(device->id(), types[i]);
        if (doc->fetch()) {
          HtmlParser parser(doc->content());
          string romName = parser.latestRomForDevice(device->id());
          fprintf(stdout, "  %s: %s\n", types[i].c_str(),
              romName.empty() ? "(not found)" : romName.c_str());
        }
      }
    }
  }

  int performAction(const Action & action,
                    const vector<Device *> & devices)
  {
    bool hasError = false;

    if (!devices.size()) {
      fprintf(stderr, "%s: error: no device(s) given\n",
          gProgramName.c_str());
      return EXIT_FAILURE;
    }

    for (vector<Device *>::size_type i = 0; i < devices.size(); i++) {
      if (!validDevice(devices[i]->id())) {
        fprintf(stderr, "%s: warning: unknown device `%s', skipping...\n",
            gProgramName.c_str(),
            util::toUpperCase(devices[i]->id()).c_str());
        continue;
      } else {
        devices[i]->createRomDir();
      }
      if (action == SHOWONLY) {
        fputs("Showing ", stdout);
      } else {
        fputs("Downloading ", stdout);
      }
      fputs("latest ", stdout);
      if (action == DOWNLOAD_ST) {
        fputs("Stable ", stdout);
      } else if (action == DOWNLOAD_NI) {
        fputs("Nightly ", stdout);
      } else if (action == DOWNLOAD_RC) {
        fputs("Release Candidate ", stdout);
      }
      fprintf(stdout, "%s for %s...\n",
          (action == SHOWONLY) ? "ROMs" : "ROM",
          util::toUpperCase(devices[i]->id()).c_str());
      downloadAction(&hasError, action, devices[i]);
    }

    if (hasError) {
      return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
  }
}

namespace bacon
{
  int showUsage()
  {
    fprintf(stdout, "Usage: %s [-S|-s|-n|-r|-d|-h|-v] DEVICE ...\n",
        gProgramName.c_str());
    return EXIT_FAILURE;
  }

  int showHelp()
  {
    showUsage();
    fprintf(stdout,
       "\nOptions:\n"
       "  -S, --show               show latest builds for DEVICE(s), "
         "do not download\n"
       "  -s, --stable             download latest stable build for "
         "DEVICE(s)\n"
       "  -n, --nightly            download latest nightly build for "
         "DEVICE(s)\n"
       "  -r, --release-candidate  download latest release candidate for "
         "DEVICE(s)\n"
       "  -d, --devices            display all available DEVICEs and exit\n"
       "  -u, --update-devices     update the current DEVICE list and exit\n"
       "  -h, -?, --help           display this message and exit\n"
       "  -v, --version            display version information and exit\n\n"
       "More than 1 DEVICE can be given: but only 1 option per DEVICE-list\n")
      ;
    return EXIT_SUCCESS;
  }

  int showVersion()
  {
    string verStr = PACKAGE_VERSION;

    fprintf(stdout, "%s %s\n"
       "Copyright (C) 2011 Nathan Forbes\n"
       "This is free software; see the source for copying conditions.\n"
       "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
       "PARTICULAR PURPOSE.\n",
       PACKAGE_NAME, verStr.c_str());
    return EXIT_SUCCESS;
  }

  int showDevices()
  {
    DeviceList deviceList;
    size_t total;

    if (!deviceList.exists()) {
      if (!deviceList.update()) {
        fprintf(stderr, "%s: error: could not retrieve device list, "
            "do you have an internet connection?\n", gProgramName.c_str());
        return EXIT_FAILURE;
      }
    } else {
      deviceList.getLocal();
    }

    total = deviceList.size();
    fprintf(stdout, "There are currently %zu supported devices:\n", total);

    for (size_t i = 0; i < total; i++) {
      if ((i + 1) < 10) {
        fprintf(stdout, "   ");
      } else if ((i + 1) < 100) {
        fprintf(stdout, "  ");
      } else if ((i + 1) < 1000) {
        fprintf(stdout, " ");
      }
      fprintf(stdout, "%zu)  %s\n", i + 1, deviceList[i].c_str());
    }
    return EXIT_SUCCESS;
  }

  int updateDeviceList()
  {
    DeviceList deviceList;
    vector<string> oldList;
    vector<string> newList;
    string lastUpdated;
    int newCount = 0;

    if (deviceList.exists()) {
      deviceList.getLocal();
      oldList = deviceList.rawList();
    }

    lastUpdated = deviceList.lastUpdate();
    fputs("Updating device list...", stdout);
    if (!lastUpdated.empty()) {
      fprintf(stdout, " (last updated %s)", lastUpdated.c_str());
    }

    fputc('\n', stdout);
    if (!deviceList.update()) {
      fprintf(stderr, "%s: error: could not retrieve devices list, "
          "do you have an internet connection?\n", gProgramName.c_str());
      return EXIT_FAILURE;
    }

    newList = deviceList.rawList();
    newCount = (int)newList.size() - (int)oldList.size();

    if (newCount) {
      fprintf(stdout, "There %s %i new %s:\n",
          (newCount > 1) ? "are" : "is", newCount,
          (newCount > 1) ? "devices" : "device");
      for (vector<string>::size_type i = 0; i < newList.size(); i++) {
        bool noMatch = true;
        for (vector<string>::size_type j = 0; j < oldList.size(); j++) {
          if (newList[i] == oldList[j]) {
            noMatch = false;
            break;
          }
        }
        if (noMatch) {
          fprintf(stdout, "%s\n", newList[i].c_str());
        }
      }
    } else {
      fputs("No new devices since last update\n", stdout);
    }
    fputs("Finished\n", stdout);
    return EXIT_SUCCESS;
  }

  int showAllRoms(const vector<Device *> & devices)
  {
    return performAction(SHOWONLY, devices);
  }

  int downloadLatestStableRom(const vector<Device *> & devices)
  {
    return performAction(DOWNLOAD_ST, devices);
  }

  int downloadLatestNightlyRom(const vector<Device *> & devices)
  {
    return performAction(DOWNLOAD_NI, devices);
  }

  int downloadLatestRcRom(const vector<Device *> & devices)
  {
    return performAction(DOWNLOAD_RC, devices);
  }
}

