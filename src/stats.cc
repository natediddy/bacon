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

BACON_NAMESPACE_BEGIN

using std::map;
using std::string;
using std::vector;

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
        vector<string> names = parser.latestRomsForDevice();
        for (size_t i = 0; i < names.size(); ++i) {
            vector<string> nfo;
            nfo.push_back(parser.sizeForFile(names[i]));
            nfo.push_back(parser.dateForFile(names[i]));
            string p[] = {
                mDevice->romDir(), names[i], ""
            };
            nfo.push_back(env::pathJoin(p));
            mInfo[names[i]] = nfo;
        }
        return true;
    }
    return false;
}

bool Stats::existsLocally(const string &name) const
{
    for (map<string, vector<string> >::const_iterator i = mInfo.begin();
         i != mInfo.end();
         ++i)
    {
        if (i->first != name)
            continue;
        File p(i->second[2]);
        return p.isFile();
    }
    return false;
}

bool Stats::isValid(const string &name) const
{
    string path("");

    for (map<string, vector<string> >::const_iterator i = mInfo.begin();
         i != mInfo.end();
         ++i)
    {
        if (i->first == name) {
            path = i->second[2];
            break;
        }
    }

    if (!path.empty()) {
        Md5 md5(path, mDevice->id(), mType);
        return md5.verify();
    }
    return false;
}

map<string, vector<string> > Stats::romInfo() const
{
    return mInfo;
}

BACON_NAMESPACE_END

