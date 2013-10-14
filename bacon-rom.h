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

#ifndef BACON_ROM_H
#define BACON_ROM_H

#include "bacon-hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BACON_ROM_NAME_MAX      128
#define BACON_ROM_GET_MAX       8
#define BACON_ROM_SIZE_MAX      12
#define BACON_ROM_DATE_MAX      24
#define BACON_ROM_TYPE_NONE     0
#define BACON_ROM_TYPE_ALL      0x0200
#define BACON_ROM_TYPE_NIGHTLY  0x0800
#define BACON_ROM_TYPE_RC       0x0400
#define BACON_ROM_TYPE_SNAPSHOT 0x0100
#define BACON_ROM_TYPE_STABLE   0x0008
#define BACON_ROM_TYPE_TEST     0x0004
#define BACON_ROM_NIGHTLY       0
#define BACON_ROM_RC            1
#define BACON_ROM_SNAPSHOT      2
#define BACON_ROM_STABLE        3
#define BACON_ROM_TEST          4
#define BACON_ROM_TOTAL         5

typedef struct BaconRom     BaconRom;
typedef struct BaconRomList BaconRomList;

struct BaconRom {
  char name[BACON_ROM_NAME_MAX];
  char get[BACON_ROM_GET_MAX];
  char size[BACON_ROM_SIZE_MAX];
  char date[BACON_ROM_DATE_MAX];
  BaconHash hash;
  BaconRom *next;
  BaconRom *prev;
};

struct BaconRomList {
  BaconRom *roms[BACON_ROM_TOTAL];
};

BaconRomList *bacon_rom_list_new (const char *codename, int type, int max);
void bacon_rom_list_destroy (BaconRomList *rom_list);
const char *bacon_rom_type_str (int index);
bool bacon_rom_do_download (const BaconRom *rom, char *dlpath);

#ifdef __cplusplus
}
#endif

#endif /* BACON_ROM_H */

