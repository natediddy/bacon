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

#ifndef BACON_LOG_H_INCLUDED
#define BACON_LOG_H_INCLUDED

namespace bacon {
namespace log {

void activate();
void deactivate();
bool _isActive();
void _write(char *prefix, const char *fmt, ...);
char *_prefix(const char *tag, const char *file, const int line);

} /* namespace log */
} /* namespace bacon */

#endif /* !BACON_LOG_H_INCLUDED */

