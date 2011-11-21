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

#ifndef BACON_UTIL_H_INCLUDED
#define BACON_UTIL_H_INCLUDED

#include <string>

namespace bacon
{
  namespace util
  {
    std::string toUpperCase(const std::string &str);
    std::string toLowerCase(const std::string &str);
    std::string bytesToReadable(const size_t size,
                                const long &bytes,
                                const bool precision = false);
    void randomSeed();
    unsigned int random();
    std::string timeString(const std::string &fmt);
    bool isAlphaNumeric(const char c);
    void strip(std::string &str);
    bool isValidUrl(std::string &url);
    std::string convertShellSymbols(const std::string &val);
  }
}

#endif /* !BACON_UTIL_H_INCLUDED */

