/*
 * bacon - A command line tool for viewing/downloading CyanogenMod ROMs
 *         for Android devices.
 *
 * Copyright (C) 2013  Nathan Forbes
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BACON_CTYPE_H
#define BACON_CTYPE_H

#include "bacon.h"

#ifdef __cplusplus
extern "C" {
#endif

BaconBoolean bacon_isalpha (char c);
BaconBoolean bacon_isdigit (char c);
BaconBoolean bacon_islower (char c);
BaconBoolean bacon_isspace (char c);
char bacon_tolower (char c);

#ifdef __cplusplus
}
#endif

#endif /* BACON_CTYPE_H */

