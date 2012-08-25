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

#include "htmldoc.h"
#include "htmlparser.h"

/* '<li id="device_' */
#define DEVICE_BULLET(s, i) \
    s[i]       == '<' && s[i+1]  == 'l' && s[i+2]  == 'i' && s[i+3]  == ' ' \
    && s[i+4]  == 'i' && s[i+5]  == 'd' && s[i+6]  == '=' && s[i+7]  == '"' \
    && s[i+8]  == 'd' && s[i+9]  == 'e' && s[i+10] == 'v' && s[i+11] == 'i' \
    && s[i+12] == 'c' && s[i+13] == 'e' && s[i+14] == '_'
#define DEVICE_BULLET_SIZE 15

/* '/get/' */
#define GET_URL(s, i) \
    s[i] == '/' && s[i+1] == 'g' && s[i+2] == 'e' && s[i+3] == 't' \
    && s[i+4] == '/'
#define GET_URL_SIZE 5

/* 'md5sum: ' */
#define MD5_CLASS(s, i) \
    s[i]      == 'm' && s[i+1] == 'd' && s[i+2] == '5' && s[i+3] == 's' \
    && s[i+4] == 'u' && s[i+5] == 'm' && s[i+6] == ':' && s[i+7] == ' '
#define MD5_CLASS_SIZE 8

/* '<td>' */
#define TD_NEW(s, i) \
    s[i] == '<' && s[i+1] == 't' && s[i+2] == 'd' && s[i+3] == '>'
#define TD_NEW_SIZE 4

BACON_NAMESPACE_BEGIN

using std::string;
using std::vector;

extern int gRomHistory;

BACON_PRIVATE_NAMESPACE_BEGIN

bool filenameMatch(const string &s, const string &n, const size_t i)
{
    for (size_t j = 0, k = i; j < n.size(); ++j, ++k) {
        if (s[k] != n[j])
            return false;
    }
    return true;
}

BACON_PRIVATE_NAMESPACE_END

HtmlParser::HtmlParser(const string &content)
    : mContent(content)
{}

HtmlParser::~HtmlParser()
{}

void HtmlParser::allDeviceIds(vector<string> &vec) const
{
    for (size_t i = 0; i < mContent.size(); ++i) {
        if (DEVICE_BULLET(mContent, i)) {
            string tmp("");
            i += DEVICE_BULLET_SIZE;
            while (mContent[i] != '"')
                tmp += mContent[i++];
            if (!tmp.empty())
                vec.push_back(tmp);
        }
    }
}

string HtmlParser::checksumStringForFile(const string &filename) const
{
    string hash("");

    for (size_t i = 0; i < mContent.size(); ++i) {
        if (filenameMatch(mContent, filename, i)) {
            i += filename.size();
            while (true) {
                if (MD5_CLASS(mContent, i)) {
                    i += MD5_CLASS_SIZE;
                    while (mContent[i] != ' ')
                        hash += mContent[i++];
                    break;
                }
                i++;
            }
        }
    }
    return hash;
}

string HtmlParser::sizeForFile(const string &filename) const
{
    string size("");
    bool found = false;

    for (size_t i = 0; i < mContent.size(); ++i) {
        if (filenameMatch(mContent, filename, i)) {
            i += filename.size();
            while (true) {
                if (MD5_CLASS(mContent, i))
                    i += MD5_CLASS_SIZE;
                if (TD_NEW(mContent, i)) {
                    i += TD_NEW_SIZE;
                    found = true;
                    break;
                }
                i++;
            }
            if (found) {
                while (mContent[i] != '<')
                    size += mContent[i];
                break;
            }
        }
    }
    return size;
}

string HtmlParser::dateForFile(const string &filename) const
{
    string date("");
    bool found = false;

    for (size_t i = 0; i < mContent.size(); ++i) {
        if (filenameMatch(mContent, filename, i)) {
            i += filename.size();
            while (true) {
                if (MD5_CLASS(mContent, i))
                    i += MD5_CLASS_SIZE;
                if (TD_NEW(mContent, i)) {
                    i += TD_NEW_SIZE;
                    while (true) {
                        if (TD_NEW(mContent, i)) {
                            i += TD_NEW_SIZE;
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        while (mContent[i] != '<')
                            date += mContent[i];
                        break;
                    }
                }
                i++;
            }
            if (found)
                break;
        }
    }
    return date;
}

vector<string> HtmlParser::latestRomsForDevice() const
{
    vector<string> names;
    string name("");
    int idx = 0;

    for (size_t i = 0; i < mContent.size(); ++i) {
        if (GET_URL(mContent, i)) {
            i += GET_URL_SIZE;
            while (mContent[i] != '"')
                name += mContent[i++];
            if (!name.empty()) {
                names.push_back(name);
                if (idx == (gRomHistory - 1))
                    break;
                ++idx;
                name = "";
                continue;
            }
        }
    }
    return names;
}

string HtmlParser::currentContent() const
{
    return mContent;
}

BACON_NAMESPACE_END

