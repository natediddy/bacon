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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-env.h"
#ifdef BACON_GTK
# include "bacon-gtk.h"
#endif
#include "bacon-inter.h"
#include "bacon-net.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_DEFAULT_MAX_ROMS 3
#define BACON_DEVICES_MAX    512
#define BACON_OPT_MAX       1024

typedef enum {
  BACON_OT_DOWNLOAD,
  BACON_OT_INTERACTIVE,
  BACON_OT_LIST_DEVICES,
  BACON_OT_NO_PROGRESS,
  BACON_OT_SHOW,
  BACON_OT_ALL,
  BACON_OT_EXPERIMENTAL,
  BACON_OT_SNAPSHOT,
  BACON_OT_NIGHTLY,
  BACON_OT_RC,
  BACON_OT_STABLE,
  BACON_OT_OUTPUT,
  BACON_OT_HASH,
  BACON_OT_LATEST,
  BACON_OT_MAX,
  BACON_OT_URL
} BaconOptionType;

extern char *g_program_data_path;

/* global variables */
const char *            g_program_name;
BaconDeviceList *       g_device_list      = NULL;
char *                  g_out_path         = NULL;
int                     g_max_roms         = BACON_DEFAULT_MAX_ROMS;
int                     g_rom_type         = BACON_ROM_TYPE_NONE;
bool                    g_show_progress    = true;

/* local variables */
static char *           opt               [BACON_OPT_MAX];
static size_t           opt_pos            = 0;
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

static struct {
  char id[BACON_DEVICE_NAME_MAX];
} devices[BACON_DEVICES_MAX];

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
  /* each item in array gets its own line */
  static const char *const help[] = {
    "General Options:",
    "  -d, --download             Download the latest ROM for DEVICE",
    "                             Requires specific ROM type option",
    "                             (See 'ROM Type Options' below)",
    "  -i, --interactive          Interactive mode",
    "  -l, --list-devices         List all available DEVICEs",
    "  -N, --no-progress          Do not show any progress when retrieving",
    "                             data from the internet (this includes the",
    "                             progress bar during ROM downloads)",
    "  -s, --show                 Show ROMs for DEVICE (no downloading)",
    "  -u, --update-device-list   Update the local DEVICE list",
    "  -?, -h, --help             Display this help text and exit",
    "  -v, --version              Display version information and exit",
    "ROM Type Options:",
    "  -a, --all                  Specify all ROM types [default when no",
    "                             ROM types are given]",
    "  -e, --experimental         Specify only Experimental",
    "  -m, --snapshot             Specify only M Snapshot ROMs",
    "  -n, --nightly              Specify only Nightly ROMs",
    "  -r, --rc                   Specify only Release Candidate ROMs",
    "  -S, --stable               Specify only Stable ROMs",
    "Download Options:",
    "  -o PATH, --output=PATH     Save downloaded ROM to PATH",
    "Show Options:",
    "  -H, --hash                 Show remote MD5 hash for each ROM",
    "                             displayed",
    "  -L, --latest               Show only the latest ROM for each ROM type",
    "                             specified",
    "  -M N, --max=N              Show a maximum of N ROMs for each ROM type",
    "                             specified",
    "  -U, --url                  Show download URL for each ROM displayed",
#ifdef BACON_GTK
    "",
    "Providing no arguments will launch the GTK+ graphical user interface.",
#endif
    NULL
  };
  size_t x;

  bacon_usage (false);
  for (x = 0; help[x]; ++x)
    bacon_outln (help[x]);
  exit (EXIT_SUCCESS);
}

