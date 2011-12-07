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

#ifndef BACON_HTMLPARSER_H_INCLUDED
#define BACON_HTMLPARSER_H_INCLUDED

#include <string>
#include <vector>

#include "bacon.h"

BACON_NAMESPACE_BEGIN

class HtmlParser {
public:
    HtmlParser(const std::string &content);
    ~HtmlParser();
    void allDeviceIds(std::vector<std::string> &vec) const;
    std::string checksumStringForFile(const std::string &filename) const;
    std::vector<std::string> latestRomsForDevice() const;
    std::string currentContent() const;
private:
    std::string mContent;
};

BACON_NAMESPACE_END

#endif /* !BACON_HTMLPARSER_H_INCLUDED */

