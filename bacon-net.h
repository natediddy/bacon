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

#ifdef BACON_GTK
# include <gtk/gtk.h>
#endif

#include "bacon.h"

/* The main URL where all the device/rom info comes from */
#define BACON_GET_CM_URL             "http://get.cm"
/* Use these URLs from the CM wiki page for device icons in the GUI */
#ifdef BACON_GTK
# define BACON_DEVICE_ICONS_URL      "http://wiki.cyanogenmod.org/w/Devices#"
# define BACON_DEVICE_ICON_THUMB_URL "http://wiki.cyanogenmod.org/images"

bool  bacon_net_init_for_device_icons      (void);
bool  bacon_net_init_for_device_icon_thumb (const char *request,
                                            const char *filename);
bool  bacon_net_gtk_init_for_device_list   (GtkProgressBar *progress_bar);
#endif
bool  bacon_net_init_for_page_data         (const char *request);
bool  bacon_net_init_for_rom               (const char *request,
                                            long offset,
                                            const char *filename);
void  bacon_net_deinit                     (void);
char *bacon_net_get_page_data              (void);
bool  bacon_net_get_file                   (void);

#endif /* BACON_NET_H */

