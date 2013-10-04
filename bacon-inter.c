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

#include <string.h>

#include "bacon.h"
#include "bacon-env.h"
#include "bacon-hash.h"
#include "bacon-inter.h"
#include "bacon-net.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_ANSWER_MAX BACON_DEVICE_NAME_MAX

#define BACON_ANSWER_INVALID 0
#define BACON_ANSWER_YES     1
#define BACON_ANSWER_NO      2

extern BaconDeviceList *g_device_list;
extern char *           g_out_path;
extern int              g_max_roms;
extern int              g_rom_type;

static const char *const yesno_answers[] = {
  "y", "Y", "yes", "YES", "yEs", "YeS", "yES", "yeS", "Yes", "YEs",
  "n", "N", "no", "NO", "nO", "No",
  NULL
};

static int
bacon_valid_yesno_answer (const char *ans)
{
  size_t x;

  for (x = 0; yesno_answers[x]; ++x) {
    if (bacon_streq (ans, yesno_answers[x])) {
      if ((*ans == 'y') || (*ans == 'Y'))
        return BACON_ANSWER_YES;
      if ((*ans == 'n') || (*ans == 'N'))
        return BACON_ANSWER_NO;
    }
  }
  return BACON_ANSWER_INVALID;
}

static bool
bacon_yesno (const char *question, ...)
{
  va_list a;
  char answer[BACON_ANSWER_MAX];

  while (true) {
    va_start (a, question);
    vfprintf (stdout, question, a);
    va_end (a);
    fputs ("? [y/n] ", stdout);
    if (!fgets (answer, BACON_ANSWER_MAX, stdin)) {
      bacon_error ("failed to read from standard input");
      exit (EXIT_FAILURE);
    }
    switch (bacon_valid_yesno_answer (answer)) {
    case BACON_ANSWER_YES:
      return true;
    case BACON_ANSWER_NO:
      return false;
    default:
      bacon_warn ("`%s' unrecognized - try again...", answer);
    }
  }
}

static bool
bacon_get_num_answer (int *input)
{
  size_t n;
  char answer[BACON_ANSWER_MAX];

  if (!fgets (answer, BACON_ANSWER_MAX, stdin)) {
    bacon_error ("failed to read from standard input");
    exit (EXIT_FAILURE);
  }
  *input = bacon_int_from_str (answer);
  if ((*input == 0) && *answer) {
    n = strlen (answer);
    if (answer[n - 1] == '\n')
      answer[n - 1] = '\0';
    bacon_warn ("'%s' not valid - please try again...", answer);
    return false;
  }
  return true;
}

static BaconDevice *
bacon_get_device_from_index (int *idx)
{
  int cidx;
  BaconDevice *device;
  BaconDeviceList *p;

  cidx = 1;
  device = NULL;
  for (p = g_device_list; p; p = p->next) {
    if (cidx == *idx) {
      device = p->device;
      break;
    }
    if (!p->next)
      break;
    cidx++;
  }
  return device;
}

static void
bacon_get_device_answer (BaconDevice **device)
{
  int idx;
  size_t n;
  char answer[BACON_ANSWER_MAX];

  if (!fgets (answer, BACON_ANSWER_MAX, stdin)) {
    bacon_error ("failed to read from standard input");
    exit (EXIT_FAILURE);
  }

  n = strlen (answer);
  if (answer[n - 1] == '\n')
    answer[n - 1] = '\0';

  if (bacon_device_is_valid_id (g_device_list, answer)) {
    (*device) = bacon_device_get_device_from_id (g_device_list, answer);
    return;
  }

  idx = bacon_int_from_str (answer);
  (*device) = bacon_get_device_from_index (&idx);
}

static void
bacon_display_device_choices (void)
{
  int idx;
  BaconDeviceList *p;

  idx = 1;
  bacon_outln ("Devices:");
  for (p = g_device_list; p; p = p->next) {
    bacon_outln ("%4i) %s - %s",
        idx, p->device->codename, p->device->fullname);
    if (!p->next)
      break;
    idx++;
  }
}

static void
bacon_display_rom_type_choices (void)
{
  int type;

  bacon_outln ("ROM Types:");
  for (type = 0; type < BACON_ROM_TOTAL; ++type)
    bacon_outln ("%4i) %s", type + 1, bacon_rom_type_str (type));
}

