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

#include "file.h"

#define BACON_PSEUDO_ALL_DEVICE_ID    "all"
#define BACON_PSEUDO_RANDOM_DEVICE_ID "random"

namespace bacon {

class DeviceList : public File {
public:
    DeviceList();
    ~DeviceList();

    bool update();
    void getLocal();
    void prep();
    bool hasMatch(const std::string &id) const;
    size_t size() const;
    std::string operator[](const size_t index) const;
    std::vector<std::string> rawList() const;
    std::string lastUpdate();

private:
    void writeDateLine();

private:
    std::vector<std::string> mDeviceIds;
};

} /* namespace bacon */

#endif /* !BACON_DEVICELIST_H_INCLUDED */

