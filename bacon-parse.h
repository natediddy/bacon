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

#ifndef BACON_PARSE_H
#define BACON_PARSE_H

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-rom.h"

BaconDeviceList *bacon_parse_for_device_list (const char *data,
                                              const bool local);
BaconRom *       bacon_parse_for_rom         (const char *data,
                                              const int max);
#ifdef BACON_GTK
BaconDeviceThumbRequestList *
bacon_parse_for_device_thumb_request_list    (const char *data,
                                              BaconDeviceList *devicelist);
#endif

#endif /* BACON_PARSE_H */

