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

#ifndef BACON_OUT_H
#define BACON_OUT_H

#include <stdarg.h>
#include <stdio.h>

#include "bacon.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_COLOR_BLACK   0
#define BACON_COLOR_RED     1
#define BACON_COLOR_GREEN   2
#define BACON_COLOR_YELLOW  3
#define BACON_COLOR_BLUE    4
#define BACON_COLOR_MAGENTA 5
#define BACON_COLOR_CYAN    6
#define BACON_COLOR_WHITE   7

#ifdef BACON_DEBUG
# define bacon_debug(msg, ...) \
  __bacon_debug (__FILE__, __func__, __LINE__, msg, __VA_ARGS__)
#else
# define bacon_debug(msg, ...)
#endif

#ifdef BACON_DEBUG
void __bacon_debug (const char *file,
                    const char *func,
                    int line,
                    const char *msg,
                    ...);
#endif
void bacon_error (const char *msg, ...);
void bacon_warn (const char *msg, ...);
void bacon_foutc (FILE *stream, const char c);
void bacon_outc (const char c);
void bacon_foutcco (FILE *stream, int color, bool bold, const char c);
void bacon_outcco (int color, bool bold, const char c);
void bacon_fout (FILE *stream, const char *msg, ...);
void bacon_foutln (FILE *stream, const char *msg, ...);
void bacon_foutco (FILE *stream, int color, bool bold, const char *msg, ...);
void bacon_foutlnco (FILE *stream,
                     int color,
                     bool bold,
                     const char *msg,
                     ...);
void bacon_out (const char *msg, ...);
void bacon_outln (const char *msg, ...);
void bacon_outco (int color, bool bold, const char *msg, ...);
void bacon_outlnco (int color, bool bold, const char *msg, ...);
void bacon_outi (int level, const char *msg, ...);
void bacon_outlni (int level, const char *msg, ...);
void bacon_msg (const char *msg, ...);
void bacon_color (int color, bool bold, FILE *stream);
void bacon_no_color (FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* BACON_OUT_H */

