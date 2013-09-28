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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bacon-env.h"
#include "bacon-os.h"
#include "bacon-util.h"

#ifdef BACON_OS_UNIX
# include <unistd.h>
#endif

#ifdef BACON_OS_WINDOWS
# include <direct.h>
# include <windows.h>
#endif

#ifdef BACON_OS_UNIX
# define BACON_HOME_VAR       "HOME"
# define BACON_DELETE_FILE(p) (unlink (p) == 0)
# define BACON_GETCWD(buf, n) (getcwd (buf, n) != NULL)
# define BACON_MKDIR(p)       mkdir (p, S_IRWXU)
#endif

#ifdef BACON_OS_WINDOWS
# define BACON_HOME_VAR       "USERPROFILE"
# define BACON_DELETE_FILE(p) (DeleteFile (p))
# define BACON_GETCWD(buf, n) (GetCurrentDirectory (n, buf) != 0)
# define BACON_MKDIR(p)       _mkdir (p)
#endif

#define BACON_PROGRAM_DIRNAME "." BACON_PROGRAM_NAME
#define BACON_PATH_BUFFER_MAX 1024

bool
bacon_env_dir_exists (const char *path)
{
  struct stat s;

  memset (&s, 0, sizeof (struct stat));
  if ((stat (path, &s) == 0) && S_ISDIR (s.st_mode))
    return true;
  return false;
}

bool
bacon_env_file_exists (const char *path)
{
  struct stat s;

  memset (&s, 0, sizeof (struct stat));
  if ((stat (path, &s) == 0) && S_ISREG(s.st_mode))
    return true;
  return false;
}

void
bacon_env_delete (const char *path)
{
  if (bacon_env_dir_exists (path))
    return;

  if (bacon_env_file_exists (path))
  {
    if (!BACON_DELETE_FILE (path))
    {
      bacon_error ("failed to delete file `%s' (%s)", path, strerror (errno));
      exit (EXIT_FAILURE);
    }
  }
}

long
bacon_env_size_of (const char *path)
{
  long res;
  struct stat s;

  res = 0;
  memset (&s, 0, sizeof (struct stat));
  if (stat (path, &s) == 0)
    res = s.st_size;
  return res;
}

FILE *
bacon_env_fopen (const char *path, const char *mode)
{
  FILE *fp;

  fp = fopen (path, mode);
  if (!fp)
  {
    bacon_error ("failed to open file `%s' (%s)", path, strerror (errno));
    exit (EXIT_FAILURE);
  }
  return fp;
}

void
bacon_env_fclose (FILE *fp)
{
  if (!fp)
    return;
  if (fclose (fp) != 0)
    bacon_warn ("failed to close file (%s)", strerror (errno));
}

char *
bacon_env_home_path (void)
{
#ifdef BACON_OS_UNIX
  char *home;

  home = getenv (BACON_HOME_VAR);
  if (!home || !*home)
    goto fail;
#endif
#ifdef BACON_OS_WINDOWS
  char home[BACON_PATH_BUFFER_MAX];

  if (GetEnvironmentVariable (BACON_HOME_VAR, home,
                              BACON_PATH_BUFFER_MAX) != 0)
    goto fail;
#endif
  return home;

fail:
  bacon_error ("could not find the current home directory path");
  exit (EXIT_FAILURE);
}

char *
bacon_env_program_path (void)
{
  char *path;

  path = bacon_strf ("%s%c%s", bacon_env_home_path(),
                     BACON_PATH_SEP, BACON_PROGRAM_DIRNAME);
  bacon_env_mkpath (path);
  return path;
}

char *
bacon_env_cwd (void)
{
  char cwd[BACON_PATH_BUFFER_MAX];

  if (BACON_GETCWD (cwd, BACON_PATH_BUFFER_MAX))
    return bacon_strdup (cwd);
  return NULL;
}

char *
bacon_env_dirname (const char *path)
{
  ssize_t pos;
  char *dirname;

  for (pos = strlen (path) - 1; pos >= 0; --pos)
    if (BACON_IS_SLASH (path[pos]))
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
  while (BACON_IS_SLASH (path[n - 1]))
    --n;

  for (pos = n - 1; pos >= 0; --pos)
    if (BACON_IS_SLASH (path[pos]))
      break;

  n_basename = n - pos;
  basename = bacon_newa (char, n_basename);
  strncpy (basename, path + (pos + 1), n_basename);
  basename[n_basename] = '\0';

  while (BACON_IS_SLASH (basename[n_basename - 1]))
  {
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

  if (BACON_IS_SLASH (*path))
    return bacon_strdup (path);
  cwd = bacon_env_cwd ();
  abs = bacon_strf ("%s%c%s", cwd, BACON_PATH_SEP, path);
  bacon_free (cwd);
  return abs;
}

/*
 * TODO: recognized '~' in paths
 */
bool
bacon_env_mkpath (const char *path)
{
  bool res;
  char c;
  char *p;
  char *ppath;
 
  res = true;
  ppath = bacon_env_mkabs (path);
  p = ppath;

  while (*p)
  {
    p++;
    while (*p && !BACON_IS_SLASH (*p))
      p++;
    c = *p;
    *p = '\0';
    if (BACON_MKDIR (ppath) == -1 && errno != EEXIST)
    {
      res = false;
      goto done;
    }
    *p = c;
  }

done:
  bacon_free (ppath);
  return res;
}

bool
bacon_env_ensure_path (const char *path, const bool file)
{
  char *dn;
  char *abs;
  bool ret;

  if (file)
  {
    abs = bacon_env_mkabs (path);
    dn = bacon_env_dirname (abs);
    ret = bacon_env_mkpath(dn);
    bacon_free (dn);
    bacon_free (abs);
  }
  else
    ret = bacon_env_mkpath (path);
  return ret;
}

