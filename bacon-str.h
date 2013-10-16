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

#ifndef BACON_STR_H
#define BACON_STR_H

#include "bacon.h"

#ifdef __cplusplus
extern "C" {
#endif

char *bacon_strdup (const char *str);
char *bacon_strndup (const char *str, size_t n);
char *bacon_strf (const char *fmt, ...);
BaconBoolean bacon_streq (const char *str1, const char *str2);
BaconBoolean bacon_streqci (const char *str1, const char *str2);
BaconBoolean bacon_strstw (const char *str, const char *pre);
BaconBoolean bacon_strew (const char *str,
                          const char *suf,
                          BaconBoolean case_sensitive);
void bacon_strtolower (char *buf, size_t n, const char *str);
ssize_t bacon_strfposof (const char *str,
                         const char *query,
                         BaconBoolean case_sensitive);
unsigned int bacon_stroccurs (const char *str,
                              const char *query,
                              BaconBoolean case_sensitive);
void bacon_strbytes (char *buf, size_t n, unsigned long bytes);
int bacon_strtoint (const char *str);

#ifdef __cplusplus
}
#endif

#endif /* BACON_STR_H */

