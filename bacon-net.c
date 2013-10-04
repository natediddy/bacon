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

#include <errno.h>
#include <string.h>

#include <curl/curl.h>

#include "bacon-env.h"
#include "bacon-net.h"
#include "bacon-progress.h"
#include "bacon-util.h"

#define BACON_USERAGENT BACON_PROGRAM_NAME " 0.0.1/CM ROM downloader"

#define BACON_FILE_RESULT ((BaconFileResult *) (net->res))
#define BACON_PAGE_RESULT ((BaconPageResult *) (net->res))

#define bacon_net_setopt(o, p) net->status = curl_easy_setopt (net->cp, o, p)

extern bool g_show_progress;

typedef struct {
  FILE *fp;
  char *path;
  long offset;
  bool (*setup) (void);
  size_t (*write) (void *, size_t, size_t, FILE *);
  int (*progress) (void *, double, double, double, double);
} BaconFileResult;

typedef struct {
  char *buffer;
  size_t n;
} BaconDataChunk;

typedef struct {
  BaconDataChunk chunk;
  bool (*setup) (void);
  size_t (*write) (void *, size_t, size_t, void *);
  int (*progress) (void *, double, double, double, double);
} BaconPageResult;

typedef enum {
  BACON_NET_ACTION_GET_FILE,
  BACON_NET_ACTION_GET_PAGE
} BaconNetAction;

typedef struct {
  CURL *cp;
  CURLcode status;
  BaconNetAction action;
  char *url;
  void *res;
} BaconNetInstance;

static BaconNetInstance *net = NULL;

static size_t
bacon_file_write (void *p, size_t size, size_t nmemb, FILE *fp)
{
  return fwrite (p, size, nmemb, fp);
}

static size_t
bacon_page_write (void *buf, size_t size, size_t nmemb, void *o)
{
  size_t n;
  BaconDataChunk *p;

  p = (BaconDataChunk *) o;
  n = size * nmemb;

  p->buffer = (char *) bacon_realloc (p->buffer, p->n + n + 1);
  if (!p->buffer)
    return 0;

  memcpy (&(p->buffer[p->n]), buf, n);
  p->n += n;
  p->buffer[p->n] = 0;
  return n;
}

static int
bacon_file_progress (void *data, double td, double cd, double tu, double cu)
{
  bacon_progress_file (td, cd);
  return 0;
}

static int
bacon_page_progress (void *data, double td, double cd, double tu, double cu)
{
  bacon_progress_page (td, cd);
  return 0;
}

static char *
bacon_build_url (const char *req)
{
  return bacon_strf ("%s/%s", BACON_ROOT_URL, (req && *req) ? req : "");
}

static bool
bacon_net_check (void)
{
  if (net->status == CURLE_OK)
    return true;
  bacon_error (curl_easy_strerror (net->status));
  return false;
}

static bool
bacon_file_setup (void)
{
  bool check;

  if (BACON_FILE_RESULT->offset == 0)
    BACON_FILE_RESULT->fp = bacon_env_fopen (BACON_FILE_RESULT->path, "wb");
  else if (BACON_FILE_RESULT->offset > 0)
    BACON_FILE_RESULT->fp = bacon_env_fopen (BACON_FILE_RESULT->path, "ab");

  bacon_net_setopt (CURLOPT_WRITEDATA, (void *) BACON_FILE_RESULT->fp);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_RESUME_FROM, BACON_FILE_RESULT->offset);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_WRITEFUNCTION, (void *) BACON_FILE_RESULT->write);
  check = bacon_net_check ();

  if (check && g_show_progress) {
    bacon_net_setopt (CURLOPT_PROGRESSFUNCTION,
                      (void *) BACON_FILE_RESULT->progress);
    check = bacon_net_check ();
  }
  return check;
}

static bool
bacon_page_setup (void)
{
  bool check;

  bacon_net_setopt (CURLOPT_WRITEDATA, (void *) &BACON_PAGE_RESULT->chunk);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_WRITEFUNCTION, (void *) BACON_PAGE_RESULT->write);
  check = bacon_net_check ();

  if (check && g_show_progress)
  {
    bacon_net_setopt (CURLOPT_PROGRESSFUNCTION,
                      (void *) BACON_PAGE_RESULT->progress);
    check = bacon_net_check ();
  }
  return check;
}

