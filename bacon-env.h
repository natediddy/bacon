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

#ifndef BACON_ENV_H
#define BACON_ENV_H

#include <stdio.h>

#include "bacon.h"
#include "bacon-os.h"

#ifdef BACON_OS_UNIX
# define BACON_PATH_SEP    '/'
# define bacon_env_is_path_sep(c) ((c) == BACON_PATH_SEP)
#else
# define BACON_PATH_SEP    '\\'
# define bacon_env_is_path_sep(c) (((c) == BACON_PATH_SEP) || ((c) == '/'))
#endif

#define BACON_PATH_MAX 1024

bool  bacon_env_is_directory          (const char *path);
bool  bacon_env_is_file               (const char *path);
void  bacon_env_delete                (const char *path);
long  bacon_env_size_of               (const char *path);
FILE *bacon_env_fopen                 (const char *path, const char *mode);
char *bacon_env_getenv                (const char *key);
char *bacon_env_home_path             (void);
#ifdef BACON_OS_UNIX
void  bacon_env_make_hidden           (char *path);
#else
void  bacon_env_make_hidden           (const char *path);
#endif
void  bacon_env_set_program_data_path (void);
char *bacon_env_cwd                   (void);
char *bacon_env_dirname               (const char *path);
char *bacon_env_basename              (const char *path);
char *bacon_env_mkabs                 (const char *path);
bool  bacon_env_mkpath                (const char *path);
bool  bacon_env_ensure_path           (const char *path, const bool file);
void  bacon_env_fix_download_path     (char **path, const char *name);

#endif /* BACON_ENV_H */

