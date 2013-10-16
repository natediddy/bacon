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

#include <errno.h>
#include <string.h>

#include "bacon-env.h"
#include "bacon-out.h"
#include "bacon-str.h"
#include "bacon-util.h"

#define BACON_PROGRAM_DIRNAME  "." BACON_PROGRAM_NAME

char *g_program_data_path = NULL;

BaconBoolean
bacon_env_is_directory (const char *path)
{
  struct stat s;

  memset (&s, 0, sizeof (struct stat));
  if (stat (path, &s) == 0) {
    if (S_ISDIR (s.st_mode))
      return BACON_TRUE;
  } else if (errno != ENOENT)
    bacon_debug ("failed to stat `%s': %s", path, strerror (errno));
  return BACON_FALSE;
}

BaconBoolean
bacon_env_is_file (const char *path)
{
  struct stat s;

  memset (&s, 0, sizeof (struct stat));
  if (stat (path, &s) == 0) {
    if (S_ISREG (s.st_mode))
      return BACON_TRUE;
  } else if (errno != ENOENT)
    bacon_debug ("failed to stat `%s': %s", path, strerror (errno));
  return BACON_FALSE;
}

void
bacon_env_delete (const char *path)
{
  if (bacon_env_is_file (path)) {
    if (!bacon_delete_file (path)) {
      bacon_error ("failed to delete file `%s' (%s)", path, strerror (errno));
      exit (EXIT_FAILURE);
    }
  }
}

unsigned long
bacon_env_size_of_file (const char *path)
{
  unsigned long res;
  struct stat s;

  res = 0;
  memset (&s, 0, sizeof (struct stat));
  if (stat (path, &s) == 0)
    res = ((unsigned long) s.st_size);
  return res;
}

FILE *
bacon_env_fopen (const char *path, const char *mode)
{
  FILE *fp;

  fp = fopen (path, mode);
  if (!fp) {
    bacon_error ("failed to open file `%s' (%s)", path, strerror (errno));
    exit (EXIT_FAILURE);
  }
  return fp;
}

void
bacon_env_fclose (FILE *fp)
{
  if (fp)
    if (fclose (fp) != 0)
      bacon_warn ("failed to close file (%s)", strerror (errno));
}

char *
bacon_env_getenv (const char *key)
{
  char *result = NULL;
#ifdef BACON_OS_UNIX
  char *valbuf;

  valbuf = getenv (key);
  if (valbuf)
    result = bacon_strdup (valbuf);
#endif
#ifdef BACON_OS_WINDOWS
  char valbuf[BACON_PATH_MAX];

  if (GetEnvironmentVariable (key, valbuf, BACON_PATH_MAX) == 0)
    result = bacon_strdup (valbuf);
#endif
  if (!result)
    bacon_debug ("failed to get environment variable '%s'", key);
  return result;
}

char *
bacon_env_home_path (void)
{
  char *home = NULL;

#ifdef BACON_OS_UNIX
  home = bacon_env_getenv ("HOME");
#endif
#ifdef BACON_OS_WINDOWS
  char *drive = NULL;
  char *path = NULL;

  drive = bacon_env_getenv ("HOMEDRIVE");
  if (drive) {
    path = bacon_env_getenv ("HOMEPATH");
    if (path)
      home = bacon_strf ("%s%s", drive, path);
  }
  bacon_free (drive);
  bacon_free (path);
#endif
  if (!home) {
    bacon_error ("could not find the current home directory path");
    exit (EXIT_FAILURE);
  }
  return home;
}

#ifdef BACON_OS_UNIX
void
bacon_env_make_hidden (char *path)
{
  char *old;
  char *base;
  char *hbase;
  char *parent;

  base = bacon_env_basename (path);
  if (base && *base && (*base != '.')) {
    hbase = bacon_strf (".%s", base);
    parent = bacon_env_dirname (path);
    old = bacon_strdup (path);
    bacon_free (path);
    path = bacon_strf ("%s%c%s", parent, BACON_PATH_SEP, hbase);
    bacon_free (parent);
    bacon_free (hbase);
    if (rename (old, path) == -1)
      bacon_debug ("failed to rename `%s' to `%s' (%s)",
                   old, path, strerror (errno));
    bacon_free (old);
  }
  bacon_free (base);
}
#else
void
bacon_env_make_hidden (const char *path)
{
  if (!SetFileAttributes (path, FILE_ATTRIBUTE_HIDDEN))
    bacon_debug ("failed to set hidden attribute for `%s'", path);
}
#endif

