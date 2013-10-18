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

#define BACON_COLOR_NONE 0
#define BACON_BOLD       (1 << 0)
#define BACON_UNDERLINE  (1 << 1)
#define BACON_BLACK      (1 << 2)
#define BACON_RED        (1 << 3)
#define BACON_GREEN      (1 << 4)
#define BACON_YELLOW     (1 << 5)
#define BACON_BLUE       (1 << 6)
#define BACON_MAGENTA    (1 << 7)
#define BACON_CYAN       (1 << 8)
#define BACON_WHITE      (1 << 9)

#define __bacon_color(c) (BACON_COLOR_NONE | (c))

/* bacon.c */
#define BACON_FULLNAME_COLOR __bacon_color (BACON_MAGENTA)
#define BACON_CODENAME_COLOR __bacon_color (BACON_CYAN)
#define BACON_NUMBER_LIST_COLOR  __bacon_color (BACON_BLUE)
#define BACON_FIND_PATTERN_COLOR __bacon_color (BACON_BOLD | BACON_RED)
#define BACON_ROM_TYPE_COLOR __bacon_color (BACON_RED)
#define BACON_ROM_NAME_COLOR __bacon_color (BACON_BOLD | BACON_GREEN)
#define BACON_ROM_DATE_COLOR __bacon_color (BACON_BLUE)
#define BACON_ROM_SIZE_COLOR __bacon_color (BACON_BLUE)
#define BACON_ROM_HASH_COLOR __bacon_color (BACON_BLUE)
#define BACON_ROM_URL_COLOR __bacon_color (BACON_BLUE)

/* bacon-out.c */
#define BACON_PROGRAM_NAME_COLOR __bacon_color (BACON_CYAN)
#define BACON_ERROR_TAG_COLOR __bacon_color (BACON_BOLD | BACON_RED)
#define BACON_WARNNG_TAG_COLOR __bacon_color (BACON_BOLD | BACON_YELLOW)
#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG_COLOR __bacon_color (BACON_BOLD | BACON_GREEN)
# define BACON_DEBUG_FILE_TAG_COLOR __bacon_color (BACON_BOLD | BACON_WHITE)
# define BACON_DEBUG_FUNC_TAG_COLOR __bacon_color (BACON_MAGENTA)
# define BACON_DEBUG_LINE_TAG_COLOR __bacon_color (BACON_YELLOW)
#endif

/* bacon-progress.c */
#define BACON_CURRENT_BYTES_COLOR __bacon_color (BACON_BOLD | BACON_CYAN)
#define BACON_TOTAL_BYTES_COLOR __bacon_color (BACON_BOLD | BACON_MAGENTA)
#define BACON_BAR_HAS_CHAR_COLOR __bacon_color (BACON_BOLD | BACON_CYAN)
#define BACON_BAR_NOT_CHAR_COLOR __bacon_color (BACON_BOLD | BACON_MAGENTA)
#define BACON_SPEED_COLOR __bacon_color (BACON_BOLD | BACON_GREEN)
#define BACON_ETA_DIGIT_COLOR __bacon_color (BACON_BOLD | BACON_GREEN)
#define BACON_ETA_ALPHA_COLOR __bacon_color (BACON_BOLD | BACON_MAGENTA)
#define BACON_PERCENT_COLOR __bacon_color (BACON_BOLD | BACON_CYAN)
#define BACON_LOADING_COLOR __bacon_color (BACON_BOLD | BACON_MAGENTA)
#define BACON_PROPELLER_COLOR __bacon_color (BACON_BOLD | BACON_CYAN)

#endif /* BACON_COLORS_H */

