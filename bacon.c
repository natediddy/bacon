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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-inter.h"
#include "bacon-net.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_VERSION_BUFFER_MAX 12
#define BACON_DEFAULT_MAX_ROMS    5
#define BACON_DEVICES_MAX       512

const char *g_program_name;

static struct {
  char id[BACON_DEVICE_NAME_MAX];
} devices[BACON_DEVICES_MAX];

BaconDeviceList *       device_list        = NULL;
char *                  output_download    = NULL;
int                     max_roms           = BACON_DEFAULT_MAX_ROMS;
int                     rom_type           = BACON_ROM_TYPE_NONE;

static bool             list_all_devices   = false;
static bool             update_device_list = false;
static bool             latest             = false;
static bool             downloading        = false;
static bool             showing            = false;
static bool             show_hash          = false;
static bool             show_url           = false;
static bool             interactive        = false;
static bool             all_types          = false;
static bool             experimental_type  = false;
static bool             snapshot_type      = false;
static bool             nightly_type       = false;
static bool             rc_type            = false;
static bool             stable_type        = false;

static void
bacon_usage (const bool error)
{
  bacon_foutln ((!error) ? stdout : stderr,
                "Usage: %s [OPTION...] [DEVICE...]",
                g_program_name);
 
}

static void
bacon_help (void)
{
  bacon_usage (false);
  bacon_outln ("\
Options:\n\
  -d, --download             Download ROM for DEVICE\n\
  -M N, --max-roms=N         Show a max of N ROMs per ROM type (with\n\
                             `--show')\n\
  -l, --list-devices         List all available devices\n\
  -L, --latest               Show or download the latest ROM for DEVICE\n\
                             (with the action of showing or downloading\n\
                             depending on whether `--show' or `--download'\n\
                             was used)\n\
  -s, --show                 Only show ROM(s) for DEVICE (don't download\n\
                             them)\n\
  -H, --show-hash            Show the MD5 hash (with `--show')\n\
  -i, --interactive          Interactive mode\n\
  -a, --all                  Specify all ROM types\n\
  -e, --experimental         Specify only Experimental\n\
  -m, --snapshot             Specify only M Snapshot ROMs\n\
  -n, --nightly              Specify only Nightly ROMs\n\
  -r, --rc                   Specify only Release Candidate ROMs\n\
  -S, --stable               Specify only Stable ROMs\n\
  -u, --update-device-list   Update the local device list\n\
  -U, --show-url             Show the URL (with `--show')\n\
  -o FILENAME, --output=FILENAME\n\
                             Download ROM to FILENAME\n\
  -?, -h, --help             Show this help message and exit\n\
  -v, --version              Show version info and exit\
");
  exit (EXIT_SUCCESS);
}

static void
bacon_version (void)
{
  bacon_outln (BACON_PROGRAM_NAME " " BACON_VERSION "\n"
               "Written by Nathan Forbes");
  exit (EXIT_SUCCESS);
}

static void
bacon_set_program_name (const char *argv0)
{
  char *x;

  if (!argv0 || !*argv0) {
    g_program_name = BACON_PROGRAM_NAME;
    return;
  }

  x = strrchr (argv0, '/');
  if (x && x[0] && x[1]) {
    g_program_name = ++x;
    return;
  }

#ifdef BACON_OS_WINDOWS
  x = strrchr (argv0, '\\');
  if (x && x[0] && x[1]) {
    g_program_name = ++x;
    return;
  }
#endif
  g_program_name = argv0;
}

static void
bacon_cleanup (void)
{
  if (device_list)
    bacon_device_list_destroy (device_list);
  bacon_free (output_download);
}

static void
bacon_list_all_devices (void)
{
  int n;
  BaconDeviceList *p;

  n = 0;
  bacon_outln ("Available devices (%i total):",
               bacon_device_list_total (device_list));
  for (p = device_list; p; p = p->next) {
    bacon_out ("  %4i) ", ++n);
    if (*p->device->codename)
      bacon_out (p->device->codename);
    if (*p->device->fullname)
      bacon_out (" - %s", p->device->fullname);
    bacon_outln (NULL);
    if (!p->next)
      break;
  }
}

static bool
bacon_set_max_roms_from_arg (const char *arg)
{
  size_t x;

  for (x = 0; arg[x]; ++x)
    if (!isdigit (arg[x]))
      return false;
  max_roms = bacon_int_from_str (arg);
  if (max_roms <= 0)
    return false;
  return true;
}

