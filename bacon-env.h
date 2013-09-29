/*
 * Copyright (C) 2013 Nathan Forbes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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

bool  bacon_env_is_directory      (const char *path);
bool  bacon_env_is_file           (const char *path);
void  bacon_env_delete            (const char *path);
long  bacon_env_size_of           (const char *path);
FILE *bacon_env_fopen             (const char *path, const char *mode);
char *bacon_env_getenv            (const char *key);
char *bacon_env_home_path         (void);
#ifdef BACON_OS_UNIX
void  bacon_env_make_hidden       (char *path);
#else
void  bacon_env_make_hidden       (const char *path);
#endif
char *bacon_env_program_path      (void);
char *bacon_env_cwd               (void);
char *bacon_env_dirname           (const char *path);
char *bacon_env_basename          (const char *path);
char *bacon_env_mkabs             (const char *path);
bool  bacon_env_mkpath            (const char *path);
bool  bacon_env_ensure_path       (const char *path, const bool file);
void  bacon_env_fix_download_path (char **path, const char *name);

#endif /* BACON_ENV_H */

