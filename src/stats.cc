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
#include "env.h"
#include "file.h"
#include "htmldoc.h"
#include "htmlparser.h"
#include "md5.h"
#include "stats.h"
#include "util.h"

using std::string;
using std::vector;

namespace bacon {

Stats::Stats(const Device *device, const string &deviceType)
    : mDevice(device)
    , mType(deviceType)
{}

Stats::~Stats()
{}

bool Stats::init()
{
    HtmlDoc doc(mDevice->id(), mType);

    if (doc.fetch()) {
        HtmlParser parser(doc.content());
        mLatestRomNames = parser.latestRomsForDevice();
        for (size_t i = 0; i < mLatestRomNames.size(); ++i) {
            string p[] = {
                mDevice->romDir(), mLatestRomNames[i], ""
            };
            mRomPaths.push_back(env::pathJoin(p));
        }
    }
    return mLatestRomNames.size() && mRomPaths.size();
}

bool Stats::existsLocally(const size_t n) const
{
    if (n >= mRomPaths.size())
        return false;

    File p(mRomPaths[n]);
    return p.isFile();
}

bool Stats::isValid(const size_t n) const
{
    if (n >= mRomPaths.size())
        return false;

    Md5 md5(mRomPaths[n], mDevice->id(), mType);
    return md5.verify();
}

vector<string> Stats::romNames() const
{
    return mLatestRomNames;
}

} /* namespace bacon */

