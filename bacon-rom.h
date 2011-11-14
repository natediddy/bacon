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

#ifndef BACON_ROM_H_INCLUDED
#define BACON_ROM_H_INCLUDED

#include "bacon-file.h"
#include "bacon-net.h"

#include <cstdio>

namespace bacon
{
  class File;

  class Rom : public Net, File {
  public:
    Rom(const std::string &, const std::string &);
    ~Rom();

    bool fetch();
    bool exists() const;

  protected:
    bool setup();

  private:
    friend size_t write_CB(void *, size_t, size_t, FILE *);
  };
}

#endif /* !BACON_ROM_H_INCLUDED */