static void
bacon_net_init (BaconNetAction action,
                const char *req,
                long offset,
                const char *loc)
{
  net = bacon_new (BaconNetInstance);
  net->url = bacon_build_url (req);
  net->action = action;
  net->cp = curl_easy_init ();

  if (!net->cp)
    net->status = CURLE_FAILED_INIT;
  else
    net->status = CURLE_OK;

  if (net->action == BACON_NET_ACTION_GET_FILE) {
    net->res = bacon_new (BaconFileResult);
    BACON_FILE_RESULT->offset = offset;
    BACON_FILE_RESULT->fp = NULL;
    if (loc)
      BACON_FILE_RESULT->path = bacon_strdup (loc);
    else
      BACON_FILE_RESULT->path = NULL;
    BACON_FILE_RESULT->setup = &bacon_file_setup;
    BACON_FILE_RESULT->write = &bacon_file_write;
    if (g_show_progress)
      BACON_FILE_RESULT->progress = &bacon_file_progress;
  } else {
    net->res = bacon_new (BaconPageResult);
    memset (&BACON_PAGE_RESULT->chunk, 0, sizeof (BaconDataChunk));
    BACON_PAGE_RESULT->chunk.buffer = bacon_newa (char, 1);
    BACON_PAGE_RESULT->chunk.n = 0;
    BACON_PAGE_RESULT->setup = &bacon_page_setup;
    BACON_PAGE_RESULT->write = &bacon_page_write;
    if (g_show_progress)
      BACON_PAGE_RESULT->progress = &bacon_page_progress;
  }
}

static bool
bacon_net_setup (void)
{
  bacon_net_setopt (CURLOPT_URL, net->url);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_USERAGENT, BACON_USERAGENT);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_FOLLOWLOCATION, 1L);
  if (!bacon_net_check ())
    return false;

  if (g_show_progress)
    bacon_net_setopt (CURLOPT_NOPROGRESS, 0L);
  else
    bacon_net_setopt (CURLOPT_NOPROGRESS, 1L);
  if (!bacon_net_check ())
    return false;

  if (net->action == BACON_NET_ACTION_GET_FILE)
    return BACON_FILE_RESULT->setup ();
  return BACON_PAGE_RESULT->setup ();
}

static bool
bacon_net_fetch (void)
{
  if (g_show_progress)
    bacon_progress_init ();
  net->status = curl_easy_perform (net->cp);
  if (g_show_progress)
    bacon_progress_deinit (net->action == BACON_NET_ACTION_GET_FILE);
  return bacon_net_check ();
}

bool
bacon_net_init_for_page_data (const char *request)
{
  if (net)
    bacon_net_deinit ();
  bacon_net_init (BACON_NET_ACTION_GET_PAGE, request, -1, NULL);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  bacon_net_deinit ();
  return false;
}

bool
bacon_net_init_for_rom (const char *request,
                        long offset,
                        const char *filename)
{
  if (net)
    bacon_net_deinit ();
  bacon_net_init (BACON_NET_ACTION_GET_FILE, request, offset, filename);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  bacon_net_deinit ();
  return false;
}

void
bacon_net_deinit (void)
{
  curl_easy_cleanup (net->cp);
  bacon_free (net->url);

  if (net->res) {
    if (net->action == BACON_NET_ACTION_GET_FILE) {
      if (BACON_FILE_RESULT->fp)
        fclose (BACON_FILE_RESULT->fp);
      bacon_free (BACON_FILE_RESULT->path);
    } else if (net->action == BACON_NET_ACTION_GET_PAGE)
      bacon_free (BACON_PAGE_RESULT->chunk.buffer);
    bacon_free (net->res);
  }
  bacon_free (net);
}

char *
bacon_net_get_page_data (void)
{
  if (net && net->action == BACON_NET_ACTION_GET_PAGE) {
    if (bacon_net_fetch ())
      return BACON_PAGE_RESULT->chunk.buffer;
    bacon_error (curl_easy_strerror (net->status));
  }
  return NULL;
}

bool
bacon_net_get_rom (void)
{
  if (net && net->action == BACON_NET_ACTION_GET_FILE) {
    if (bacon_net_fetch ())
      return true;
    bacon_error (curl_easy_strerror (net->status));
  }
  return false;
}

