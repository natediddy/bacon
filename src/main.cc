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

#include <cstdlib>

#include "cmd.h"
#include "prefs.h"

using namespace bacon;

int main(int argc, char **argv)
{
    prefs::init();
    Cmd cmd(&argv);

    if (prefs::check()) {
        atexit(log::deactivate);
        log::activate();
        exit(cmd.exec());
    }
    exit(EXIT_FAILURE);
}

