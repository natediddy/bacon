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

#ifndef BACON_H
#define BACON_H

#ifdef HAVE_CONFIG_H
# include "bacon-config.h"
#endif
#include "bacon-sys.h"

#ifdef PACKAGE_NAME
# define BACON_PROGRAM_NAME PACKAGE_NAME
#else
# define BACON_PROGRAM_NAME "bacon"
#endif

#ifdef PACKAGE_VERSION
# define BACON_VERSION PACKAGE_VERSION
#else
# define BACON_VERSION "3.0"
#endif

#ifdef PACKAGE_BUGREPORT
# define BACON_BUG_REPORT_EMAIL PACKAGE_BUGREPORT
#else
# define BACON_BUG_REPORT_EMAIL "me@nathanforbes.com"
#endif

#endif /* BACON_H */

