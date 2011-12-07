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

#ifndef BACON_PREFS_H_INCLUDED
#define BACON_PREFS_H_INCLUDED

#include <string>

#include "bacon.h"

#define BACON_PREFS_KEY_BASE_DIR       0
#define BACON_PREFS_KEY_LOG_PATH       1
#define BACON_PREFS_KEY_CM_ROOT_SERVER 2
#define BACON_PREFS_KEY_TOTAL          3

BACON_NAMESPACE_BEGIN

namespace prefs {

typedef unsigned int Key;

void init();
void override(const Key key, const std::string &val);
std::string get(const Key key);
bool check();

} /* namespace prefs */

BACON_NAMESPACE_END
#endif /* !BACON_PREFS_H_INCLUDED */

