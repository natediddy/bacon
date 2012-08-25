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

#ifndef BACON_STATS_H_INCLUDED
#define BACON_STATS_H_INCLUDED

#include <map>
#include <string>
#include <vector>

#include "bacon.h"

BACON_NAMESPACE_BEGIN

class Device;

class Stats {
public:
    Stats(const Device *device, const std::string &deviceType);
    ~Stats();
    bool init();
    bool existsLocally(const std::string &name) const;
    bool isValid(const std::string &name) const;
    std::map<std::string, std::vector<std::string> > romInfo() const;
private:
    const Device *mDevice;
    std::string mType;

    /* key=ROMNAME, value=[SIZE,DATE,PATH] */
    std::map<std::string, std::vector<std::string> > mInfo;
};

BACON_NAMESPACE_END

#endif /* !BACON_STATS_H_INCLUDED */

