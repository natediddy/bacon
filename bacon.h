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

#ifndef BACON_H
#define BACON_H

#ifdef HAVE_CONFIG_H
# include "bacon-config.h"
# define BACON_PROGRAM_NAME PACKAGE_NAME
# define BACON_VERSION      PACKAGE_VERSION
#else
# define BACON_PROGRAM_NAME "bacon"
# define BACON_VERSION      "1.0.0"
#endif

#define BACON_ROOT_URL "http://get.cm"

#ifndef __cplusplus
typedef int bool;
# define bool bool
# undef false
# define false (0)
# undef true
# define true (!false)
#endif

#endif /* BACON_H */

