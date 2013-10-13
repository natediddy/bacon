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

#define BACON_URL_MAX          1024
#define BACON_USERAGENT        \
  BACON_PROGRAM_NAME " " BACON_VERSION "/CM ROM downloader"
#define BACON_FILE_RESULT      ((BaconFileResult *) (s_net->res))
#define BACON_PAGE_RESULT      ((BaconPageResult *) (s_net->res))
#define bacon_net_setopt(o, p) \
  s_net->status = curl_easy_setopt (s_net->cp, o, p)

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
  void *res;
} BaconNetInstance;

extern bool              g_show_progress;
static BaconNetInstance *s_net          = NULL;
#ifdef BACON_GTK
static bool              s_for_icons    = false;
#endif
static char              s_url          [BACON_URL_MAX];

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

#ifdef BACON_GTK
static int
bacon_gtk_progress (void *progress_bar,
                    double td,
                    double cd,
                    double tu,
                    double cu)
{
  gdouble fraction;

  fraction = (cd / td);
  if (!bacon_nan_value (fraction)) {
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
    while (gtk_events_pending ())
      gtk_main_iteration ();
  }
  return 0;
}
#endif

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
bacon_set_url (const char *root, const char *req)
{
  if (root && *root) {
    if (req && *req)
      snprintf (s_url, BACON_URL_MAX, "%s/%s", root, req);
    else
      snprintf (s_url, BACON_URL_MAX, "%s", root);
  } else
    *s_url = '\0';
}

static bool
bacon_net_check (void)
{
  if (s_net->status == CURLE_OK)
    return true;
  bacon_error (curl_easy_strerror (s_net->status));
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

  if (check && BACON_FILE_RESULT->progress) {
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

  if (check && BACON_PAGE_RESULT->progress) {
    bacon_net_setopt (CURLOPT_PROGRESSFUNCTION,
                      (void *) BACON_PAGE_RESULT->progress);
    check = bacon_net_check ();
  }
  return check;
}

static void
bacon_net_init (BaconNetAction action, long offset, const char *loc)
{
  s_net = bacon_new (BaconNetInstance);
  s_net->action = action;

  s_net->cp = curl_easy_init ();
  if (!s_net->cp) {
    s_net->status = CURLE_FAILED_INIT;
    return;
  }
  s_net->status = CURLE_OK;

  if (s_net->action == BACON_NET_ACTION_GET_FILE) {
    s_net->res = bacon_new (BaconFileResult);
    BACON_FILE_RESULT->offset = offset;
    BACON_FILE_RESULT->fp = NULL;
    if (loc)
      BACON_FILE_RESULT->path = bacon_strdup (loc);
    else
      BACON_FILE_RESULT->path = NULL;
    BACON_FILE_RESULT->setup = &bacon_file_setup;
    BACON_FILE_RESULT->write = &bacon_file_write;
#ifdef BACON_GTK
    if (!s_for_icons && g_show_progress)
#else
    if (g_show_progress)
#endif
      BACON_FILE_RESULT->progress = &bacon_file_progress;
    else
      BACON_FILE_RESULT->progress = NULL;
  } else {
    s_net->res = bacon_new (BaconPageResult);
    memset (&BACON_PAGE_RESULT->chunk, 0, sizeof (BaconDataChunk));
    BACON_PAGE_RESULT->chunk.buffer = bacon_newa (char, 1);
    BACON_PAGE_RESULT->chunk.n = 0;
    BACON_PAGE_RESULT->setup = &bacon_page_setup;
    BACON_PAGE_RESULT->write = &bacon_page_write;
#ifdef BACON_GTK
    if (!s_for_icons && g_show_progress)
#else
    if (g_show_progress)
#endif
      BACON_PAGE_RESULT->progress = &bacon_page_progress;
    else
      BACON_PAGE_RESULT->progress = NULL;
  }
}

static bool
bacon_net_setup (void)
{
  bool null_progress_cb;

  bacon_net_setopt (CURLOPT_URL, s_url);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_USERAGENT, BACON_USERAGENT);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_FOLLOWLOCATION, 1L);
  if (!bacon_net_check ())
    return false;

  if ((s_net->action == BACON_NET_ACTION_GET_FILE) &&
      !BACON_FILE_RESULT->progress)
    null_progress_cb = true;
  else if ((s_net->action == BACON_NET_ACTION_GET_PAGE) &&
           !BACON_PAGE_RESULT->progress)
    null_progress_cb = true;
  else
    null_progress_cb = false;

  if (!null_progress_cb)
    bacon_net_setopt (CURLOPT_NOPROGRESS, 0L);
  else
    bacon_net_setopt (CURLOPT_NOPROGRESS, 1L);
  if (!bacon_net_check ())
    return false;

  if (s_net->action == BACON_NET_ACTION_GET_FILE)
    return BACON_FILE_RESULT->setup ();
  return BACON_PAGE_RESULT->setup ();
}