void
bacon_env_set_program_data_path (void)
{
  char *home;

  if (!g_program_data_path) {
    home = bacon_env_home_path ();
    g_program_data_path =
      bacon_strf ("%s%c%s", home, BACON_PATH_SEP, BACON_PROGRAM_DIRNAME);
    bacon_free (home);
  }

  if (!bacon_env_is_directory (g_program_data_path)) {
    if (!bacon_env_mkpath (g_program_data_path)) {
      bacon_error ("could not create program path `%s'", g_program_data_path);
      exit (EXIT_FAILURE);
    }
    bacon_env_make_hidden (g_program_data_path);
  }
}

char *
bacon_env_cwd (void)
{
  char cwd[BACON_PATH_MAX];

  if (bacon_getcwd (cwd, BACON_PATH_MAX))
    return bacon_strdup (cwd);
  return NULL;
}

char *
bacon_env_dirname (const char *path)
{
  ssize_t pos;
  char *dirname;

  for (pos = strlen (path) - 1; pos >= 0; --pos)
    if (bacon_env_is_path_sep (path[pos]))
      break;
  dirname = bacon_newa (char, pos);
  strncpy (dirname, path, pos);
  dirname[pos] = '\0';
  return dirname;
}

char *
bacon_env_basename (const char *path)
{
  size_t n;
  size_t n_basename;
  ssize_t pos;
  char *basename;

  n = strlen (path);
  while (bacon_env_is_path_sep (path[n - 1]))
    --n;

  for (pos = n - 1; pos >= 0; --pos)
    if (bacon_env_is_path_sep (path[pos]))
      break;

  n_basename = n - pos;
  basename = bacon_newa (char, n_basename);
  strncpy (basename, path + (pos + 1), n_basename);
  basename[n_basename] = '\0';

  while (bacon_env_is_path_sep (basename[n_basename - 1])) {
    basename[n_basename - 1] = '\0';
    --n_basename;
  }
  return basename;
}

char *
bacon_env_mkabs (const char *path)
{
  char *cwd;
  char *abs;

  if (bacon_env_is_path_sep (*path))
    return bacon_strdup (path);
  cwd = bacon_env_cwd ();
  abs = bacon_strf ("%s%c%s", cwd, BACON_PATH_SEP, path);
  bacon_free (cwd);
  return abs;
}

/* TODO: recognize '~' for home directory in
   paths used with `--output=~/foo/bar' option.
   The shell does the work fine whenever '~' is
   used with `-o ~/foo/bar' or `--output ~/foo/bar'.
   The trouble is from the '=' in the long option version. */
/*void
bacon_env_exppath (char *path)
{
  size_t n;
  size_t x;
  ssize_t tilde;
  char *p;
  char *home;

  n = strlen (path);
  tilde = -1;
  for (x = 0; x < n; ++x) {
    if (path[x] == '~') {
      tilde = x;
      break;
    }
  }

  if (tilde == -1)
    return;

  home = bacon_env_home_path ();
}*/

BaconBoolean
bacon_env_mkpath (const char *path)
{
  BaconBoolean res;
  char c;
  char *p;
  char *ppath;
 
  res = BACON_TRUE;
  ppath = bacon_env_mkabs (path);
  p = ppath;

  while (*p) {
    p++;
    while (*p && !bacon_env_is_path_sep (*p))
      p++;
    c = *p;
    *p = '\0';
    if ((bacon_mkdir (ppath) == -1) && (errno != EEXIST)) {
      res = BACON_FALSE;
      goto done;
    }
    *p = c;
  }

done:
  bacon_free (ppath);
  return res;
}

BaconBoolean
bacon_env_ensure_path (const char *path, BaconBoolean file)
{
  char *dn;
  char *abs;
  BaconBoolean ret;

  if (file) {
    abs = bacon_env_mkabs (path);
    dn = bacon_env_dirname (abs);
    ret = bacon_env_mkpath(dn);
    bacon_free (dn);
    bacon_free (abs);
  } else
    ret = bacon_env_mkpath (path);
  return ret;
}

void
bacon_env_fix_download_path (char **path, const char *name)
{
  char *p;

  p = NULL;
  if (!(*path)) {
    p = bacon_env_cwd ();
    if (p && *p)
      (*path) = bacon_strf ("%s%c%s", p, BACON_PATH_SEP, name);
    else
      (*path) = bacon_strdup (name);
    bacon_free (p);
    return;
  }

  if (bacon_env_is_directory ((*path))) {
    p = bacon_strf ("%s%c%s", (*path), BACON_PATH_SEP, name);
    bacon_free ((*path));
    (*path) = bacon_strdup (p);
  }
}

