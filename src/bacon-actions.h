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

#ifndef BACON_ACTIONS_H_INCLUDED
#define BACON_ACTIONS_H_INCLUDED

#include <vector>

namespace bacon
{
  int showUsage();
  int showHelp();
  int showVersion();
  int showDevices();
  int updateDeviceList();

  class Device;

  int showAllRoms(const std::vector<Device *> &devices);
  int downloadLatestStableRom(const std::vector<Device *> &devices);
  int downloadLatestNightlyRom(const std::vector<Device *> &devices);
  int downloadLatestRcRom(const std::vector<Device *> &devices);
}

#endif /* !BACON_ACTIONS_H_INCLUDED */

