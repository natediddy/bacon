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

#ifndef BACON_DEVICE_H
#define BACON_DEVICE_H

#include "bacon.h"
#include "bacon-env.h"

#define BACON_DEVICE_NAME_MAX 128
#ifdef BACON_USING_GTK
# define BACON_DEVICE_THUMB_REQUEST_MAX 1024
#endif

typedef struct BaconDevice                 BaconDevice;
typedef struct BaconDeviceList             BaconDeviceList;
#ifdef BACON_USING_GTK
typedef struct BaconDeviceThumbRequestList BaconDeviceThumbRequestList;

struct BaconDeviceThumbRequestList {
  char request[BACON_DEVICE_THUMB_REQUEST_MAX];
  char filename[BACON_PATH_MAX];
  BaconDeviceThumbRequestList *next;
  BaconDeviceThumbRequestList *prev;
};
#endif

struct BaconDevice {
  char codename[BACON_DEVICE_NAME_MAX];
  char fullname[BACON_DEVICE_NAME_MAX];
};

struct BaconDeviceList {
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

