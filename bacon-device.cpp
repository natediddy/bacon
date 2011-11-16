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
#include "bacon-devicelist.h"
#include "bacon-env.h"
#include "bacon-file.h"
#include "bacon-util.h"

using std::string;

namespace
{
  string getRandomDeviceIndex()
  {
    bacon::DeviceList list;
    unsigned int index;

    list.getLocal();
    bacon::util::randomSeed();
    index = bacon::util::random();
    while (index >= (list.size() - 1)) {
      index = bacon::util::random() / 10000;
    }
    return list[index];
  }
}

namespace bacon
{
  Device::Device(const string & device_id)
    : mId(device_id)
  {
    if (mId == PSEUDO_RANDOM_DEVICE_ID) {
      mId = getRandomDeviceIndex();
    }
  }

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

