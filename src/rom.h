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

#include "file.h"
#include "net.h"

#include <cstdio>

namespace bacon {

class Rom : public Net, File {
public:
    Rom(const std::string &name, const std::string &path);
    ~Rom();

    bool fetch();

protected:
    bool setup();

private:
    friend size_t write_CB(void *p, size_t s, size_t n, FILE *f);
};

} /* namespace bacon */

#endif /* !BACON_ROM_H_INCLUDED */

