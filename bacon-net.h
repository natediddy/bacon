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

#ifndef BACON_NET_H
#define BACON_NET_H

#include "bacon.h"

bool  bacon_net_init_for_page_data (const char *request);
bool  bacon_net_init_for_rom       (const char *request,
                                    long offset,
                                    const char *local_filename);
void  bacon_net_deinit             (void);
char *bacon_net_get_page_data      (void);
bool  bacon_net_get_rom            (void);

#endif /* BACON_NET_H */

