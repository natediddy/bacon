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

#include <string.h>

#include "bacon-colors.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-hash.h"
#include "bacon-inter.h"
#include "bacon-net.h"
#include "bacon-out.h"
#include "bacon-rom.h"
#include "bacon-str.h"
#include "bacon-util.h"

#define BACON_ANSWER_MAX     BACON_DEVICE_NAME_MAX

extern BaconDeviceList *g_device_list;
extern char *           g_out_path;
extern int              g_max_roms;
extern int              g_rom_type;
static BaconRomList *   s_rom_list   = NULL;
static BaconRom *       s_rom        = NULL;
static BaconDevice *    s_device     = NULL;
static char *           s_dirpath    = NULL;
static int              s_rom_type_i = -1;

static void
bacon_do_cleanup (void)
{
  bacon_free (s_dirpath);
  bacon_rom_list_destroy (s_rom_list);
}

static void
bacon_do_exit (void)
{
  bacon_do_cleanup ();
  exit (EXIT_FAILURE);
}

static BaconBoolean
bacon_get_num_answer (int *input)
{
  size_t n;
  char answer[BACON_ANSWER_MAX];

  if (!fgets (answer, BACON_ANSWER_MAX, stdin)) {
    bacon_error ("failed to read from standard input");
    bacon_do_exit ();
  }
  *input = bacon_strtoint (answer);
  if ((*input == 0) && *answer) {
    n = strlen (answer);
    if (answer[n - 1] == '\n')
      answer[n - 1] = '\0';
    bacon_warn ("'%s' not valid - please try again...", answer);
    return BACON_FALSE;
  }
  return BACON_TRUE;
}

static void
bacon_set_device_from_index (int *idx)
{
  int cidx;
  BaconDeviceList *p;

  cidx = 1;
  s_device = NULL;
  for (p = g_device_list; p; p = p->next) {
    if (cidx == *idx) {
      s_device = p->device;
      break;
    }
    if (!p->next)
      break;
    cidx++;
  }
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
    s_device = bacon_device_get_device_from_id (g_device_list, answer);
    return;
  }

  idx = bacon_strtoint (answer);
  bacon_set_device_from_index (&idx);
}

static void
bacon_display_device_choices (void)
{
  int i;
  int n;
  int n_total_digits;
  BaconDeviceList *p;

  n = 1;
  n_total_digits = bacon_ndigits (bacon_device_list_total (g_device_list));

  bacon_outln ("Devices:");
  for (p = g_device_list; p; p = p->next) {
    bacon_outi (1, NULL);
    for (i = bacon_ndigits (n); i < n_total_digits; ++i)
      bacon_outc (' ');
    bacon_outln ("%s) %s - %s",
                 BACON_COLOR_I (BACON_NUMBER_LIST_COLOR, n),
                 BACON_COLOR_S (BACON_CODENAME_COLOR, p->device->codename),
                 BACON_COLOR_S (BACON_FULLNAME_COLOR, p->device->fullname));
    if (!p->next)
      break;
    ++n;
  }
}

static void
bacon_display_rom_type_choices (void)
{
  int i;
  int n;
  int n_total_digits;

  n_total_digits = bacon_ndigits (BACON_ROM_TOTAL);

  bacon_outln ("ROM Types:");
  for (n = 0; n < BACON_ROM_TOTAL; ++n) {
    bacon_outi (1, NULL);
    for (i = bacon_ndigits (n + 1); i < n_total_digits; ++i)
      bacon_outc (' ');
    bacon_outln ("%s) %s",
                 BACON_COLOR_I (BACON_NUMBER_LIST_COLOR, n + 1),
                 BACON_COLOR_S (BACON_ROM_TYPE_COLOR,
                                bacon_rom_type_str (n)));
  }
}

