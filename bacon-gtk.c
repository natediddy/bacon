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
#ifdef BACON_GTK
#include <string.h>

#include <gtk/gtk.h>

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-gtk.h"
#include "bacon-hash.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-pixbufs.h"
#include "bacon-rom.h"
#include "bacon-util.h"

#define BACON_DEVICE_ICON_SCALE_WIDTH  100
#define BACON_DEVICE_ICON_SCALE_HEIGHT 100
#define BACON_WINDOW_SIZE_WIDTH        900
#define BACON_WINDOW_SIZE_HEIGHT       600

#define BACON_DISPLAY_NAME          "Bacon"
#define BACON_HELP_MENU_ITEM_LABEL  "Help"
#define BACON_ABOUT_MENU_ITEM_LABEL "About"
#define BACON_CONFIRM_QUIT_TITLE    "Quit"
#define BACON_CONFIRM_QUIT_MESSAGE  "Are you sure you want to exit?"

typedef struct BaconData      BaconData;
typedef struct BaconThumbList BaconThumbList;

struct BaconData {
  BaconDevice *device;
  BaconRomList *rom_list;
  GdkPixbuf *pixbuf;
  BaconData *next;
  BaconData *prev;
};

struct BaconThumbList {
  const char *name;
  BaconThumbList *next;
};

enum {
  DEVICE_FULLNAME_COLUMN,
  DEVICE_CODENAME_COLUMN,
  DEVICE_PIXBUF_COLUMN,
  N_COLUMNS
};

extern BaconDeviceList *g_device_list;
extern char *           g_program_data_path;

static BaconThumbList *s_thumbs      = NULL;
static BaconData *     s_data        = NULL;
static char *          s_thumbs_path = NULL;

static const char *const license_text =
#include "bacon-license.h"
;

static void
bacon_add_thumb (const char *name)
{
  BaconThumbList *n;

  n = bacon_new (BaconThumbList);
  n->name = name;
  n->next = s_thumbs;
  s_thumbs = n;
}

static char *
bacon_full_icon_path (const char *filename)
{
  return g_strdup_printf ("%s%c%s", s_thumbs_path, BACON_PATH_SEP, filename);
}

static void
bacon_set_thumbs_path (void)
{
  s_thumbs_path = g_strdup_printf ("%s%cthumbs",
                                   g_program_data_path, BACON_PATH_SEP);
  if (!bacon_env_is_directory (s_thumbs_path))
    bacon_env_mkpath (s_thumbs_path);
}

static void
bacon_init_icon_cache (void)
{
  char *data;
  char *iconpath;
  BaconDeviceThumbRequestList *p;
  BaconDeviceThumbRequestList *thumbs;

  data = NULL;
  thumbs = NULL;

  if (bacon_net_init_for_device_icons ()) {
    data = bacon_net_get_page_data ();
    if (data)
      thumbs =
        bacon_parse_for_device_thumb_request_list (data, g_device_list);
    bacon_net_deinit ();
  }

  bacon_set_thumbs_path ();

  for (p = thumbs; p; p = p->next) {
    bacon_add_thumb (p->filename);
    iconpath = bacon_full_icon_path (p->filename);
    if (bacon_env_is_file (iconpath))
      continue;
    g_message ("downloading thumb `%s'\n", iconpath);
    if (bacon_net_init_for_device_icon_thumb (p->request, iconpath)) {
      if (!bacon_net_get_file ())
        g_printerr ("failed to download icon from '%s/%s' to '%s'\n",
                    BACON_DEVICE_ICON_THUMB_URL, p->request, p->filename);
      bacon_net_deinit ();
    }
    g_free (iconpath);
    if (!p->next)
      break;
  }
}

