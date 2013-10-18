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

#include "bacon.h"

#include <stdarg.h>
#include <stdio.h>

#include "bacon-colors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_PRINT_PROGRAM_NAME \
  ((g_program_name && *g_program_name) ? g_program_name : "")

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
void bacon_foutc (FILE *stream, char c);
void bacon_outc (const char c);
void bacon_foutcco (FILE *stream, int colorp, char c);
void bacon_outcco (int colorp, char c);
void bacon_fout (FILE *stream, const char *msg, ...);
void bacon_foutln (FILE *stream, const char *msg, ...);
void bacon_foutco (FILE *stream, int colorp, const char *msg, ...);
void bacon_foutlnco (FILE *stream, int colorp, const char *msg, ...);
void bacon_out (const char *msg, ...);
void bacon_outln (const char *msg, ...);
void bacon_outco (int colorp, const char *msg, ...);
void bacon_outlnco (int colorp, const char *msg, ...);
void bacon_outi (int level, const char *msg, ...);
void bacon_outlni (int level, const char *msg, ...);
void bacon_msg (const char *msg, ...);
void bacon_color (FILE *stream, int colorp);
void bacon_no_color (FILE *stream);

#ifdef __cplusplus
}
#endif

#endif /* BACON_OUT_H */

