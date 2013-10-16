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

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-net.h"
#include "bacon-out.h"
#include "bacon-parse.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_ROM_TYPE_TEST_STRING     "Experimental"
#define BACON_ROM_TYPE_SNAPSHOT_STRING "Snapshot"
#define BACON_ROM_TYPE_NIGHTLY_STRING  "Nightly"
#define BACON_ROM_TYPE_RC_STRING       "Release Candidate"
#define BACON_ROM_TYPE_STABLE_STRING   "Stable"
#define BACON_ROM_TYPE_ALL_STRING      "All"
#define BACON_NIGHTLY_FORMAT           "nightly"
#define BACON_RC_FORMAT                "RC"
#define BACON_SNAPSHOT_FORMAT          "snapshot"
#define BACON_STABLE_FORMAT            "stable"
#define BACON_TEST_FORMAT              "test"
#define BACON_ALL_FORMAT               ""
#define BACON_REQUEST_FORMAT           "?device=%s&type=%s"

#define BACON_REQUEST_MAX 256

static char s_request[BACON_REQUEST_MAX];

static void
bacon_form_request (const char *codename, int id)
{
  const char *type_str;

  *s_request = '\0';
  switch (id) {
  case BACON_ROM_NIGHTLY:
    type_str = BACON_NIGHTLY_FORMAT;
    break;
  case BACON_ROM_RC:
    type_str = BACON_RC_FORMAT;
    break;
  case BACON_ROM_SNAPSHOT:
    type_str = BACON_SNAPSHOT_FORMAT;
    break;
  case BACON_ROM_STABLE:
    type_str = BACON_STABLE_FORMAT;
    break;
  case BACON_ROM_TEST:
    type_str = BACON_TEST_FORMAT;
    break;
  default:
    type_str = BACON_ALL_FORMAT;
    break;
  }

  snprintf (s_request, BACON_REQUEST_MAX,
            BACON_REQUEST_FORMAT, codename, type_str);
}

static BaconRom *
bacon_setup_rom (const char *codename, int id, int *max)
{
  char *data;
  BaconRom *rom;

  rom = NULL;
  bacon_form_request (codename, id);
  if (bacon_net_init_for_page_data (s_request)) {
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
  BaconRomList *list;

  list = bacon_new (BaconRomList);

  if (type & BACON_ROM_TYPE_ALL) {
    for (x = 0; x < BACON_ROM_TOTAL; ++x)
      list->roms[x] = bacon_setup_rom (codename, x, &max);
    return list;
  }

  if (type & BACON_ROM_TYPE_NIGHTLY)
    list->roms[BACON_ROM_NIGHTLY] =
      bacon_setup_rom (codename, BACON_ROM_NIGHTLY, &max);
  else
    list->roms[BACON_ROM_NIGHTLY] = NULL;

  if (type & BACON_ROM_TYPE_RC)
    list->roms[BACON_ROM_RC] = bacon_setup_rom (codename, BACON_ROM_RC, &max);
  else
    list->roms[BACON_ROM_RC] = NULL;

  if (type & BACON_ROM_TYPE_SNAPSHOT)
    list->roms[BACON_ROM_SNAPSHOT] =
      bacon_setup_rom (codename, BACON_ROM_SNAPSHOT, &max);
  else
    list->roms[BACON_ROM_SNAPSHOT] = NULL;

  if (type & BACON_ROM_TYPE_STABLE)
    list->roms[BACON_ROM_STABLE] =
      bacon_setup_rom (codename, BACON_ROM_STABLE, &max);
  else
    list->roms[BACON_ROM_STABLE] = NULL;

  if (type & BACON_ROM_TYPE_TEST)
    list->roms[BACON_ROM_TEST] =
      bacon_setup_rom (codename, BACON_ROM_TEST, &max);
  else
    list->roms[BACON_ROM_TEST] = NULL;
  return list;
}

void
bacon_rom_list_destroy (BaconRomList *list)
{
  int x;
  BaconRom *rom;

  if (!list)
    return;

  for (x = 0; x < BACON_ROM_TOTAL; ++x) {
    rom = list->roms[x];
    bacon_list_free (rom);
  }
  bacon_free (list);
}

const char *
bacon_rom_type_str (int index)
{
  switch (index) {
  case BACON_ROM_NIGHTLY:
    return BACON_ROM_TYPE_NIGHTLY_STRING;
  case BACON_ROM_RC:
    return BACON_ROM_TYPE_RC_STRING;
  case BACON_ROM_SNAPSHOT:
    return BACON_ROM_TYPE_SNAPSHOT_STRING;
  case BACON_ROM_STABLE:
    return BACON_ROM_TYPE_STABLE_STRING;
  case BACON_ROM_TEST:
    return BACON_ROM_TYPE_TEST_STRING;
  default:
    return BACON_ROM_TYPE_ALL_STRING;
  }
  return NULL;
}

BaconBoolean
bacon_rom_do_download (const BaconRom *rom, char *dlpath)
{
  unsigned long offset;
  BaconBoolean dlres;
  BaconHash hash;

  bacon_env_fix_download_path (&dlpath, rom->name);
  if (!bacon_env_ensure_path (dlpath, BACON_TRUE)) {
    bacon_error ("`%s' is an invalid path", dlpath);
    return BACON_FALSE;
  }

  offset = 0L;
  if (bacon_env_is_file (dlpath)) {
    bacon_hash_from_file (&hash, dlpath);
    if (bacon_hash_match (&hash, &rom->hash)) {
      bacon_msg ("`%s' already exists - no need to redownload", dlpath);
      return BACON_TRUE;
    }
    bacon_msg ("resuming download of `%s'", dlpath);
    offset = bacon_env_size_of_file (dlpath);
  }

  dlres = BACON_TRUE;
  if (bacon_net_init_for_rom (rom->get, offset, dlpath)) {
    if (!bacon_net_get_file ())
      dlres = BACON_FALSE;
    bacon_net_deinit ();
  } else
    dlres = BACON_FALSE;

  if (dlres) {
    bacon_hash_from_file (&hash, dlpath);
    if (!bacon_hash_match (&hash, &rom->hash)) {
      bacon_warn ("checksum mismatch for `%s' (possibly corrupt)", dlpath);
      return BACON_FALSE;
    }
    return BACON_TRUE;
  }
  return BACON_FALSE;
}