static void
bacon_version (void)
{
  /* each item in array gets its own line */
  static const char *const version[] = {
    BACON_PROGRAM_NAME " " BACON_VERSION,
    "Copyright (C) 2013 Nathan Forbes <" BACON_BUG_REPORT_EMAIL ">",
    "This is free software; see the source for copying conditions.",
    "There NO warranty; not even for MERCHANTABILITY or FITNESS FOR A",
    "PARTICULAR PURPOSE.",
#if defined (BACON_TARGET_SYSTEM) || defined (BACON_GTK)
    "",
# ifdef BACON_TARGET_SYSTEM
    "Built for: " BACON_TARGET_SYSTEM,
# endif
# ifdef BACON_GTK
    "Using GTK+ interface",
# endif
#endif
    NULL
  };
  size_t x;

  for (x = 0; version[x]; ++x)
    bacon_outln (version[x]);
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
  if (g_device_list)
    bacon_device_list_destroy (g_device_list);
  bacon_free (g_out_path);
  bacon_free (g_program_data_path);
}

static void
bacon_list_all_devices (void)
{
  int n;
  BaconDeviceList *p;

  n = 0;
  bacon_outln ("Available Devices:");
  for (p = g_device_list; p; p = p->next) {
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
  g_max_roms = bacon_int_from_str (arg);
  if (g_max_roms <= 0)
    return false;
  return true;
}

static char *
bacon_get_specific_option (BaconOptionType otype)
{
  size_t x;

  for (x = 0; opt[x]; ++x) {
    switch (otype) {
    case BACON_OT_DOWNLOAD:
      if (bacon_streq (opt[x], "-d") || bacon_streq (opt[x], "--download"))
        return opt[x];
      break;
    case BACON_OT_INTERACTIVE:
      if (bacon_streq (opt[x], "-i") || bacon_streq (opt[x], "--interactive"))
        return opt[x];
      break;
    case BACON_OT_LIST_DEVICES:
      if (bacon_streq (opt[x], "-l") ||
          bacon_streq (opt[x], "--list-devices"))
        return opt[x];
      break;
    case BACON_OT_NO_PROGRESS:
      if (bacon_streq (opt[x], "-N") || bacon_streq (opt[x], "--no-progress"))
        return opt[x];
      break;
    case BACON_OT_SHOW:
      if (bacon_streq (opt[x], "-s") || bacon_streq (opt[x], "--show"))
        return opt[x];
      break;
    case BACON_OT_ALL:
      if (bacon_streq (opt[x], "-a") || bacon_streq (opt[x], "--all"))
        return opt[x];
      break;
    case BACON_OT_EXPERIMENTAL:
      if (bacon_streq (opt[x], "-e") ||
          bacon_streq (opt[x], "--experimental"))
        return opt[x];
      break;
    case BACON_OT_SNAPSHOT:
      if (bacon_streq (opt[x], "-m") || bacon_streq (opt[x], "--snapshot"))
        return opt[x];
      break;
    case BACON_OT_NIGHTLY:
      if (bacon_streq (opt[x], "-n") || bacon_streq (opt[x], "--nightly"))
        return opt[x];
      break;
    case BACON_OT_RC:
      if (bacon_streq (opt[x], "-r") || bacon_streq (opt[x], "--rc"))
        return opt[x];
      break;
    case BACON_OT_STABLE:
      if (bacon_streq (opt[x], "-S") || bacon_streq (opt[x], "--stable"))
        return opt[x];
      break;
    case BACON_OT_OUTPUT:
      if (bacon_streq (opt[x], "-o") || bacon_strstw (opt[x], "--output"))
        return opt[x];
      break;
    case BACON_OT_HASH:
      if (bacon_streq (opt[x], "-H") || bacon_streq (opt[x], "--hash"))
        return opt[x];
      break;
    case BACON_OT_LATEST:
      if (bacon_streq (opt[x], "-L") || bacon_streq (opt[x], "--latest"))
        return opt[x];
      break;
    case BACON_OT_MAX:
      if (bacon_strstw (opt[x], "-M") || bacon_strstw (opt[x], "--max"))
        return opt[x];
      break;
    case BACON_OT_URL:
      if (bacon_streq (opt[x], "-U") || bacon_streq (opt[x], "--url"))
        return opt[x];
      break;
    default:
      ;
    }
  }
  return NULL;
}

static void
bacon_check_opts (void)
{
  if (!opt[0] && !*devices[0].id) {
    bacon_error ("nothing to do");
    goto error;
  }

  if (interactive && (g_rom_type != BACON_ROM_TYPE_NONE)) {
    if (g_rom_type & BACON_ROM_TYPE_ALL)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_ALL),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (g_rom_type & BACON_ROM_TYPE_TEST)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_EXPERIMENTAL),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (g_rom_type & BACON_ROM_TYPE_SNAPSHOT)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_SNAPSHOT),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (g_rom_type & BACON_ROM_TYPE_NIGHTLY)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_NIGHTLY),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (g_rom_type & BACON_ROM_TYPE_RC)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_RC),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (g_rom_type & BACON_ROM_TYPE_STABLE)
      bacon_error ("ROM type option `%s' cannot be used with `%s'",
                   bacon_get_specific_option (BACON_OT_STABLE),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    goto error;
  }

  if (downloading && showing) {
    bacon_error ("`%s' and `%s' are mutually exclusive",
                 bacon_get_specific_option (BACON_OT_DOWNLOAD),
                 bacon_get_specific_option (BACON_OT_SHOW));
    goto error;
  }

  if (interactive && (downloading || showing)) {
    if (downloading)
      bacon_error ("`%s' and `%s' are mutually exclusive",
                   bacon_get_specific_option (BACON_OT_DOWNLOAD),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    if (showing)
      bacon_error ("`%s' and `%s' are mutually exclusive",
                   bacon_get_specific_option (BACON_OT_SHOW),
                   bacon_get_specific_option (BACON_OT_INTERACTIVE));
    goto error;
  }

  if (downloading) {
    if (show_hash || show_url || bacon_get_specific_option (BACON_OT_MAX)) {
      if (show_hash)
        bacon_error ("`%s' cannot be used with `%s'",
                     bacon_get_specific_option (BACON_OT_HASH),
                     bacon_get_specific_option (BACON_OT_DOWNLOAD));
      if (show_url)
        bacon_error ("`%s' cannot be used with `%s'",
                     bacon_get_specific_option (BACON_OT_URL),
                     bacon_get_specific_option (BACON_OT_DOWNLOAD));
      if (bacon_get_specific_option (BACON_OT_MAX))
        bacon_error ("`%s' cannot be used with `%s",
                     bacon_get_specific_option (BACON_OT_MAX),
                     bacon_get_specific_option (BACON_OT_DOWNLOAD));
      goto error;
    } else
      latest = true;
  }

  if (showing && g_out_path) {
    bacon_error ("`%s' cannot be used with `%s'",
                 bacon_get_specific_option (BACON_OT_OUTPUT),
                 bacon_get_specific_option (BACON_OT_SHOW));
    goto error;
  }

  if (latest && bacon_get_specific_option (BACON_OT_MAX)) {
    bacon_error ("`%s' and `%s' are mutually exclusive",
                 bacon_get_specific_option (BACON_OT_LATEST),
                 bacon_get_specific_option (BACON_OT_MAX));
    goto error;
  }

  if (g_out_path && !downloading) {
    bacon_error ("`%s' can only be used with `-d'/`--download'",
                 bacon_get_specific_option (BACON_OT_OUTPUT));
    goto error;
  }

  if (!downloading && !showing)
    showing = true;

  if (g_rom_type == BACON_ROM_TYPE_NONE)
    g_rom_type |= BACON_ROM_TYPE_ALL;

  return;

error:
  bacon_error ("try `--help' for details");
  exit (EXIT_FAILURE);
}

