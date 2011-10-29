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

#define LIST_CONFIG_FILENAME "device_list"

using std::string;
using std::vector;

namespace
{
  string realName()
  {
    string name;

#ifdef _WIN32
    name = LIST_CONFIG_FILENAME;
    name += ".txt";
#else
    name = LIST_CONFIG_FILENAME;
#endif
    return name;
  }

  void makeHidden(string * path)
  {
#ifdef _WIN32
    SetFileAttributes((*path).c_str(), FILE_ATTRIBUTE_HIDDEN);
#else
    string _path = ".";

    _path += (*path);
    (*path) = _path;
#endif
  }

  string prepareListFile()
  {
    string name = realName();

    makeHidden(&name);
    string p[] = {
      bacon::env::appDir(), name, ""
    };
    return bacon::env::pathJoin(p);
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
    HtmlDoc doc;
    HtmlParser * parser = 0;

    if (!mDeviceIds.empty()) {
      mDeviceIds.clear();
    }

    if (doc.fetch()) {
      parser = new HtmlParser(doc.content());
      if (parser) {
        parser->allDeviceIds(mDeviceIds);
        if (mDeviceIds.size()) {
          if (exists()) {
            dispose();
          }
          if (open("w+")) {
            writeDateLine();
            for (vector<string>::size_type i=0; i < mDeviceIds.size(); i++) {
              writeLine(mDeviceIds[i]);
            }
            close();
          }
        }
        delete parser;
        parser = 0;
        return true;
      }
    }
    return false;
  }

  void DeviceList::getLocal()
  {
    if (open("r")) {
      for (string l = readLine(); !l.empty(); l = readLine()) {
        mDeviceIds.push_back(l);
      }
      close();
    }
  }

  size_t DeviceList::size() const
  {
    return mDeviceIds.size();
  }

  bool DeviceList::hasMatch(const string & device_id) const
  {
    for (vector<string>::size_type i = 0; i < mDeviceIds.size(); i++) {
      if (device_id == mDeviceIds[i]) {
        return true;
      }
    }
    return false;
  }

  string DeviceList::operator[](const size_t & index) const
  {
    return mDeviceIds[index];
  }

  void DeviceList::writeLine(const string & line)
  {
    if (File::mStream) {
      const char * real = line.c_str();
      for (size_t i = 0; real[i]; i++) {
        if (fputc(real[i], File::mStream) == EOF) {
          return;
        }
      }
      if (fputc('\n', File::mStream) == EOF) {
        return;
      }
    }
  }

  string DeviceList::readLine()
  {
    string line = "";
    bool inDateLine = false;

    if (File::mStream) {
      int ch;
      while ((ch = fgetc(File::mStream)) != EOF) {
        if (ch == (int)'#') {
          if (!inDateLine) {
            inDateLine = true;
          }
        }
        if (ch == (int)'\n') {
          if (inDateLine) {
            inDateLine = false;
            continue;
          }
          break;
        }
        if (!inDateLine) {
          line += (char)ch;
        }
      }
    }
    return line;
  }

  void DeviceList::writeDateLine()
  {
    char buf[128];
    time_t now;
    struct tm * timeBuf = 0;
    string dateString = "#";

    time(&now);
    timeBuf = localtime(&now);
    strftime(buf, 128, "%A, %B %d, %I:%M%p", timeBuf);
    dateString += buf;

    if (!dateString.empty()) {
      writeLine(dateString);
    }
  }

  vector<string> DeviceList::rawList() const
  {
    return mDeviceIds;
  }

  string DeviceList::lastUpdate()
  {
    string result = "";
    bool inDateLine = false;

    if (open("r")) {
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