static bool
bacon_net_fetch (void)
{
#ifdef BACON_GTK
  if (!s_for_icons && g_show_progress)
#else
  if (g_show_progress)
#endif
    bacon_progress_init ();
  s_net->status = curl_easy_perform (s_net->cp);
#ifdef BACON_GTK
  if (!s_for_icons && g_show_progress)
#else
  if (g_show_progress)
#endif
    bacon_progress_deinit (s_net->action == BACON_NET_ACTION_GET_FILE);
  return bacon_net_check ();
}

bool
bacon_net_init_for_page_data (const char *request)
{
  if (s_net)
    bacon_net_deinit ();
  bacon_set_url (BACON_GET_CM_URL, request);
  bacon_net_init (BACON_NET_ACTION_GET_PAGE, -1, NULL);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  return false;
}

bool
bacon_net_init_for_rom (const char *request,
                        long offset,
                        const char *filename)
{
  if (s_net)
    bacon_net_deinit ();
  bacon_set_url (BACON_GET_CM_URL, request);
  bacon_net_init (BACON_NET_ACTION_GET_FILE, offset, filename);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  return false;
}

#ifdef BACON_GTK
bool
bacon_net_init_for_device_icons (void)
{
  s_for_icons = true;
  if (s_net)
    bacon_net_deinit ();
  bacon_set_url (BACON_DEVICE_ICONS_URL, NULL);
  bacon_net_init (BACON_NET_ACTION_GET_PAGE, -1, NULL);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  return false;
}

bool
bacon_net_init_for_device_icon_thumb (const char *request,
                                      const char *filename)
{
  s_for_icons = true;
  if (s_net)
    bacon_net_deinit ();
  bacon_set_url (BACON_DEVICE_ICON_THUMB_URL, request);
  bacon_net_init (BACON_NET_ACTION_GET_FILE, 0, filename);
  if (bacon_net_check () && bacon_net_setup ())
    return true;
  return false;
}

bool
bacon_net_gtk_init_for_device_list (GtkProgressBar *progress_bar)
{
  if (s_net)
    bacon_net_deinit ();
  bacon_set_url (BACON_GET_CM_URL, "");

  bacon_net_init (BACON_NET_ACTION_GET_PAGE, -1, NULL);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_URL, s_url);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_USERAGENT, BACON_USERAGENT);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_FOLLOWLOCATION, 1L);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_NOPROGRESS, 0L);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_PROGRESSFUNCTION, bacon_gtk_progress);
  if (!bacon_net_check ())
    return false;

  bacon_net_setopt (CURLOPT_PROGRESSDATA, progress_bar);
  if (!bacon_net_check ())
    return false;

  BACON_PAGE_RESULT->progress = NULL;
  return BACON_PAGE_RESULT->setup ();
}
#endif

void
bacon_net_deinit (void)
{
  *s_url = '\0';
  curl_easy_cleanup (s_net->cp);
#ifdef BACON_GTK
  s_for_icons = false;
#endif

  if (s_net->res) {
    if (s_net->action == BACON_NET_ACTION_GET_FILE) {
      bacon_env_fclose (BACON_FILE_RESULT->fp);
      bacon_free (BACON_FILE_RESULT->path);
    } else if (s_net->action == BACON_NET_ACTION_GET_PAGE)
      bacon_free (BACON_PAGE_RESULT->chunk.buffer);
    bacon_free (s_net->res);
  }
  bacon_free (s_net);
}

char *
bacon_net_get_page_data (void)
{
  if (s_net && (s_net->action == BACON_NET_ACTION_GET_PAGE)) {
    if (bacon_net_fetch ())
      return BACON_PAGE_RESULT->chunk.buffer;
    bacon_error (curl_easy_strerror (s_net->status));
  }
  return NULL;
}

bool
bacon_net_get_file (void)
{
  if (s_net && (s_net->action == BACON_NET_ACTION_GET_FILE)) {
    if (bacon_net_fetch ())
      return true;
    bacon_error (curl_easy_strerror (s_net->status));
  }
  return false;
}