static void
bacon_parse_string_for_short_opts (const char *s)
{
  size_t x;
  bool addopt;

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
      opt[opt_pos++] = "-d";
      break;
    case 'H':
      show_hash = true;
      opt[opt_pos++] = "-H";
      break;
    case 'i':
      interactive = true;
      opt[opt_pos++] = "-i";
      break;
    case 'l':
      list_all_devices = true;
      opt[opt_pos++] = "-l";
      break;
    case 'L':
      latest = true;
      opt[opt_pos++] = "-L";
      break;
    case 's':
      showing = true;
      opt[opt_pos++] = "-s";
      break;
    case 'a':
      g_rom_type |= BACON_ROM_TYPE_ALL;
      opt[opt_pos++] = "-a";
      break;
    case 'e':
      g_rom_type |= BACON_ROM_TYPE_TEST;
      opt[opt_pos++] = "-e";
      break;
    case 'm':
      g_rom_type |= BACON_ROM_TYPE_SNAPSHOT;
      opt[opt_pos++] = "-m";
      break;
    case 'n':
      g_rom_type |= BACON_ROM_TYPE_NIGHTLY;
      opt[opt_pos++] = "-n";
      break;
    case 'r':
      g_rom_type |= BACON_ROM_TYPE_RC;
      opt[opt_pos++] = "-r";
      break;
    case 'S':
      g_rom_type |= BACON_ROM_TYPE_STABLE;
      opt[opt_pos++] = "-S";
      break;
    case 'u':
      update_device_list = true;
      opt[opt_pos++] = "-u";
      break;
    case 'N':
      g_show_progress = false;
      opt[opt_pos++] = "-N";
      break;
    case 'U':
      show_url = true;
      opt[opt_pos++] = "-U";
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
  bool addopt;
  char *o;

  pos = 0;

  for (x = 1; v[x]; ++x) {
    addopt = true;
    if (bacon_streq (v[x], "-?") ||
        bacon_streq (v[x], "-h") ||
        bacon_streq (v[x], "--help"))
      bacon_help ();
    else if (bacon_streq (v[x], "-v") || bacon_streq (v[x], "--version"))
      bacon_version ();
    else if (bacon_streq (v[x], "-H") || bacon_streq (v[x], "--hash"))
      show_hash = true;
    else if (bacon_streq (v[x], "-l") || bacon_streq (v[x], "--list-devices"))
      list_all_devices = true;
    else if (bacon_streq (v[x], "-L") || bacon_streq (v[x], "--latest"))
      latest = true;
    else if (bacon_streq (v[x], "-d") || bacon_streq (v[x], "--download"))
      downloading = true;
    else if (bacon_streq (v[x], "-M") || bacon_streq (v[x], "--max")) {
      if (!v[x + 1]) {
        bacon_error ("`%s' requires an argument (try `--help')", v[x]);
        exit (EXIT_FAILURE);
      }
      opt[opt_pos++] = v[x];
      addopt = false;
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
      opt[opt_pos++] = "-M";
      addopt = false;
    } else if (bacon_strstw (v[x], "--max=")) {
      o = strchr (v[x], '=');
      ++o;
      if (!o || !*o) {
        bacon_error ("`--max' requires an argument (try `--help')");
        exit (EXIT_FAILURE);
      }
      if (!bacon_set_max_roms_from_arg (o)) {
        bacon_error ("'%s' is not a valid argument for `--max' "
                     "(try `--help')", o);
        exit (EXIT_FAILURE);
      }
      opt[opt_pos++] = "--max";
      addopt = false;
    } else if (bacon_streq (v[x], "-N") ||
               bacon_streq (v[x], "--no-progress"))
      g_show_progress = false;
    else if (bacon_streq (v[x], "-i") ||
             bacon_streq (v[x], "--interactive"))
      interactive = true;
    else if (bacon_streq (v[x], "-s") || bacon_streq (v[x], "--show"))
      showing = true;
    else if (bacon_streq (v[x], "-a") || bacon_streq (v[x], "--all"))
      g_rom_type |= BACON_ROM_TYPE_ALL;
    else if (bacon_streq (v[x], "-e") ||
             bacon_streq (v[x], "--experimental"))
      g_rom_type |= BACON_ROM_TYPE_TEST;
    else if (bacon_streq (v[x], "-m") ||
             bacon_streq (v[x], "--snapshot"))
      g_rom_type |= BACON_ROM_TYPE_SNAPSHOT;
    else if (bacon_streq (v[x], "-n") ||
             bacon_streq (v[x], "--nightly"))
      g_rom_type |= BACON_ROM_TYPE_NIGHTLY;
    else if (bacon_streq (v[x], "-r") || bacon_streq (v[x], "--rc"))
      g_rom_type |= BACON_ROM_TYPE_RC;
    else if (bacon_streq (v[x], "-S") || bacon_streq (v[x], "--stable"))
      g_rom_type |= BACON_ROM_TYPE_STABLE;
    else if (bacon_streq (v[x], "-u") ||
             bacon_streq (v[x], "--update-device-list"))
      update_device_list = true;
    else if (bacon_streq (v[x], "-U") || bacon_streq (v[x], "--url"))
      show_url = true;
    else if (bacon_streq (v[x], "-o") || bacon_streq (v[x], "--output")) {
      if (!v[x + 1] || v[x + 1][0] == '-') {
        bacon_error ("`%s' requires an argument (try `--help')", v[x]);
        exit (EXIT_FAILURE);
      }
      opt[opt_pos++] = v[x];
      addopt = false;
      g_out_path = bacon_strdup (v[++x]);
    } else if (bacon_strstw (v[x], "--output=")) {
      o = strchr (v[x], '=');
      ++o;
      if (!o || !*o) {
        bacon_error ("`--output' requires an argument (try `--help')");
        exit (EXIT_FAILURE);
      }
      g_out_path = bacon_strdup (o);
      opt[opt_pos++] = "--output";
      addopt = false;
    } else if (v[x][0] == '-') {
      if (!v[x][1] || v[x][1] == '-') {
        bacon_error ("`%s' is an unrecognized option (try `--help')", v[x]);
        exit (EXIT_FAILURE);
      }
      bacon_parse_string_for_short_opts (v[x]);
      addopt = false;
    } else if (pos < BACON_DEVICES_MAX) {
      n = strlen (v[x]);
      strncpy (devices[pos].id, v[x], n);
      devices[pos++].id[n] = '\0';
      addopt = false;
    }
    if (addopt)
      opt[opt_pos++] = v[x];
  }

  opt[opt_pos] = NULL;
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
        bacon_outlni (3, "url:      " BACON_GET_CM_URL "/%s", rom->get);
      if (!rom->next)
        break;
      ++n;
    }
  }
}