static void
bacon_data_init (void)
{
  char *iconpath;
  GdkPixbuf *i;
  GError *error;
  BaconData *dp;
  BaconDeviceList *p;
  BaconThumbList *tp;

  error = NULL;

  for (p = g_device_list; p; p = p->next) {
    if (!s_data) {
      dp = bacon_new (BaconData);
      dp->prev = NULL;
    } else {
      for (dp = s_data; dp; dp = dp->next)
        if (!dp->next)
          break;
      dp->next = bacon_new (BaconData);
      dp->next->prev = dp;
      dp = dp->next;
    }
    dp->next = NULL;
    dp->device = p->device;
    tp = s_thumbs;
    while (tp) {
      if (strstr (tp->name, dp->device->codename))
        break;
      tp = tp->next;
    }
    if (tp && tp->name && *tp->name) {
      iconpath = bacon_full_icon_path (tp->name);
      dp->pixbuf =
        gdk_pixbuf_new_from_file_at_scale (iconpath,
                                           BACON_DEVICE_ICON_SCALE_WIDTH,
                                           BACON_DEVICE_ICON_SCALE_HEIGHT,
                                           TRUE, &error);
      if (error) {
        g_warning ("failed to create pixbuf from `%s': %s",
                   iconpath, error->message);
        g_error_free (error);
      }
      g_free (iconpath);
    } else {
      dp->pixbuf = gdk_pixbuf_new_from_inline (-1, fallback_device_icon,
                                               FALSE, &error);
      if (error) {
        g_warning ("failed to create pixbuf from inline: %s", error->message);
        g_error_free (error);
        dp->pixbuf = NULL;
      }
    }
    dp->rom_list = NULL;
    for (; dp; dp = dp->prev)
      if (!dp->prev)
        break;
    s_data = dp;
    if (!p->next)
      break;
  }

  if (error)
    g_error_free (error);
}

static gboolean
bacon_confirm_quit (GtkWidget *parent)
{
  gint response;
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (GTK_WINDOW (parent),
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_YES_NO,
                                   BACON_CONFIRM_QUIT_MESSAGE);
  gtk_window_set_title (GTK_WINDOW (dialog), BACON_CONFIRM_QUIT_TITLE);

  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (response == GTK_RESPONSE_YES)
    return TRUE;
  return FALSE;
}

static gboolean
bacon_on_main_window_delete_event (GtkWidget *widget, gpointer data)
{
  if (bacon_confirm_quit (widget)) {
    gtk_main_quit ();
    return FALSE;
  }
  return TRUE;
}

static void
bacon_on_menu_item_about_activate (GtkMenuItem *menu_item, gpointer user_data)
{
  static const gchar *const authors[] = {
    "Nathan Forbes " BACON_BUG_REPORT_EMAIL,
    NULL
  };
  static const gchar copyright[] = \
          "Copyright \xc2\xa9 2013 Nathan Forbes";
  static const gchar comments[] = \
          BACON_DISPLAY_NAME " is a tool for viewing and downloading "
          "CyanogenMod ROMs for Android handsets\n"
          "For more information about the CyanogenMod project, visit "
          "http://www.cyanogenmod.org/\n"
          "For more information about Android, visit http://www.android.com/";


  gtk_show_about_dialog (GTK_WINDOW (user_data),
                         "program-name", BACON_DISPLAY_NAME,
                         "authors", authors,
                         "comments", comments,
                         "copyright", copyright,
                         "license-type", GTK_LICENSE_GPL_3_0,
                         /* "documenters", documenters, */
                         /* "logo", logo, */
                         /* "translator-credits", "translator-credits", */
                         "version", BACON_VERSION,
                         "website", PACKAGE_URL,
                         "website-label", "github.com/natediddy",
                         NULL);
}

static void
bacon_on_icon_view_button_press_event (GtkWidget *widget,
                                       GdkEvent *event,
                                       gpointer user_data)
{
  char *name;
  GtkTreePath *path;
  GtkTreeModel *model;
  GtkTreeIter iter;

  if (event->type == GDK_BUTTON_PRESS) {
    path = gtk_icon_view_get_path_at_pos (GTK_ICON_VIEW (widget),
                                          event->button.x, event->button.y);
    if (path) {
      model = gtk_icon_view_get_model (GTK_ICON_VIEW (widget));
      if (gtk_tree_model_get_iter (model, &iter, path)) {
        gtk_tree_model_get (model, &iter, DEVICE_CODENAME_COLUMN, &name, -1);
        switch (event->button.button) {
        case 1:
          g_message ("Click on %s", name);
          break;
        case 3:
          g_message ("Right click on %s", name);
          break;
        default:
          g_message ("That button (%i) is not recognized",
                     event->button.button);
          break;
        }
      }
      gtk_tree_path_free (path);
    }
  }
}

