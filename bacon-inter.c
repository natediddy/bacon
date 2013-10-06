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

static BaconRomList *   rom_list   = NULL;
static BaconRom *       rom        = NULL;
static BaconDevice *    device     = NULL;
static char *           dirpath    = NULL;
static int              rom_type_i = -1;

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

static void
bacon_do_cleanup (void)
{
  bacon_free (dirpath);
  bacon_rom_list_destroy (rom_list);
}

static void
bacon_do_exit (void)
{
  bacon_do_cleanup ();
  exit (EXIT_FAILURE);
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
      bacon_do_exit ();
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
    bacon_do_exit ();
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
bacon_get_device_answer (void)
{
  int idx;
  size_t n;
  char answer[BACON_ANSWER_MAX];

  if (!fgets (answer, BACON_ANSWER_MAX, stdin)) {
    bacon_error ("failed to read from standard input");
    bacon_do_exit ();
  }

  n = strlen (answer);
  if (answer[n - 1] == '\n')
    answer[n - 1] = '\0';

  if (bacon_device_is_valid_id (g_device_list, answer)) {
    device = bacon_device_get_device_from_id (g_device_list, answer);
    return;
  }

  idx = bacon_int_from_str (answer);
  device = bacon_get_device_from_index (&idx);
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
bacon_display_rom_choices (const BaconRomList *list, int *total_choices)
{
  int idx;
  BaconRom *rom;

  idx = 0;
  /* add some extra spaces here to cover up the "Loading..." propeller */
  bacon_outlni (0, "%s:         ", bacon_rom_type_str (rom_type_i));
  rom = list->roms[rom_type_i];
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
bacon_get_rom_by_index (const BaconRomList *list, const int index)
{
  int idx;
  BaconRom *rom;

  rom = list->roms[rom_type_i];
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

static void
bacon_copyright (void)
{
  static const char *const text[] = {
    "",
    BACON_PROGRAM_NAME " Copyright (C) 2013 Nathan Forbes",
    "This program comes with ABSOLUTELY NO WARRANTY; for details type `d'.",
    "This is free software, and you are welcome to redistribute it",
    "under certain conditions; type `c' for details.",
    "",
    NULL
  };
  size_t x;

  for (x = 0; text[x]; ++x)
    bacon_outln (text[x]);
}

static void
bacon_license (void)
{
  static const char *const text =
#include "bacon-license.h"
  ;

  bacon_outln (text);
}

static bool
bacon_all_currents_satisfied (void)
{
  if (device && rom && (rom_type_i != -1))
    return true;
  return false;
}

static void
bacon_currents (void)
{
  bacon_out ("Current device:   ");
  if (device)
    bacon_out ("%s [%s]", device->fullname, device->codename);
  bacon_outln (NULL);

  bacon_outln ("Current ROM Type: %s",
      (rom_type_i != -1) ? bacon_rom_type_str (rom_type_i) : "(none)");

  bacon_outln ("Current ROM:      %s",
      (rom) ? rom->name : "(none)");

  if (bacon_all_currents_satisfied ())
    bacon_outln ("Ready to download!");
}

static void
bacon_choose_device (void)
{
  bacon_display_device_choices ();
  while (true) {
    bacon_out ("Enter choice: [1-%i or codename] ",
        bacon_device_list_total (g_device_list));
    bacon_get_device_answer ();
    if (device)
      break;
    bacon_outln ("Invalid response - please try again...");
  }
}

static void
bacon_choose_rom_type (void)
{
  bacon_display_rom_type_choices ();
  while (true) {
    bacon_out ("Enter number: [1-%i] ", BACON_ROM_TOTAL);
    if (!bacon_get_num_answer (&rom_type_i))
      continue;
    --rom_type_i;
    if (rom_type_i == BACON_ROM_NIGHTLY)
      g_rom_type |= BACON_ROM_TYPE_NIGHTLY;
    else if (rom_type_i == BACON_ROM_RC)
      g_rom_type |= BACON_ROM_TYPE_RC;
    else if (rom_type_i == BACON_ROM_SNAPSHOT)
      g_rom_type |= BACON_ROM_TYPE_SNAPSHOT;
    else if (rom_type_i == BACON_ROM_STABLE)
      g_rom_type |= BACON_ROM_TYPE_STABLE;
    else if (rom_type_i == BACON_ROM_TEST)
      g_rom_type |= BACON_ROM_TYPE_TEST;
    else {
      bacon_warn ("'%i' not valid - please try again...", rom_type_i);
      continue;
    }
    break;
  }
}

static void
bacon_choose_rom (void)
{
  int idx;
  int total;

  g_max_roms = 20;
  bacon_rom_list_destroy (rom_list);
  rom_list =
    bacon_rom_list_new (device->codename, g_rom_type, g_max_roms);
  total = 1;

  bacon_display_rom_choices (rom_list, &total);
  if (!total)
    return;

  while (true) {
    bacon_out ("Enter number: [1-%i] ", total);
    if (!bacon_get_num_answer (&idx))
      continue;
    rom = bacon_get_rom_by_index (rom_list, idx - 1);
    if (!rom) {
      bacon_warn ("'%i' not valid - please try again...", idx);
      continue;
    }
    break;
  }
}

static void
bacon_specify_download_path (void)
{
  size_t n;
  char answer[BACON_PATH_MAX];

  while (true) {
    bacon_out ("Enter path [without file basename]: ");
    if (!fgets (answer, BACON_PATH_MAX, stdin)) {
      bacon_error ("failed to read from standard input");
      bacon_do_exit ();
    }
    n = strlen (answer);
    if (answer[n - 1] == '\n')
      answer[n - 1] = '\0';
    if (!bacon_env_is_file (answer))
      break;
    bacon_outln ("`%s' exists and is a file - try again", answer);
  }

  if (!bacon_env_ensure_path (answer, false)) {
    bacon_debug ("BUG: bacon_env_ensure_path (\"%s\", false) == false",
                 answer);
    bacon_do_exit ();
  }

  dirpath = bacon_strdup (answer);
}

static void
bacon_download (void)
{
  bool ret;

  bacon_outln ("\n==========================================");
  bacon_outln ("device:      %s [%s]",
               device->fullname,
               device->codename);
  bacon_outln ("type:        %s", bacon_rom_type_str (rom_type_i));
  bacon_outln ("filename:    %s", rom->name);
  bacon_outln ("released on: %s", rom->date);
  bacon_outln ("size:        %s", rom->size);
  bacon_outln ("url:         %s/%s", BACON_GET_CM_URL, rom->get);
  bacon_outln ("md5:         %s", rom->hash.hash);
  if (dirpath)
    bacon_outln ("saving to:   %s%c%s", dirpath, BACON_PATH_SEP, rom->name);
  else
    bacon_outln ("saving to:   %s", (!g_out_path) ? "." : g_out_path);
  bacon_outln ("==========================================\n");

  if (dirpath)
    ret = bacon_rom_do_download (rom, dirpath);
  else
    ret = bacon_rom_do_download (rom, g_out_path);

  if (!ret)
    bacon_do_exit ();
}

static void
bacon_main_menu (void)
{
  static const char *const text[] = {
    "",
    "Main Menu",
    "  1) Choose device",
    "  2) Choose ROM Type",
    "  3) Choose ROM",
    "  4) Specify download path",
    "  5) Download",
    "  6) Copyright",
    "  7) License",
    "  8) Exit",
    "",
    NULL
  };
  int choice;
  size_t x;
  bool stop;

  stop = false;
  while (true) {
    for (x = 0; text[x]; ++x)
      bacon_outln (text[x]);
    bacon_currents ();
    bacon_out ("Enter choice: [1-8] ");
    if (!bacon_get_num_answer (&choice))
      continue;
    switch (choice) {
    case 1:
      if (device || rom || (rom_type_i != -1)) {
        bacon_outln ("Resetting previously selected choices...");
        rom_type_i = -1;
        device = NULL;
        rom = NULL;
      }
      bacon_choose_device ();
      break;
    case 2:
      if (!device)
        bacon_outln ("Specify a device first!");
      else
        bacon_choose_rom_type ();
      break;
    case 3:
      if (!device || (rom_type_i == -1)) {
        if (!device)
          bacon_outln ("You must specify a device first!");
        if (rom_type_i == -1)
          bacon_outln ("You must specify a ROM type first!");
      } else
        bacon_choose_rom ();
      break;
    case 4:
      bacon_specify_download_path ();
      break;
    case 5:
      if (!bacon_all_currents_satisfied ()) {
        bacon_outln ("Not enough information!");
        bacon_outln ("Still need:");
        if (!device)
          bacon_outln ("  device of choice");
        if (rom_type_i == -1)
          bacon_outln ("  rom type of choice");
        if (!rom)
          bacon_outln ("  rom of choice");
      } else
        bacon_download ();
      break;
    case 6:
      bacon_copyright ();
      break;
    case 7:
      bacon_license ();
      break;
    case 8:
      stop = true;
      break;
    default:
      bacon_outln ("Invalid reponse - please try again");
      break;
    }
    if (stop)
      break;
    bacon_out ("Press any key to continue ");
    if (fgetc (stdin) == EOF) {
      bacon_error ("BUG: fgetc (stdin) == EOF");
      bacon_do_exit ();
    }
  }
}

void
bacon_interactive (void)
{
  bacon_main_menu ();
  bacon_do_cleanup ();
}

