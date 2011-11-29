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

#ifndef BACON_CONFIG_H_INCLUDED
#define BACON_CONFIG_H_INCLUDED

#include "bacon-file.h"

namespace bacon
{
  class Config : public File {
  public:
    Config();
    ~Config();

    std::string valueOf(const std::string &key);

  private:
    std::string userDefined(const std::string &key);

  private:
    File *mUser;
  };
} /* namespace bacon */

#endif /* !BACON_CONFIG_H_INCLUDED */

