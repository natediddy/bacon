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

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_ROM_TYPE_TEST_STRING     "Experimental"
#define BACON_ROM_TYPE_SNAPSHOT_STRING "Snapshot"
#define BACON_ROM_TYPE_NIGHTLY_STRING  "Nightly"
#define BACON_ROM_TYPE_RC_STRING       "Release Candidate"
#define BACON_ROM_TYPE_STABLE_STRING   "Stable"
#define BACON_ROM_TYPE_ALL_STRING      ""

#define BACON_NIGHTLY_FORMAT  "nightly"
#define BACON_RC_FORMAT       "RC"
#define BACON_SNAPSHOT_FORMAT "snapshot"
#define BACON_STABLE_FORMAT   "stable"
#define BACON_TEST_FORMAT     "test"
#define BACON_ALL_FORMAT      ""

#define BACON_REQUEST_FORMAT "?device=%s&type=%s"

#define BACON_REQUEST_MAX 128

char request[BACON_REQUEST_MAX];

static void
bacon_form_request (const char *codename, int id)
{
  const char *type_str;

  *request = '\0';
  switch (id)
  {
    case BACON_NIGHTLY:
      type_str = BACON_NIGHTLY_FORMAT;
      break;
    case BACON_RC:
      type_str = BACON_RC_FORMAT;
      break;
    case BACON_SNAPSHOT:
      type_str = BACON_SNAPSHOT_FORMAT;
      break;
    case BACON_STABLE:
      type_str = BACON_STABLE_FORMAT;
      break;
    case BACON_TEST:
      type_str = BACON_TEST_FORMAT;
      break;
    default:
      type_str = BACON_ALL_FORMAT;
      break;
  }

  snprintf (request, BACON_REQUEST_MAX, BACON_REQUEST_FORMAT,
            codename, type_str);
}

static BaconRom *
bacon_setup_rom (const char *codename, int id, int *max)
{
  char *data;
  BaconRom *rom;

  rom = NULL;
  bacon_form_request (codename, id);
  if (bacon_net_init_for_page_data (request))
  {
    data = bacon_net_get_page_data ();
    if (data)
      rom = bacon_parse_for_rom (data, *max);
    bacon_net_deinit ();
  }
  return rom;
}

BaconRomList *
bacon_rom_list_new (const char *codename, int type, int max)
{
  int x;
  char *data;
  BaconRomList *list;

  list = bacon_new (BaconRomList);

  if (type & BACON_ROM_TYPE_ALL)
  {
    for (x = 0; x < BACON_TOTAL; ++x)
      list->roms[x] = bacon_setup_rom (codename, x, &max);
    return list;
  }

  if (type & BACON_ROM_TYPE_NIGHTLY)
    list->roms[BACON_NIGHTLY] =
      bacon_setup_rom (codename, BACON_NIGHTLY, &max);
  else
    list->roms[BACON_NIGHTLY] = NULL;

  if (type & BACON_ROM_TYPE_RC)
    list->roms[BACON_RC] = bacon_setup_rom (codename, BACON_RC, &max);
  else
    list->roms[BACON_RC] = NULL;

  if (type & BACON_ROM_TYPE_SNAPSHOT)
    list->roms[BACON_SNAPSHOT] =
      bacon_setup_rom (codename, BACON_SNAPSHOT, &max);
  else
    list->roms[BACON_SNAPSHOT] = NULL;

  if (type & BACON_ROM_TYPE_TEST)
    list->roms[BACON_TEST] = bacon_setup_rom (codename, BACON_TEST, &max);
  else
    list->roms[BACON_TEST] = NULL;
  return list;
}

void
bacon_rom_list_destroy (BaconRomList *list)
{
  int x;
  BaconRom *rom;

  if (!list)
    return;

  for (x = 0; x < BACON_TOTAL; ++x)
  {
    rom = list->roms[x];
    for (; rom; rom = rom->next)
    {
      bacon_free (rom->prev);
      if (!rom->next)
        break;
    }
    bacon_free (rom);
  }
  bacon_free (list);
}

const char *
bacon_rom_type_str (int index)
{
  switch (index)
  {
    case BACON_NIGHTLY:
      return BACON_ROM_TYPE_NIGHTLY_STRING;
    case BACON_RC:
      return BACON_ROM_TYPE_RC_STRING;
    case BACON_SNAPSHOT:
      return BACON_ROM_TYPE_SNAPSHOT_STRING;
    case BACON_STABLE:
      return BACON_ROM_TYPE_STABLE_STRING;
    case BACON_TEST:
      return BACON_ROM_TYPE_TEST_STRING;
    default:
      return BACON_ROM_TYPE_ALL_STRING;
  }
  return NULL;
}