static void
bacon_display_rom_choices (const BaconRomList *list, int *total_choices)
{
  int n;
  BaconRom *rom;

  /* add some extra spaces here to cover up the "Loading..." progress */
  bacon_outln ("%s:        ",
               BACON_COLOR_S (BACON_ROM_TYPE_COLOR,
                              bacon_rom_type_str (s_rom_type_i)));
  rom = list->roms[s_rom_type_i];
  if (rom) {
    *total_choices = bacon_rom_total (rom);
    n = 1;
    for (; rom; rom = rom->next) {
      bacon_outlni (1, "%s) %s",
                    BACON_COLOR_I (BACON_NUMBER_LIST_COLOR, n),
                    BACON_COLOR_S (BACON_ROM_NAME_COLOR, rom->name));
      bacon_outlni (2, "%s: %s",
                    BACON_COLOR_S (BACON_ROM_INFO_TAG_COLOR, "released"),
                    BACON_COLOR_S (BACON_ROM_INFO_COLOR, rom->date));
      bacon_outlni (2, "%s:     %s",
                    BACON_COLOR_S (BACON_ROM_INFO_TAG_COLOR, "size"),
                    BACON_COLOR_S (BACON_ROM_INFO_COLOR, rom->size));
      bacon_outlni (2, "%s:     %s",
                    BACON_COLOR_S (BACON_ROM_INFO_TAG_COLOR, "hash"),
                    BACON_COLOR_S (BACON_ROM_INFO_COLOR, rom->hash.hash));
      bacon_outlni (2, "%s:      %s%s%s",
                    BACON_COLOR_S (BACON_ROM_INFO_TAG_COLOR, "url"),
                    BACON_COLOR_S (BACON_ROM_INFO_COLOR, BACON_GET_CM_URL),
                    BACON_COLOR_C (BACON_ROM_INFO_COLOR, '/'),
                    BACON_COLOR_S (BACON_ROM_INFO_COLOR, rom->get));
      if (!rom->next)
        break;
      ++n;
    }
  } else {
    bacon_outlni (1, "%s", BACON_COLOR_S (BACON_BLUE, "None"));
    *total_choices = 0;
  }
}

static void
bacon_set_rom_by_index (const BaconRomList *list, int index)
{
  int idx;
  BaconRom *rom;

  rom = list->roms[s_rom_type_i];
  if (rom) {
    idx = 0;
    for (; rom; rom = rom->next) {
      if (idx == index) {
        s_rom = rom;
        return;
      }
      if (!rom->next)
        break;
      ++idx;
    }
  }
  s_rom = NULL;
}

