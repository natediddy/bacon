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
#include "bacon-htmldoc.h"
#include "bacon-htmlparser.h"
#include "bacon-md5.h"
#include "bacon-stats.h"
#include "bacon-util.h"

using std::string;

namespace bacon
{
  Stats::Stats(const Device *device, const string &deviceType)
    : mDevice(device)
    , mType(deviceType)
    , mLatestRomName("")
    , mRomPath("")
  {}

  Stats::~Stats()
  {}

  bool Stats::init()
  {
    HtmlDoc doc(mDevice->id(), mType);

    if (doc.fetch())
    {
      HtmlParser parser(doc.content());
      mLatestRomName = parser.latestRomForDevice();
      string p[] = {
        mDevice->romDir(), mLatestRomName, ""
      };
      mRomPath = env::pathJoin(p);
    }
    return !mLatestRomName.empty() && !mRomPath.empty();
  }

  bool Stats::existsLocally() const
  {
    File p(mRomPath);

    return p.isFile();
  }

  bool Stats::isValid() const
  {
    Md5 md5(mRomPath, mDevice->id(), mType);

    return md5.verify();
  }

  string Stats::romName() const
  {
    return mLatestRomName;
  }
} /* namespace bacon */

