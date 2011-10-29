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

#include "bacon-htmldoc.h"
#include "bacon-htmlparser.h"

/* <li class="bullet"><a href="/?device= */
#define DEVICE_BULLET(s, i) \
  s[i]       == '<' && s[i+1]  == 'l' && s[i+2]  == 'i' && s[i+3]  == ' ' \
  && s[i+4]  == 'c' && s[i+5]  == 'l' && s[i+6]  == 'a' && s[i+7]  == 's' \
  && s[i+8]  == 's' && s[i+9]  == '=' && s[i+10] == '"' && s[i+11] == 'b' \
  && s[i+12] == 'u' && s[i+13] == 'l' && s[i+14] == 'l' && s[i+15] == 'e' \
  && s[i+16] == 't' && s[i+17] == '"' && s[i+18] == '>' && s[i+19] == '<' \
  && s[i+20] == 'a' && s[i+21] == ' ' && s[i+22] == 'h' && s[i+23] == 'r' \
  && s[i+24] == 'e' && s[i+25] == 'f' && s[i+26] == '=' && s[i+27] == '"' \
  && s[i+28] == '/' && s[i+29] == '?' && s[i+30] == 'd' && s[i+31] == 'e' \
  && s[i+32] == 'v' && s[i+33] == 'i' && s[i+34] == 'c' && s[i+35] == 'e' \
  && s[i+36] == '='

#define GET_URL(s, i) \
  s[i] == '/' && s[i+1] == 'g' && s[i+2] == 'e' && s[i+3] == 't' \
  && s[i+4] == '/'

using std::string;
using std::vector;

namespace bacon
{
  HtmlParser::HtmlParser(const string & content)
    : mContent(content)
  {}

  HtmlParser::~HtmlParser()
  {}

  void HtmlParser::allDeviceIds(vector<string> & vec) const
  {
    string tmp = "";
    string sub = mContent;

    for (size_t i = 0; i < sub.size(); i++) {
      if (DEVICE_BULLET(sub, i)) {
        i += 37;
        while (sub[i] != '"') {
          tmp += sub[i++];
        }
        if (!tmp.empty()) {
          vec.push_back(tmp);
        }
        tmp = "";
      }
    }
  }

  string HtmlParser::latestRomForDevice(const string & device) const
  {
    string name = "";
    string sub = mContent;

    for (size_t i = 0; i < sub.size(); i++) {
      if (GET_URL(sub, i)) {
        i += 5;
        while (sub[i] != '"') {
          name += sub[i++];
        }
        break;
      }
    }
    return name;
  }

  string HtmlParser::currentContent() const
  {
    return mContent;
  }
}

