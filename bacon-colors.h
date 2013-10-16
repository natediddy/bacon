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

#ifndef BACON_COLORS_H
#define BACON_COLORS_H

/* bacon.c */
#define BACON_FULLNAME_COLOR        BACON_COLOR_MAGENTA
#define BACON_CODENAME_COLOR        BACON_COLOR_CYAN
#define BACON_NUMBER_LIST_COLOR     BACON_COLOR_BLUE
#define BACON_FIND_PATTERN_COLOR    BACON_COLOR_RED
#define BACON_ROM_TYPE_COLOR        BACON_COLOR_RED
#define BACON_ROM_NAME_COLOR        BACON_COLOR_GREEN
#define BACON_ROM_DATE_COLOR        BACON_COLOR_BLUE
#define BACON_ROM_SIZE_COLOR        BACON_COLOR_BLUE
#define BACON_ROM_HASH_COLOR        BACON_COLOR_BLUE
#define BACON_ROM_URL_COLOR         BACON_COLOR_BLUE

/* bacon-out.c */
#define BACON_PROGRAM_NAME_COLOR    BACON_COLOR_CYAN
#define BACON_ERROR_TAG_COLOR       BACON_COLOR_RED
#define BACON_WARNNG_TAG_COLOR      BACON_COLOR_YELLOW
#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG_COLOR      BACON_COLOR_GREEN
# define BACON_DEBUG_FILE_TAG_COLOR BACON_COLOR_CYAN
# define BACON_DEBUG_FUNC_TAG_COLOR BACON_COLOR_MAGENTA
# define BACON_DEBUG_LINE_TAG_COLOR BACON_COLOR_YELLOW
#endif

/* bacon-progress.c */
#define BACON_CURRENT_BYTES_COLOR   BACON_COLOR_CYAN
#define BACON_TOTAL_BYTES_COLOR     BACON_COLOR_MAGENTA
#define BACON_BAR_HAS_CHAR_COLOR    BACON_COLOR_CYAN
#define BACON_BAR_NOT_CHAR_COLOR    BACON_COLOR_MAGENTA
#define BACON_SPEED_COLOR           BACON_COLOR_GREEN
#define BACON_ETA_DIGIT_COLOR       BACON_COLOR_GREEN
#define BACON_ETA_ALPHA_COLOR       BACON_COLOR_MAGENTA
#define BACON_PERCENT_COLOR         BACON_COLOR_CYAN
#define BACON_LOADING_COLOR         BACON_COLOR_MAGENTA
#define BACON_PROPELLER_COLOR       BACON_COLOR_CYAN

#endif /* BACON_COLORS_H */