static void
bacon_display_rom_choices (const BaconRomList *list,
                           const int id,
                           int *total_choices)
{
  int idx;
  BaconRom *rom;

  idx = 0;
  /* add some extra spaces here to cover up the "Loading..." propeller */
  bacon_outlni (0, "%s:         ", bacon_rom_type_str (id));
  rom = list->roms[id];
  if (rom) {
    idx = 1;
    for (; rom; rom = rom->next) {
      bacon_outlni (1, "%i) %s", idx, rom->name);
      bacon_outlni (2, "released: %s", rom->date);
      bacon_outlni (2, "size:     %s", rom->size);
      if (!rom->next)
        break;
      ++idx;
    }
  } else
    bacon_outlni (1, "none");
  *total_choices = idx;
}

static BaconRom *
bacon_get_rom_by_index (const BaconRomList *list,
                        const int id,
                        const int index)
{
  int idx;
  BaconRom *rom;

  rom = list->roms[id];
  if (rom) {
    idx = 0;
    for (; rom; rom = rom->next) {
      if (idx == index)
        return rom;
      if (!rom->next)
        break;
      ++idx;
    }
  }
  return NULL;
}

void
bacon_interactive (void)
{
  int idx;
  int total;
  int id;
  long offset;
  bool success;
  BaconHash hash;
  BaconDevice *chosen_device;
  BaconRomList *rom_list;
  BaconRom *rom;

  chosen_device = NULL;
  bacon_display_device_choices ();

  while (true) {
    bacon_out ("Enter choice: [1-%i or codename] ",
        bacon_device_list_total (g_device_list));
    bacon_get_device_answer (&chosen_device);
    if (!chosen_device) {
      bacon_outln ("Invalid response - please try again...");
      continue;
    }
    break;
  }

  bacon_outln (NULL);

  bacon_display_rom_type_choices ();
  while (true) {
    bacon_out ("Enter number: [1-%i] ", BACON_ROM_TOTAL);
    if (!bacon_get_num_answer (&idx))
      continue;
    --idx;
    if (idx == BACON_ROM_NIGHTLY)
      g_rom_type |= BACON_ROM_TYPE_NIGHTLY;
    else if (idx == BACON_ROM_RC)
      g_rom_type |= BACON_ROM_TYPE_RC;
    else if (idx == BACON_ROM_SNAPSHOT)
      g_rom_type |= BACON_ROM_TYPE_SNAPSHOT;
    else if (idx == BACON_ROM_STABLE)
      g_rom_type |= BACON_ROM_TYPE_STABLE;
    else if (idx == BACON_ROM_TEST)
      g_rom_type |= BACON_ROM_TYPE_TEST;
    else {
      bacon_warn ("'%i' not valid - please try again...", idx);
      continue;
    }
    id = idx;
    break;
  }

  bacon_outln (NULL);
  g_max_roms = 50;
  rom_list =
    bacon_rom_list_new (chosen_device->codename, g_rom_type, g_max_roms);
  total = 1;
  bacon_display_rom_choices (rom_list, id, &total);

  if (!total)
    return;

  while (true) {
    bacon_out ("Enter number: [1-%i] ", total);
    if (!bacon_get_num_answer (&idx))
      continue;
    rom = bacon_get_rom_by_index (rom_list, id, idx - 1);
    if (!rom) {
      bacon_warn ("'%i' not valid - please try again...", idx);
      continue;
    }
    break;
  }

  bacon_outln ("\n==========================================");
  bacon_outln ("device:      %s [%s]",
               chosen_device->fullname,
               chosen_device->codename);
  bacon_outln ("type:        %s", bacon_rom_type_str (id));
  bacon_outln ("filename:    %s", rom->name);
  bacon_outln ("released on: %s", rom->date);
  bacon_outln ("size:        %s", rom->size);
  bacon_outln ("url:         %s/%s", BACON_ROOT_URL, rom->get);
  bacon_outln ("md5:         %s", rom->hash.hash);
  bacon_outln ("saving to:   %s", g_out_path);
  bacon_outln ("==========================================\n");

  if (!bacon_rom_do_download (rom, g_out_path)) {
    bacon_rom_list_destroy (rom_list);
    exit (EXIT_FAILURE);
  }
  bacon_rom_list_destroy (rom_list);
}

