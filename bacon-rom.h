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

#ifndef BACON_ROM_H
#define BACON_ROM_H

#include "bacon-hash.h"

#define BACON_ROM_NAME_MAX 128
#define BACON_ROM_GET_MAX  8
#define BACON_ROM_SIZE_MAX 12
#define BACON_ROM_DATE_MAX 24

#define BACON_ROM_TYPE_NONE     0

#define BACON_ROM_TYPE_ALL      0x0200
#define BACON_ROM_TYPE_NIGHTLY  0x0800
#define BACON_ROM_TYPE_RC       0x0400
#define BACON_ROM_TYPE_SNAPSHOT 0x0100
#define BACON_ROM_TYPE_STABLE   0x0008
#define BACON_ROM_TYPE_TEST     0x0004

#define BACON_NIGHTLY  0
#define BACON_RC       1
#define BACON_SNAPSHOT 2
#define BACON_STABLE   3
#define BACON_TEST     4
#define BACON_TOTAL    5

typedef struct BaconRom     BaconRom;
typedef struct BaconRomList BaconRomList;

struct BaconRom
{
  char name[BACON_ROM_NAME_MAX];
  char get[BACON_ROM_GET_MAX];
  char size[BACON_ROM_SIZE_MAX];
  char date[BACON_ROM_DATE_MAX];
  BaconHash hash;
  BaconRom *next;
  BaconRom *prev;
};

struct BaconRomList
{
  BaconRom *roms[BACON_TOTAL];
};

BaconRomList *bacon_rom_list_new     (const char *codename,
                                      int type,
                                      int max);
void          bacon_rom_list_destroy (BaconRomList *rom_list);
const char *  bacon_rom_type_str     (int index);

#endif /* BACON_ROM_H */

