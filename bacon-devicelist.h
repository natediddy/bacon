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

#ifndef BACON_DEVICELIST_H_INCLUDED
#define BACON_DEVICELIST_H_INCLUDED

#include <string>
#include <vector>

#include "bacon-file.h"

namespace bacon
{
  class DeviceList : public File {
  public:
    DeviceList();
    ~DeviceList();

    bool update();
    void getLocal();
    bool hasMatch(const std::string &) const;
    size_t size() const;
    std::string operator[](const size_t &) const;
    std::vector<std::string> rawList() const;
    std::string lastUpdate();

  private:
    void writeLine(const std::string &);
    std::string readLine();
    void writeDateLine();

  private:
    std::vector<std::string> mDeviceIds;
  };
}

#endif /* !BACON_DEVICELIST_H_INCLUDED */

