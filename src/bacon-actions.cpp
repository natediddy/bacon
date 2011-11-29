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

#ifdef _WIN32
#include <conio.h>
#endif

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

namespace bacon
{
  namespace
  {
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

      if (!deviceList.exists())
        deviceList.update();
      else
        deviceList.getLocal();
      return deviceList.hasMatch(id);
    }

    bool romExists(const string &path,
                   const string &deviceId,
                   const string &deviceType)
    {
      File fp(path);
      bool ret = false;

      if (fp.isFile())
      {
        Md5 md5(path, deviceId, deviceType);
        ret = md5.verify();
      }
      else if (fp.isDir())
        LOGW("`%s' is a directory, not a file!", fp.name().c_str());
      return ret;
    }

    enum Action {
      SHOWONLY,
      DOWNLOAD_ST,
      DOWNLOAD_NI,
      DOWNLOAD_RC
    };

    void downloadAction(bool *hasError,
                        const Action &action,
                        const Device *device)
    {
      if (action != SHOWONLY)
      {
        string type;
        switch (action)
        {
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
        if (doc->fetch())
        {
          HtmlParser parser(doc->content());
          string romName = parser.latestRomForDevice();
          delete doc;
          doc = NULL;
          if (!romName.empty())
          {
            string p[] = {
              device->romDir(), romName, ""
            };
            string romPath = env::pathJoin(p);
            if (!romExists(romPath, device->id(), type))
            {
              Rom rom(romName, romPath);
              if (rom.fetch())
              {
                fputs("\nVerifying file integrity...", stdout);
                Md5 md5(romPath, device->id(), type);
                if (!md5.verify())
                {
                  fputs(" FAIL\n", stdout);
                  fprintf(stderr, "%s: `%s' may be corrupt!\n",
                      gProgramName.c_str(), romPath.c_str());
                }
                else
                {
                  fputs(" PASS\n", stdout);
                  fprintf(stdout, "New ROM located at:\n\t%s\n",
                      romPath.c_str());
                }
              }
              else
              {
                fprintf(stderr, "\n%s: error: failed to fetch %s\n",
                    gProgramName.c_str(), romName.c_str());
                if (!*hasError)
                  *hasError = true;
              }
            }
            else
              fprintf(stdout, "%s: `%s' exists and has already been "
                  "downloaded\n", gProgramName.c_str(), romName.c_str());
          }
        }
        else if (doc)
        {
          delete doc;
          doc = NULL;
          if (!*hasError)
            *hasError = true;
        }
      }
      else
      {
        string types[3] = {
          "stable", "nightly", "rc"
        };
        fprintf(stdout, "%s:\n", device->id().c_str());
        for (size_t i = 0; i < 3; ++i)
        {
          Stats stats(device, types[i]);
          fprintf(stdout, "  %s:", types[i].c_str());
          for (size_t j = 0; j < (7 - types[i].size() + 2); ++j)
            fputc(' ', stdout);
          if (stats.init())
          {
            fprintf(stdout, "%s\n", stats.romName().c_str());
            for (size_t j = 0;
                 j < (12 - types[i].size() + types[i].size());
                 ++j)
              fputc(' ', stdout);
            fputs("-- ", stdout);
            if (stats.existsLocally())
            {
              if (stats.isValid())
                fputs("already downloaded", stdout);
              else
                fputs("partially downloaded or corrupt", stdout);
            }
            else
              fputs("not downloaded", stdout);
            fputc('\n', stdout);
          }
          else
            fputs("(not found)\n", stdout);
        }
      }
    }

    int performAction(const Action &action,
                      const vector<Device *> &devices)
    {
      bool hasError = false;

      if (!devices.size())
      {
        fprintf(stderr, "%s: error: no device(s) given\n",
            gProgramName.c_str());
        return EXIT_FAILURE;
      }

      for (size_t i = 0; i < devices.size(); i++)
      {
        if (!validDevice(devices[i]->id()))
        {
          fprintf(stderr, "%s: warning: unknown device `%s', skipping...\n",
              gProgramName.c_str(),
              util::toUpperCase(devices[i]->id()).c_str());
          continue;
        }
        else if (action != SHOWONLY)
          devices[i]->createRomDir();
        if (action == SHOWONLY)
          fputs("Showing ", stdout);
        else
          fputs("Downloading ", stdout);
        fputs("latest ", stdout);
        if (action == DOWNLOAD_ST)
          fputs("Stable ", stdout);
        else if (action == DOWNLOAD_NI)
          fputs("Nightly ", stdout);
        else if (action == DOWNLOAD_RC)
          fputs("Release Candidate ", stdout);
        fprintf(stdout, "%s for %s...\n",
            (action == SHOWONLY) ? "ROMs" : "ROM",
            util::toUpperCase(devices[i]->id()).c_str());
        downloadAction(&hasError, action, devices[i]);
      }

      if (hasError)
        return EXIT_FAILURE;
      return EXIT_SUCCESS;
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
       "Copyright (C) 2011 Nathan Forbes\n"
       "This is free software; see the source for copying conditions.\n"
       "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n"
       "PARTICULAR PURPOSE.\n",
       PACKAGE_NAME, PACKAGE_VERSION);

    return EXIT_SUCCESS;
  }