static void
bacon_copyright (void)
{
  static const char *const text[] = {
    "",
    BACON_PROGRAM_NAME " Copyright (C) 2013 Nathan Forbes",
    "This program comes with ABSOLUTELY NO WARRANTY.",
    "This is free software, and you are welcome to redistribute it",
    "under certain conditions",
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

static BaconBoolean
bacon_all_currents_satisfied (void)
{
  if (s_device && s_rom && (s_rom_type_i != -1))
    return BACON_TRUE;
  return BACON_FALSE;
}

static void
bacon_currents (void)
{
  bacon_out ("Current device:   ");
  if (s_device)
    bacon_outln ("%s [%s]",
                 BACON_COLOR_S (BACON_FULLNAME_COLOR, s_device->fullname),
                 BACON_COLOR_S (BACON_CODENAME_COLOR, s_device->codename));
  else
    bacon_outln ("");

  bacon_out ("Current ROM Type: ");
  if (s_rom_type_i != -1)
    bacon_outln ("%s",
                 BACON_COLOR_S (BACON_ROM_TYPE_COLOR,
                                bacon_rom_type_str (s_rom_type_i)));
  else
    bacon_outln ("");

  bacon_out ("Current ROM:      ");
  if (s_rom)
    bacon_outln ("%s", BACON_COLOR_S (BACON_ROM_NAME_COLOR, s_rom->name));
  else
    bacon_outln ("");

  if (bacon_all_currents_satisfied ())
    bacon_outln ("Ready to download!");
}

static void
bacon_choose_device (void)
{
  bacon_display_device_choices ();
  while (BACON_TRUE) {
    bacon_out ("%s: [%s-%s or %s]: ",
               BACON_COLOR_S (BACON_CYAN, "Enter choice"),
               BACON_COLOR_C (BACON_BOLD, '1'),
               BACON_COLOR_I (BACON_BOLD,
                              bacon_device_list_total (g_device_list)),
               BACON_COLOR_S (BACON_BOLD, "codename"));
    bacon_get_device_answer ();
    if (s_device)
      break;
    bacon_outln ("Invalid response - please try again...");
  }
}

static void
bacon_choose_rom_type (void)
{
  bacon_display_rom_type_choices ();
  while (BACON_TRUE) {
    bacon_out ("%s: [%s-%s]: ",
               BACON_COLOR_S (BACON_CYAN, "Enter number"),
               BACON_COLOR_C (BACON_BOLD, '1'),
               BACON_COLOR_I (BACON_BOLD, BACON_ROM_TOTAL));
    if (!bacon_get_num_answer (&s_rom_type_i))
      continue;
    --s_rom_type_i;
    if (s_rom_type_i == BACON_ROM_NIGHTLY)
      g_rom_type |= BACON_ROM_TYPE_NIGHTLY;
    else if (s_rom_type_i == BACON_ROM_RC)
      g_rom_type |= BACON_ROM_TYPE_RC;
    else if (s_rom_type_i == BACON_ROM_SNAPSHOT)
      g_rom_type |= BACON_ROM_TYPE_SNAPSHOT;
    else if (s_rom_type_i == BACON_ROM_STABLE)
      g_rom_type |= BACON_ROM_TYPE_STABLE;
    else if (s_rom_type_i == BACON_ROM_TEST)
      g_rom_type |= BACON_ROM_TYPE_TEST;
    else {
      bacon_warn ("'%s' not valid - please try again...",
                  BACON_COLOR_I (BACON_BOLD, s_rom_type_i));
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

  total = 0;
  g_max_roms = 20;
  bacon_rom_list_destroy (s_rom_list);
  s_rom_list = bacon_rom_list_new (s_device->codename,
                                   g_rom_type,
                                   g_max_roms);

  bacon_display_rom_choices (s_rom_list, &total);
  if (!total)
    return;

  while (BACON_TRUE) {
    bacon_out ("%s: [%s-%s]: ",
               BACON_COLOR_S (BACON_CYAN, "Enter number"),
               BACON_COLOR_C (BACON_BOLD, '1'),
               BACON_COLOR_I (BACON_BOLD, total));
    if (!bacon_get_num_answer (&idx))
      continue;
    bacon_set_rom_by_index (s_rom_list, idx - 1);
    if (!s_rom) {
      bacon_warn ("'%s' not valid - please try again...",
                  BACON_COLOR_I (BACON_BOLD, idx));
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

  while (BACON_TRUE) {
    bacon_out ("%s [%s]: ",
               BACON_COLOR_S (BACON_CYAN, "Enter path"),
               BACON_COLOR_S (BACON_BOLD, "without basename"));
    if (!fgets (answer, BACON_PATH_MAX, stdin)) {
      bacon_error ("failed to read from standard input");
      bacon_do_exit ();
    }
    n = strlen (answer);
    if (answer[n - 1] == '\n')
      answer[n - 1] = '\0';
    if (!bacon_env_is_file (answer))
      break;
    bacon_outln ("`%s' exists and is a file - try again",
                 BACON_COLOR_S (BACON_BOLD, answer));
  }

  if (!bacon_env_ensure_path (answer, BACON_FALSE)) {
    bacon_debug ("BUG: bacon_env_ensure_path (\"%s\", BACON_FALSE) == "
                   "BACON_FALSE",
                 answer);
    bacon_do_exit ();
  }

  s_dirpath = bacon_strdup (answer);
}

static void
bacon_download (void)
{
  BaconBoolean ret;

  bacon_outln ("\n%s",
               BACON_COLOR_S (BACON_BOLD,
                              "=========================================="));
  bacon_outln ("%s:      %s [%s]",
               BACON_COLOR_S (BACON_BOLD, "device"),
               BACON_COLOR_S (BACON_FULLNAME_COLOR, s_device->fullname),
               BACON_COLOR_S (BACON_CODENAME_COLOR, s_device->codename));
  bacon_outln ("%s:        %s",
               BACON_COLOR_S (BACON_BOLD, "type"),
               BACON_COLOR_S (BACON_ROM_TYPE_COLOR,
                              bacon_rom_type_str (s_rom_type_i)));
  bacon_outln ("%s:    %s",
               BACON_COLOR_S (BACON_BOLD, "filename"),
               BACON_COLOR_S (BACON_ROM_NAME_COLOR, s_rom->name));
  bacon_outln ("%s: %s",
               BACON_COLOR_S (BACON_BOLD, "released on"),
               BACON_COLOR_S (BACON_ROM_INFO_COLOR, s_rom->date));
  bacon_outln ("%s:        %s",
               BACON_COLOR_S (BACON_BOLD, "size"),
               BACON_COLOR_S (BACON_ROM_INFO_COLOR, s_rom->size));
  bacon_outln ("%s:         %s%s%s",
               BACON_COLOR_S (BACON_BOLD, "url"),
               BACON_COLOR_S (BACON_ROM_INFO_COLOR, BACON_GET_CM_URL),
               BACON_COLOR_C (BACON_ROM_INFO_COLOR, '/'),
               BACON_COLOR_S (BACON_ROM_INFO_COLOR, s_rom->get));
  bacon_outln ("%s:         %s",
               BACON_COLOR_S (BACON_BOLD, "md5"),
               BACON_COLOR_S (BACON_ROM_INFO_COLOR, s_rom->hash.hash));
  bacon_out ("%s:   ", BACON_COLOR_S (BACON_BOLD, "saving to"));
  if (s_dirpath)
    bacon_outln ("%s%s%s",
                 BACON_COLOR_S (BACON_YELLOW, s_dirpath),
                 BACON_COLOR_C (BACON_YELLOW, BACON_PATH_SEP),
                 BACON_COLOR_S (BACON_YELLOW, s_rom->name));
  else
    bacon_outln ("%s",
                 (!g_out_path) ?
                   BACON_COLOR_C (BACON_YELLOW, '.') :
                   BACON_COLOR_S (BACON_YELLOW, g_out_path));
  bacon_outln ("%s",
               BACON_COLOR_S (BACON_BOLD,
                              "=========================================="));

  if (s_dirpath)
    ret = bacon_rom_do_download (s_rom, s_dirpath);
  else
    ret = bacon_rom_do_download (s_rom, g_out_path);

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
  BaconBoolean stop;

  stop = BACON_FALSE;
  while (BACON_TRUE) {
    for (x = 0; text[x]; ++x)
      bacon_outln (text[x]);
    bacon_currents ();
    bacon_out ("Enter choice: [1-8] ");
    if (!bacon_get_num_answer (&choice))
      continue;
    switch (choice) {
    case 1:
      if (s_device || s_rom || (s_rom_type_i != -1)) {
        bacon_outln ("Resetting previously selected choices...");
        s_rom_type_i = -1;
        s_device = NULL;
        s_rom = NULL;
      }
      bacon_choose_device ();
      break;
    case 2:
      if (!s_device)
        bacon_outln ("Specify a device first!");
      else
        bacon_choose_rom_type ();
      break;
    case 3:
      if (!s_device || (s_rom_type_i == -1)) {
        if (!s_device)
          bacon_outln ("You must specify a device first!");
        if (s_rom_type_i == -1)
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
        if (!s_device)
          bacon_outln ("  device of choice");
        if (s_rom_type_i == -1)
          bacon_outln ("  rom type of choice");
        if (!s_rom)
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
      stop = BACON_TRUE;
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