static void
bacon_check_opts (void)
{
  if (rom_type == BACON_ROM_TYPE_NONE)
    rom_type |= BACON_ROM_TYPE_ALL;

  if (interactive &&
      (showing ||
       downloading ||
       latest ||
       output_download))
  {
    bacon_error ("`--interactive' and `--%s' are mutually exclusive "
                 "(try `--help')",
                 (showing) ? "show" :
                 (downloading) ? "download" :
                 (latest) ? "latest" :
                 "output");
    exit (EXIT_FAILURE);
  }

  if (showing && downloading) {
    bacon_error ("`--download' and `--show' are mutually exclusive");
    exit (EXIT_FAILURE);
  }

  if (!showing && !downloading)
    showing = true;

  if (downloading)
    latest = true;

  if (output_download && *output_download) {
    if (showing) {
      bacon_error ("`--output' and `--show' are mutually exclusive");
      exit (EXIT_FAILURE);
    }
    if (interactive) {
      bacon_error ("`--output' and `--interactive' are mutually exclusive");
      exit (EXIT_FAILURE);
    }
    if (!latest)
      latest = true;
    if (!downloading)
      downloading = true;
  }

  if (!interactive &&
      !*devices[0].id &&
      !update_device_list &&
      !list_all_devices)
  {
    bacon_error ("no devices given (try `--help')");
    exit (EXIT_FAILURE);
  }

  if (latest)
    max_roms = 1;
}

static void
bacon_parse_string_for_short_opts (const char *s)
{
  size_t x;

  for (x = 1; s[x]; ++x) {
    switch (s[x]) {
    case '?':
    case 'h':
    case 'M':
    case 'v':
    case 'o':
      bacon_error ("invalid context for `-%c' (try `--help')", s[x]);
      exit (EXIT_FAILURE);
    case 'd':
      downloading = true;
      break;
    case 'H':
      show_hash = true;
      break;
    case 'i':
      interactive = true;
      break;
    case 'l':
      list_all_devices = true;
      break;
    case 'L':
      latest = true;
      break;
    case 's':
      showing = true;
      break;
    case 'a':
      rom_type |= BACON_ROM_TYPE_ALL;
      break;
    case 'e':
      rom_type |= BACON_ROM_TYPE_TEST;
      break;
    case 'm':
      rom_type |= BACON_ROM_TYPE_SNAPSHOT;
      break;
    case 'n':
      rom_type |= BACON_ROM_TYPE_NIGHTLY;
      break;
    case 'r':
      rom_type |= BACON_ROM_TYPE_RC;
      break;
    case 'S':
      rom_type |= BACON_ROM_TYPE_STABLE;
      break;
    case 'u':
      update_device_list = true;
      break;
    case 'U':
      show_url = true;
      break;
    default:
      bacon_error ("`-%c' is an unrecognized option (try `--help')", s[x]);
      exit (EXIT_FAILURE);
    }
  }
}

static void
bacon_parse_opt (char **v)
{
  size_t x;
  size_t n;
  size_t pos;
  char *o;

  pos = 0;
  bacon_set_program_name (v[0]);

  for (x = 1; v[x]; ++x) {
    if (bacon_streq (v[x], "-?") ||
        bacon_streq (v[x], "-h") ||
        bacon_streq (v[x], "--help"))
      bacon_help ();
    else if (bacon_streq (v[x], "-v") || bacon_streq (v[x], "--version"))
      bacon_version ();
    else if (bacon_streq (v[x], "-H") || bacon_streq (v[x], "--show-hash"))
      show_hash = true;
    else if (bacon_streq (v[x], "-l") || bacon_streq (v[x], "--list-devices"))
      list_all_devices = true;
    else if (bacon_streq (v[x], "-L") || bacon_streq (v[x], "--latest"))
      latest = true;
    else if (bacon_streq (v[x], "-d") || bacon_streq (v[x], "--download"))
      downloading = true;
    else if (bacon_streq (v[x], "-M") || bacon_streq (v[x], "--max-roms")) {
      if (!v[x + 1]) {
        bacon_error ("`%s' requires an argument (try `--help')", v[x]);
        exit (EXIT_FAILURE);
      }
      if (!bacon_set_max_roms_from_arg (v[++x])) {
        bacon_error ("'%s' is not a valid argument for `%s' (try `--help')",
                     v[x], v[x - 1]);
        exit (EXIT_FAILURE);
      }
    } else if (bacon_strstw (v[x], "-M")) {
      o = v[x] + 2;
      if (!o || !*o) {
        bacon_error ("`-M' requires an argument (try `--help')");
        exit (EXIT_FAILURE);
      }
      if (!bacon_set_max_roms_from_arg (o)) {
        bacon_error ("'%s' is not a valid argument for `-M' (try `--help')",
                     o);
        exit (EXIT_FAILURE);
      }
    } else if (bacon_strstw (v[x], "--max-roms=")) {
      o = strchr (v[x], '=');
      ++o;
      if (!o || !*o) {
        bacon_error ("`--max-roms' requires an argument (try `--help')");
        exit (EXIT_FAILURE);
      }
      if (!bacon_set_max_roms_from_arg (o)) {
        bacon_error ("'%s' is not a valid argument for `--max-roms' "
                     "(try `--help')", o);
        exit (EXIT_FAILURE);
      }
    } else if (bacon_streq (v[x], "-i") ||
               bacon_streq (v[x], "--interactive"))
      interactive = true;
    else if (bacon_streq (v[x], "-s") || bacon_streq (v[x], "--show"))
      showing = true;
    else if (bacon_streq (v[x], "-a") || bacon_streq (v[x], "--all"))
      rom_type |= BACON_ROM_TYPE_ALL;
    else if (bacon_streq (v[x], "-e") || bacon_streq (v[x], "--experimental"))
      rom_type |= BACON_ROM_TYPE_TEST;
    else if (bacon_streq (v[x], "-m") || bacon_streq (v[x], "--snapshot"))
      rom_type |= BACON_ROM_TYPE_SNAPSHOT;
    else if (bacon_streq (v[x], "-n") || bacon_streq (v[x], "--nightly"))
      rom_type |= BACON_ROM_TYPE_NIGHTLY;
    else if (bacon_streq (v[x], "-r") || bacon_streq (v[x], "--rc"))
      rom_type |= BACON_ROM_TYPE_RC;
    else if (bacon_streq (v[x], "-S") || bacon_streq (v[x], "--stable"))
      rom_type |= BACON_ROM_TYPE_STABLE;
    else if (bacon_streq (v[x], "-u") ||
             bacon_streq (v[x], "--update-device-list"))
      update_device_list = true;
    else if (bacon_streq (v[x], "-U") || bacon_streq (v[x], "--show-url"))
      show_url = true;
    else if (bacon_streq (v[x], "-o") || bacon_streq (v[x], "--output")) {
      if (!v[x + 1] || v[x + 1][0] == '-') {
        bacon_error ("`%s' requires an argument (try `--help')", v[x]);
        exit (EXIT_FAILURE);
      }
      output_download = bacon_strdup (v[++x]);
    } else if (bacon_strstw (v[x], "--output=")) {
      o = strchr (v[x], '=');
      ++o;
      if (!o || !*o) {
        bacon_error ("`--output' requires an argument (try `--help')");
        exit (EXIT_FAILURE);
      }
      output_download = bacon_strdup (o);
    } else if (v[x][0] == '-') {
      if (!v[x][1] || v[x][1] == '-') {
        bacon_error ("`%s' is an unrecognized option (try `--help')");
        exit (EXIT_FAILURE);
      }
      bacon_parse_string_for_short_opts (v[x]);
    } else if (pos < BACON_DEVICES_MAX) {
      n = strlen (v[x]);
      strncpy (devices[pos].id, v[x], n);
      devices[pos++].id[n] = '\0';
    }
  }

  devices[pos].id[0] = '\0';
  bacon_check_opts ();
}