  int showDevices()
  {
    DeviceList deviceList;

    if (!deviceList.exists())
    {
      if (!deviceList.update())
      {
        fprintf(stderr, "%s: error: could not retrieve device list, "
            "do you have an internet connection?\n", gProgramName.c_str());
        return EXIT_FAILURE;
      }
    }
    else
      deviceList.getLocal();

    size_t total = deviceList.size();

    fprintf(stdout, "There are currently %zu supported devices:\n", total);

    for (size_t i = 0; i < total; ++i)
    {
      alignPrintNumber(i);
      fputs(deviceList[i].c_str(), stdout);
      if (deviceList[i] == PSEUDO_RANDOM_DEVICE_ID)
        fputs("    -> (chooses a device at random)", stdout);
      fputc('\n', stdout);
    }
    return EXIT_SUCCESS;
  }

  int updateDeviceList()
  {
    DeviceList deviceList;
    vector<string> oldList;

    if (deviceList.exists())
    {
      deviceList.getLocal();
      oldList = deviceList.rawList();
    }

    string lastUpdated(deviceList.lastUpdate());
    fputs("Updating device list...", stdout);

    if (!lastUpdated.empty())
      fprintf(stdout, " (last updated %s)", lastUpdated.c_str());

    fputc('\n', stdout);

    if (!deviceList.update())
    {
      fprintf(stderr, "%s: error: could not retrieve devices list, "
          "do you have an internet connection?\n", gProgramName.c_str());
      return EXIT_FAILURE;
    }

    vector<string> newList = deviceList.rawList();
    int newCount = (int)newList.size() - (int)oldList.size();

    if (newCount)
    {
      /* Use proper grammer... */
      fputs("There ", stdout);
      if (newCount > 1)
        fputs("are ", stdout);
      else
        fputs("is ", stdout);
      fprintf(stdout, "%d new device", newCount);
      if (newCount > 1)
        fputc('s', stdout);
      fputs(":\n", stdout);
      for (size_t i = 0, n = 0; i < newList.size(); ++i)
      {
        bool noMatch = true;
        for (size_t j = 0; j < oldList.size(); ++j)
        {
          if (newList[i] == oldList[j])
          {
            noMatch = false;
            break;
          }
        }
        if (noMatch)
        {
          alignPrintNumber(n++);
          fputs(newList[i].c_str(), stdout);
        }
      }
    }
    else
      fputs("No new devices since last update", stdout);

    fputs("\nFinished\n", stdout);
    return EXIT_SUCCESS;
  }

  int showAllRoms(const vector<Device *> &devices)
  {
    return performAction(SHOWONLY, devices);
  }

  int downloadLatestStableRom(const vector<Device *> &devices)
  {
    return performAction(DOWNLOAD_ST, devices);
  }

  int downloadLatestNightlyRom(const vector<Device *> &devices)
  {
    return performAction(DOWNLOAD_NI, devices);
  }

  int downloadLatestRcRom(const vector<Device *> &devices)
  {
    return performAction(DOWNLOAD_RC, devices);
  }
} /* namespace bacon */

