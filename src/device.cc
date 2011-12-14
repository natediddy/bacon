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

#include "device.h"
#include "devicelist.h"
#include "env.h"
#include "file.h"
#include "prefs.h"
#include "util.h"

BACON_NAMESPACE_BEGIN

using std::string;

BACON_PRIVATE_NAMESPACE_BEGIN

string getRandomDeviceIndex()
{
    DeviceList list;

    list.getLocal();
    util::randomSeed();

    unsigned int index = util::random();

    while (index >= list.size()) {
        index = util::random() / 10000;
        if (index < list.size() &&
            (list[index] == BACON_PSEUDO_ALL_DEVICE_ID ||
             list[index] == BACON_PSEUDO_RANDOM_DEVICE_ID))
        {
            index = list.size();
            continue;
        }
    }
    return list[index];
}

BACON_PRIVATE_NAMESPACE_END

Device::Device(const string &deviceId)
    : mId(deviceId)
{
    if (mId == BACON_PSEUDO_RANDOM_DEVICE_ID)
        mId = getRandomDeviceIndex();
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
        prefs::get(BACON_PREFS_KEY_BASE_DIR), mId, ""
    };

    return env::pathJoin(p);
}

void Device::createRomDir() const
{
    File myDir(romDir());

    myDir.makeDir();
}

BACON_NAMESPACE_END

