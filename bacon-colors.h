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

#include "bacon.h"
#include "bacon-ctype.h"

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

#define BACON_MAKE_COLOR(attributes) (BACON_COLOR_NONE | (attributes))

/* bacon.c {{{ */
#define BACON_FULLNAME_COLOR     BACON_MAGENTA
#define BACON_CODENAME_COLOR     BACON_CYAN
#define BACON_NUMBER_LIST_COLOR  BACON_BOLD
#define BACON_FIND_PATTERN_COLOR BACON_BOLD | BACON_RED
#define BACON_ROM_TYPE_COLOR     BACON_BOLD | BACON_YELLOW
#define BACON_ROM_NAME_COLOR     BACON_BOLD | BACON_GREEN
#define BACON_ROM_INFO_TAG_COLOR BACON_GREEN
#define BACON_ROM_INFO_COLOR     BACON_BLUE
/* bacon.c }}} */

/* bacon-out.c {{{ */
#define BACON_PROGRAM_NAME_COLOR    BACON_CYAN
#define BACON_ERROR_TAG_COLOR       BACON_BOLD | BACON_RED
#define BACON_WARNING_TAG_COLOR      BACON_BOLD | BACON_YELLOW
#ifdef BACON_DEBUG
# define BACON_DEBUG_TAG_COLOR      BACON_BOLD | BACON_GREEN
# define BACON_DEBUG_FILE_TAG_COLOR BACON_BOLD | BACON_WHITE

# define BACON_DEBUG_FUNC_TAG_COLOR BACON_MAGENTA
# define BACON_DEBUG_LINE_TAG_COLOR BACON_YELLOW
#endif /* BACON_DEBUG */
/* }}} bacon-out.c */

/* bacon-progress.c {{{ */
#define BACON_CURRENT_BYTES_COLOR BACON_CYAN
#define BACON_TOTAL_BYTES_COLOR   BACON_MAGENTA
#define BACON_BAR_HAS_CHAR_COLOR  BACON_CYAN
#define BACON_BAR_NOT_CHAR_COLOR  BACON_MAGENTA
#define BACON_SPEED_COLOR         BACON_GREEN
#define BACON_ETA_DIGIT_COLOR     BACON_GREEN
#define BACON_ETA_ALPHA_COLOR     BACON_MAGENTA
#define BACON_PERCENT_COLOR       BACON_CYAN
#define BACON_LOADING_COLOR       BACON_MAGENTA
#define BACON_PROPELLER_COLOR     BACON_BOLD | BACON_CYAN
/* }}} bacon-progress.c */

#ifdef BACON_ASCII
# ifdef BACON_OS_UNIX
#  define BACON_COLOR_CODE_NONE      "\x1B[0m"
#  define BACON_COLOR_CODE_BOLD      "\x1B[1m"
#  define BACON_COLOR_CODE_UNDERLINE "\x1B[4m"
#  define BACON_COLOR_CODE_BLACK     "\x1B[30m"
#  define BACON_COLOR_CODE_RED       "\x1B[31m"
#  define BACON_COLOR_CODE_GREEN     "\x1B[32m"
#  define BACON_COLOR_CODE_YELLOW    "\x1B[33m"
#  define BACON_COLOR_CODE_BLUE      "\x1B[34m"
#  define BACON_COLOR_CODE_MAGENTA   "\x1B[35m"
#  define BACON_COLOR_CODE_CYAN      "\x1B[36m"
#  define BACON_COLOR_CODE_WHITE     "\x1B[37m"
# endif
#else
# define BACON_COLOR_CODE_NONE      ""
# define BACON_COLOR_CODE_BOLD      ""
# define BACON_COLOR_CODE_UNDERLINE ""
# define BACON_COLOR_CODE_BLACK     ""
# define BACON_COLOR_CODE_RED       ""
# define BACON_COLOR_CODE_GREEN     ""
# define BACON_COLOR_CODE_YELLOW    ""
# define BACON_COLOR_CODE_BLUE      ""
# define BACON_COLOR_CODE_MAGENTA   ""
# define BACON_COLOR_CODE_CYAN      ""
# define BACON_COLOR_CODE_WHITE     ""
#endif

#define BACON_COLOR_S(attributes, s) \
  (__bacon_color_str (BACON_MAKE_COLOR (attributes), s))

#define BACON_COLOR_C(attributes, c) \
  (__bacon_color_char (BACON_MAKE_COLOR (attributes), c))

#define BACON_COLOR_I(attributes, i) \
  (__bacon_color_int (BACON_MAKE_COLOR (attributes), i))

#define BACON_COLOR_D(attributes, d) \
  (__bacon_color_double (BACON_MAKE_COLOR (attributes), d))

void bacon_init_color (void);
const char *__bacon_color_str (int colorp, const char *str);
const char *__bacon_color_char (int colorp, char c);
const char *__bacon_color_int (int colorp, int n);

#endif /* BACON_COLORS_H */