static void
bacon_show_rom_list (const BaconDevice *device, const BaconRomList *list)
{
  int n;
  int x;
  BaconRom *rom;

  bacon_outlni (0, "%s [%s]:", device->fullname, device->codename);
  for (x = 0; x < BACON_ROM_TOTAL; ++x) {
    rom = list->roms[x];
    if (!rom)
      continue;
    bacon_outlni (1, "%s%s:",
                  (!latest) ? "" : "Latest ",
                  bacon_rom_type_str (x));
    n = 0;
    for (; rom; rom = rom->next) {
      if (!latest)
        bacon_outlni (2, "%i) %s", n + 1, rom->name);
      else
        bacon_outlni (2, "%s", rom->name);
      bacon_outlni (3, "released: %s", rom->date);
      bacon_outlni (3, "size:     %s", rom->size);
      if (show_hash)
        bacon_outlni (3, "hash:     %s", rom->hash.hash);
      if (show_url)
        bacon_outlni (3, "url:      " BACON_ROOT_URL "/%s", rom->get);
      if (!rom->next)
        break;
      ++n;
    }
  }
}

static void
bacon_download_rom (const BaconDevice *device, const BaconRom *rom)
{
  if (rom && !bacon_rom_do_download (rom, output_download))
    exit (EXIT_FAILURE);
}

static void
bacon_check_given_devices (void)
{
  size_t x;
  bool bad_device;

  bad_device = false;
  for (x = 0; *devices[x].id; ++x) {
    if (!bacon_device_is_valid_id (device_list, devices[x].id)) {
      bacon_error ("'%s' is not a valid device", devices[x].id);
      if (!bad_device)
        bad_device = true;
    }
  }

  if (bad_device) {
    bacon_error ("try `--list-devices' for all available devices "
                 "supported by CyanogenMod");
    exit (EXIT_FAILURE);
  }
}

static void
bacon_perform (void)
{
  int x;
  size_t pos;
  BaconDevice *device;
  BaconRomList *rom_list;

  if (!device_list)
    device_list = bacon_device_list_new (update_device_list);

  if (interactive) {
    bacon_interactive ();
    return;
  }

  if (list_all_devices)
    bacon_list_all_devices ();

  if (*devices[0].id)
    bacon_check_given_devices ();

  for (pos = 0; *devices[pos].id; ++pos) {
    device = bacon_device_get_device_from_id (device_list, devices[pos].id);
    rom_list = bacon_rom_list_new (device->codename, rom_type, max_roms);
    if (showing)
      bacon_show_rom_list (device, rom_list);
    else if (downloading && latest)
      for (x = 0; x < BACON_ROM_TOTAL; ++x)
        bacon_download_rom (device, rom_list->roms[x]);
    bacon_rom_list_destroy (rom_list);
  }
}

int
main (const int argc, char **argv)
{
  atexit (bacon_cleanup);
  bacon_parse_opt (argv);
  bacon_perform ();
  exit (EXIT_SUCCESS);
  return 0;
}

