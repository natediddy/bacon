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

/* '<li class="bullet"><a href="/?device=' */
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

/* '/get/' */
#define GET_URL(s, i) \
  s[i] == '/' && s[i+1] == 'g' && s[i+2] == 'e' && s[i+3] == 't' \
  && s[i+4] == '/'

/* 'md5sum: ' */
#define MD5_CLASS(s, i) \
  s[i]      == 'm' && s[i+1] == 'd' && s[i+2] == '5' && s[i+3] == 's' \
  && s[i+4] == 'u' && s[i+5] == 'm' && s[i+6] == ':' && s[i+7] == ' '

using std::string;
using std::vector;

namespace bacon
{
  namespace
  {
    bool filenameMatch(const string &s, const string &n, const size_t i)
    {
      for (size_t j = 0, k = i; j < n.size(); ++j, ++k)
      {
        if (s[k] != n[j])
          return false;
      }
      return true;
    }
  } /* namespace */

  HtmlParser::HtmlParser(const string &content)
    : mContent(content)
  {}

  HtmlParser::~HtmlParser()
  {}

  void HtmlParser::allDeviceIds(vector<string> &vec) const
  {
    for (size_t i = 0; i < mContent.size(); i++)
    {
      if (DEVICE_BULLET(mContent, i))
      {
        string tmp("");
        i += 37;
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

    for (size_t i = 0; i < mContent.size(); ++i)
    {
      if (filenameMatch(mContent, filename, i))
      {
        i += filename.size();
        while (true)
        {
          if (MD5_CLASS(mContent, i))
          {
            i += 8;
            while (mContent[i] != '<')
              hash += mContent[i++];
            break;
          }
          i++;
        }
      }
    }

    return hash;
  }

  string HtmlParser::latestRomForDevice() const
  {
    string name("");

    for (size_t i = 0; i < mContent.size(); i++)
    {
      if (GET_URL(mContent, i))
      {
        i += 5;
        while (mContent[i] != '"')
          name += mContent[i++];
        break;
      }
    }

    return name;
  }

  string HtmlParser::currentContent() const
  {
    return mContent;
  }
} /* namespace bacon */

