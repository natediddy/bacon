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

#ifndef BACON_PARSE_H
#define BACON_PARSE_H

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-rom.h"

BaconDeviceList *bacon_parse_for_device_list (const char *data,
                                              const bool local);
BaconRom *       bacon_parse_for_rom         (const char *data,
                                              const int max);

#endif /* BACON_PARSE_H */