static gboolean
bacon_on_icon_view_query_tooltip (GtkWidget *widget,
                                  gint x,
                                  gint y,
                                  gboolean keyboard_mode,
                                  GtkTooltip *tooltip,
                                  gpointer user_data)
{
  char *codename;
  GtkTreeIter iter;
  GtkTreeModel *model;
  GtkTreePath *path;

  if (gtk_icon_view_get_tooltip_context (GTK_ICON_VIEW (widget),
                                         &x, &y, keyboard_mode,
                                         &model, &path, &iter))
  {
    gtk_tree_model_get (model, &iter, DEVICE_CODENAME_COLUMN, &codename, -1);
    gtk_tooltip_set_text (tooltip, codename);
    gtk_icon_view_set_tooltip_item (GTK_ICON_VIEW (widget), tooltip, path);
    return TRUE;
  }
  return FALSE;
}

static GtkTreeModel *
bacon_get_icon_view_tree_model (void)
{
  int x;
  GtkTreeIter iter;
  GtkListStore *store;
  BaconData *p;

  if (!g_device_list)
    g_device_list = bacon_device_list_new (false);
  bacon_init_icon_cache ();
  bacon_data_init ();

  store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING,
                              G_TYPE_STRING, GDK_TYPE_PIXBUF);
  for (p = s_data; p; p = p->next) {
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        DEVICE_FULLNAME_COLUMN, p->device->fullname,
                        DEVICE_CODENAME_COLUMN, p->device->codename,
                        DEVICE_PIXBUF_COLUMN, p->pixbuf, -1);
    if (!p->next)
      break;
  }
  return GTK_TREE_MODEL (store);
}

static GtkWidget *
bacon_get_main_window (void)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget *icon_view;
  GtkWidget *menu_bar;
  GtkWidget *help_menu;
  GtkWidget *help;
  GtkWidget *about;
  GtkTreeModel *model;

  /* setup window (contains everything) */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
  gtk_window_set_title (GTK_WINDOW (window), BACON_DISPLAY_NAME);
  gtk_window_set_default_size (GTK_WINDOW (window),
                               BACON_WINDOW_SIZE_WIDTH,
                               BACON_WINDOW_SIZE_HEIGHT);

  g_signal_connect (G_OBJECT (window), "delete-event",
                    G_CALLBACK (bacon_on_main_window_delete_event), window);

  /* setup vbox (contains menu_bar, scrolled_window, and icon_view) */
  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  /* setup menu_bar (contains help_menu, help, about) */
  menu_bar = gtk_menu_bar_new ();
  help_menu = gtk_menu_new ();

  help = gtk_menu_item_new_with_label (BACON_HELP_MENU_ITEM_LABEL);
  about = gtk_menu_item_new_with_label (BACON_ABOUT_MENU_ITEM_LABEL);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), help_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (help_menu), about);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), help);
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (about), "activate",
                    G_CALLBACK (bacon_on_menu_item_about_activate), window);

  /* scrolled_window (contains icon_view) */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);

  /* setup model (contains all the device data) */
  model = bacon_get_icon_view_tree_model ();

  /* setup icon_view (contains tree_model) */
  icon_view = gtk_icon_view_new_with_model (model);
  gtk_icon_view_set_text_column (GTK_ICON_VIEW (icon_view),
                                 DEVICE_FULLNAME_COLUMN);
  gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (icon_view),
                                   DEVICE_PIXBUF_COLUMN);
  gtk_icon_view_set_selection_mode (GTK_ICON_VIEW (icon_view),
                                    GTK_SELECTION_SINGLE);
  gtk_icon_view_set_item_width (GTK_ICON_VIEW (icon_view), -1);
  gtk_widget_set_has_tooltip (icon_view, TRUE);
  gtk_container_add (GTK_CONTAINER (scrolled_window), icon_view);

  g_signal_connect (G_OBJECT (icon_view), "button-press-event",
                    G_CALLBACK (bacon_on_icon_view_button_press_event), NULL);
  g_signal_connect (G_OBJECT (icon_view), "query-tooltip",
                    G_CALLBACK (bacon_on_icon_view_query_tooltip), NULL);

  /* return window */
  gtk_widget_show_all (window);
  return window;
}

void
bacon_gtk_main (int *argc, char ***argv)
{
  BaconThumbList *p;
  GtkWidget *window;

  gtk_init (argc, argv);
  window = bacon_get_main_window ();
  gtk_main ();

  bacon_free (s_thumbs_path);

  while (s_thumbs) {
    p = s_thumbs->next;
    bacon_free (s_thumbs);
    s_thumbs = p;
  }

  for (; s_data; s_data = s_data->next) {
    if (s_data->pixbuf)
      g_object_unref (s_data->pixbuf);
    bacon_rom_list_destroy (s_data->rom_list);
    if (!s_data->next)
      break;
  }
}
#endif /* BACON_GTK */

