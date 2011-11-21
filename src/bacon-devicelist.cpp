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

#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

#include "bacon-devicelist.h"
#include "bacon-env.h"
#include "bacon-htmldoc.h"
#include "bacon-htmlparser.h"
#include "bacon-prefs.h"
#include "bacon-util.h"

#define LIST_CONFIG_FILENAME "device_list"

#define APPEND_RANDOM_ID \
  mDeviceIds.push_back(string(PSEUDO_RANDOM_DEVICE_ID))

#define DATE_LINE_FORMAT "#%A, %B %d, %I:%M%p"

using std::string;
using std::vector;

namespace
{
  string prepareListFile()
  {
    string name(LIST_CONFIG_FILENAME);

#ifndef _WIN32
    string _name(".");
    _name += name;
    name = _name;
#else
    name += ".txt";
#endif

    string p[] = {
      bacon::prefs::get(KEY_BASE_DIR), name, ""
    };
    string path(bacon::env::pathJoin(p));

#ifdef _WIN32
    SetFileAttributes(path.c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif

    return path;
  }
}

namespace bacon
{
  DeviceList::DeviceList()
    : File(prepareListFile())
  {}

  DeviceList::~DeviceList()
  {}

  bool DeviceList::update()
  {
    if (!mDeviceIds.empty()) {
      mDeviceIds.clear();
    }

    HtmlDoc doc;

    if (doc.fetch()) {
      HtmlParser *parser = new HtmlParser(doc.content());

      if (parser) {
        parser->allDeviceIds(mDeviceIds);
 
        if (mDeviceIds.size()) {

          if (exists()) {
            dispose();
          }

          if (open("w+")) {
            writeDateLine();

            for (size_t i = 0; i < mDeviceIds.size(); i++) {
              if (mDeviceIds[i] == PSEUDO_RANDOM_DEVICE_ID) {
                continue;
              }
              writeLine(mDeviceIds[i]);
            }
            close();
          }
        }

        delete parser;
        parser = NULL;
        APPEND_RANDOM_ID;
        return true;
      }
    }
    return false;
  }

  void DeviceList::getLocal()
  {
    if (open()) {
      for (string l = readLine(); !l.empty(); l = readLine()) {
        mDeviceIds.push_back(l);
      }
      close();
    }

    APPEND_RANDOM_ID;
  }

  size_t DeviceList::size() const
  {
    return mDeviceIds.size();
  }

  bool DeviceList::hasMatch(const string &deviceId) const
  {
    for (size_t i = 0; i < mDeviceIds.size(); i++) {
      if (deviceId == mDeviceIds[i]) {
        return true;
      }
    }
    return false;
  }

  string DeviceList::operator[](const size_t index) const
  {
    return mDeviceIds[index];
  }

  void DeviceList::writeDateLine()
  {
    writeLine(string(util::timeString(DATE_LINE_FORMAT)));
  }

  vector<string> DeviceList::rawList() const
  {
    return mDeviceIds;
  }

  string DeviceList::lastUpdate()
  {
    string result("");
    bool inDateLine = false;

    if (open()) {
      int ch;

      while ((ch = fgetc(File::mStream)) != EOF) {

        if (ch == (int)'#') {
          if (!inDateLine) {
            inDateLine = true;
            continue;
          }
        }

        if (ch == (int)'\n') {
          break;
        } else if (inDateLine) {
          result += (char)ch;
        }
      }

      close();
    }

    return result;
  }
}

