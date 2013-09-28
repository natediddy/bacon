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

#ifdef BACON_OS_WINDOWS
# define BACON_PATH_SEP    '\\'
# define BACON_IS_SLASH(c) (((c) == BACON_PATH_SEP) || ((c) == '/'))
#else
# define BACON_PATH_SEP    '/'
# define BACON_IS_SLASH(c) ((c) == BACON_PATH_SEP)
#endif

bool  bacon_env_dir_exists     (const char *path);
bool  bacon_env_file_exists    (const char *path);
void  bacon_env_delete         (const char *path);
long  bacon_env_size_of        (const char *path);
FILE *bacon_env_fopen          (const char *path, const char *mode);
char *bacon_env_home_path      (void);
char *bacon_env_program_path   (void);
char *bacon_env_cwd            (void);
char *bacon_env_make_absolute  (const char *path);
bool  bacon_env_mkpath         (const char *path);
bool  bacon_env_validate_path  (const char *path, bool file);

#endif /* BACON_ENV_H */

