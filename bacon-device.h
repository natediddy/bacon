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

#ifndef BACON_DEVICE_H
#define BACON_DEVICE_H

#include "bacon.h"

#define BACON_DEVICE_NAME_MAX 128

typedef struct BaconDeviceList BaconDeviceList;

typedef struct
{
  char codename[BACON_DEVICE_NAME_MAX];
  char fullname[BACON_DEVICE_NAME_MAX];
} BaconDevice;

struct BaconDeviceList
{
  BaconDevice *device;
  BaconDeviceList *next;
  BaconDeviceList *prev;
};

BaconDeviceList *bacon_device_list_new           (const bool local);
void             bacon_device_list_destroy       (BaconDeviceList *list);
int              bacon_device_list_total         (BaconDeviceList *list);
bool             bacon_device_is_valid_id        (BaconDeviceList *list,
                                                  const char *id);
BaconDevice *    bacon_device_get_device_from_id (BaconDeviceList *list,
                                                  const char *id);

#endif /* BACON_DEVICE_H */

