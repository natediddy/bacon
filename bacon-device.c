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
#include <sys/stat.h>
#include <sys/types.h>

#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-util.h"

#define BACON_DEVICE_LIST_LOCAL_FILENAME "devicelist.txt"

static char *local_device_list_path = NULL;

static void
bacon_set_local_device_list_path (void)
{
  char *ppath;

  ppath = bacon_env_program_path ();
  local_device_list_path = bacon_strf ("%s%c%s", ppath, BACON_PATH_SEP,
                                       BACON_DEVICE_LIST_LOCAL_FILENAME);
  bacon_free (ppath);
}

static bool
bacon_has_local_device_list (void)
{
  return bacon_env_file_exists (local_device_list_path);
}

static void
bacon_write_local_device_list (BaconDeviceList *list)
{
  FILE *fp;
  BaconDeviceList *p;

  fp = bacon_env_fopen (local_device_list_path, "w");
  for (p = list; p; p = p->next)
  {
    bacon_foutln (fp, "%s@%s", p->device->codename, p->device->fullname);
    if (!p->next)
      break;
  }
  bacon_env_fclose (fp);
}

static char *
bacon_device_local_data (void)
{
  int c;
  size_t n;
  size_t x;
  char *data;
  FILE *fp;
  struct stat s;

  n = 0;
  memset (&s, 0, sizeof (struct stat));
  if (stat (local_device_list_path, &s) == 0)
    n = ((size_t) s.st_size);

  data = bacon_newa (char, n);
  fp = bacon_env_fopen (local_device_list_path, "r");

  x = 0;
  while (true)
  {
    c = fgetc (fp);
    if (c == EOF)
      break;
    data[x++] = ((char) c);
  }

  data[x] = '\0';
  bacon_env_fclose (fp);
  return data;
}

BaconDeviceList *
bacon_device_list_new (bool force_new)
{
  char *data;
  BaconDeviceList *list;

  data = NULL;
  bacon_set_local_device_list_path ();

  if (force_new || !bacon_has_local_device_list ())
  {
    if (bacon_net_init_for_page_data (""))
    {
      data = bacon_net_get_page_data ();
      if (data)
      {
        list = bacon_parse_for_device_list (data, false);
        bacon_write_local_device_list (list);
      }
      bacon_net_deinit ();
    }
  }
  else
  {
    data = bacon_device_local_data ();
    if (data)
      list = bacon_parse_for_device_list (data, true);
  }
  return list;
}

void
bacon_device_list_destroy (BaconDeviceList *list)
{
  for (; list; list = list->next)
  {
    bacon_free (list->device);
    bacon_free (list->prev);
    if (!list->next)
      break;
  }
  bacon_free (list);
}

int
bacon_device_list_total (BaconDeviceList *list)
{
  int total;
  BaconDeviceList *p;

  total = 0;
  for (p = list; p; p = p->next)
  {
    total++;
    if (!p->next)
      break;
  }
  return total;
}

bool
bacon_device_is_valid_id (BaconDeviceList *list, const char *id)
{
  BaconDeviceList *p;

  for (p = list; p; p = p->next)
  {
    if (bacon_streq (id, p->device->codename))
      return true;
    if (!p->next)
      break;
  }
  return false;
}

BaconDevice *
bacon_device_get_device_from_id (BaconDeviceList *list, const char *id)
{
  BaconDevice *device;
  BaconDeviceList *p;

  device = NULL;
  for (p = list; p; p = p->next)
  {
    if (bacon_streq (id, p->device->codename))
    {
      device = p->device;
      break;
    }
    if (!p->next)
      break;
  }
  return device;
}

