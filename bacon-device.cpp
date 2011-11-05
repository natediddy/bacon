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

#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-file.h"

using std::string;

namespace bacon
{
  Device::Device(const string & device_id)
    : mId(device_id)
  {}

  Device::~Device()
  {}

  string Device::id() const
  {
    return mId;
  }

  string Device::romDir() const
  {
    string p[] = {
      env::appDir(), mId, ""
    };
    return env::pathJoin(p);
  }

  void Device::createRomDir() const
  {
    File myDir(romDir());
    myDir.makeDir();
  }
}