static void
bacon_download_rom (const BaconDevice *device, const BaconRom *rom)
{
  if (rom && !bacon_rom_do_download (rom, g_out_path))
    exit (EXIT_FAILURE);
}

static void
bacon_check_given_devices (void)
{
  size_t x;
  bool bad_device;

  bad_device = false;
  for (x = 0; *devices[x].id; ++x) {
    if (!bacon_device_is_valid_id (g_device_list, devices[x].id)) {
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

  if (!g_device_list)
    g_device_list = bacon_device_list_new (update_device_list);

  if (interactive) {
    bacon_interactive ();
    return;
  }

  if (list_all_devices)
    bacon_list_all_devices ();

  if (*devices[0].id)
    bacon_check_given_devices ();

  for (pos = 0; *devices[pos].id; ++pos) {
    device = bacon_device_get_device_from_id (g_device_list, devices[pos].id);
    rom_list = bacon_rom_list_new (device->codename, g_rom_type, g_max_roms);
    if (showing)
      bacon_show_rom_list (device, rom_list);
    else if (downloading && latest)
      for (x = 0; x < BACON_ROM_TOTAL; ++x)
        bacon_download_rom (device, rom_list->roms[x]);
    bacon_rom_list_destroy (rom_list);
  }
}

int
main (int argc, char **argv)
{
  bacon_set_program_name (argv[0]);
  atexit (bacon_cleanup);
  bacon_env_set_program_data_path ();
#ifdef BACON_GTK
  if (argc == 1) {
    bacon_gtk_main (&argc, &argv);
    exit (EXIT_SUCCESS);
  }
#endif
  bacon_parse_opt (argv);
  bacon_perform ();
  exit (EXIT_SUCCESS);
  return 0;
}

