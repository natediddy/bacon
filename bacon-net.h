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

#ifndef BACON_NET_H
#define BACON_NET_H

#include "bacon.h"

bool  bacon_net_init_for_page_data (const char *request);
bool  bacon_net_init_for_rom       (const char *request,
                                    long offset,
                                    const char *filename);
void  bacon_net_deinit             (void);
char *bacon_net_get_page_data      (void);
bool  bacon_net_get_rom            (void);

#endif /* BACON_NET_H */

