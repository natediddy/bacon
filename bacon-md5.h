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

#ifndef BACON_MD5_H_INCLUDED
#define BACON_MD5_H_INCLUDED

namespace bacon
{
  class Md5 {
  public:
    Md5(const std::string &, const std::string &, const std::string &);

    bool verify() const;

  private:
    std::string mLocalHash;
    std::string mRemoteHash;
  };
}

#endif /* !BACON_MD5_H_INCLUDED */

