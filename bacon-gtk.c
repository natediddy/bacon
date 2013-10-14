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

#include <curl/curl.h>
#include <gtk/gtk.h>

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-gtk.h"
#include "bacon-hash.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-pixbufs.h"
#include "bacon-progress.h"
#include "bacon-rom.h"
#include "bacon-search.h"
#include "bacon-util.h"

#define BACON_DEVICE_ICON_SCALE_WIDTH   100
#define BACON_DEVICE_ICON_SCALE_HEIGHT  100
#define BACON_WINDOW_DEFAULT_WIDTH      900
#define BACON_WINDOW_DEFAULT_HEIGHT     600
#define BACON_PROGRESS_WINDOW_WIDTH     200
#define BACON_PROGRESS_WINDOW_HEIGHT    100
#define BACON_PREFERENCES_WINDOW_WIDTH  450
#define BACON_PREFERENCES_WINDOW_HEIGHT 300

#define BACON_FILE_MENU_ITEM_LABEL         "File"
#define BACON_QUIT_MENU_ITEM_LABEL         "Quit"
#define BACON_EDIT_MENU_ITEM_LABEL         "Edit"
#define BACON_PREFERENCES_MENU_ITEM_LABEL  "Preferences"
#define BACON_PREFERENCES_WINDOW_TITLE     "Preferences"
#define BACON_HELP_MENU_ITEM_LABEL         "Help"
#define BACON_ABOUT_MENU_ITEM_LABEL        "About"
#define BACON_CONFIRM_QUIT_TITLE           "Quit"
#define BACON_CONFIRM_QUIT_MESSAGE         "Are you sure you want to exit?"
#define BACON_PROGRESS_WINDOW_TITLE        "Obtaining data"
#define BACON_REFRESH_DEVICES_BUTTON_LABEL "Refresh"

#define BACON_REFRESH_DEVICES_LABEL_TEXT_FORMAT \
  "<i>Currently %i devices</i>"

#define BACON_LONG_ICON_CACHE_PROGRESS_MESSAGE \
  "Retrieving icon cache " \
  "(this may take a while since a significant " \
  "portion of the cache is needed)"

#define BACON_ICON_CACHE_PROGRESS_MESSAGE \
  "Refreshing icon cache"

#define BACON_DEVICE_LIST_PROGRESS_MESSAGE \
  "Refreshing device list"

#define BACON_ROM_LIST_PROGRESS_MESSAGE_FORMAT \
  "Retrieving ROM list for %s"

#define BACON_ROM_PROGRESS_MESSAGE_FORMAT "Downloading %s"
#define BACON_SEARCH_LABEL                "<b>Search devices:</b> "
#define BACON_CODENAME_COLOR              "dark blue"
#define BACON_DEVICE_COUNT_COLOR          "dark blue"

#define BACON_DISPLAY_NAME_MARKUP_FORMAT \
  "<b>%s <span color=\"" BACON_CODENAME_COLOR "\">%s</span></b>"

#define BACON_DEVICE_COUNT_LABEL_DEFAULT "<b>Devices:</b>"

#define BACON_DEVICE_COUNT_MARKUP_FORMAT \
  BACON_DEVICE_COUNT_LABEL_DEFAULT " <b><span color=\"" \
  BACON_DEVICE_COUNT_COLOR "\">%3i</span></b>"

typedef struct BaconData           BaconData;
typedef struct BaconThumbList      BaconThumbList;

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

typedef enum {
  BACON_PROGRESS_TYPE_NONE,
  BACON_PROGRESS_TYPE_LONG_ICON_CACHE,
  BACON_PROGRESS_TYPE_ICON_CACHE,
  BACON_PROGRESS_TYPE_DEVICE_LIST,
  BACON_PROGRESS_TYPE_ROM_LIST
} BaconProgressType;

enum {
  DEVICE_DISPLAY_NAME_COLUMN,
  DEVICE_FULLNAME_COLUMN,
  DEVICE_CODENAME_COLUMN,
  DEVICE_PIXBUF_COLUMN,
  N_COLUMNS
};

extern BaconDeviceList * g_device_list;
extern char *            g_program_data_path;
static BaconProgressType s_progress_type      = BACON_PROGRESS_TYPE_NONE;
static BaconDevice *     s_device             = NULL;
static BaconThumbList *  s_thumbs             = NULL;
static BaconData *       s_data               = NULL;
static GtkWindow *       s_progress_window    = NULL;
static GtkProgressBar *  s_progress_bar       = NULL;
static GtkEntryBuffer *  s_entry_buffer       = NULL;
static GtkIconView *     s_icon_view          = NULL;
static GtkLabel *        s_device_count_label = NULL;
static GtkTreeModel *    s_model              = NULL;
static GtkWindow *       s_window             = NULL;
static gchar *           s_thumbs_path        = NULL;
static gint              s_device_count       = 0;

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

static GtkWidget *
bacon_hbox_new (gint spacing)
{
#if GTK_CHECK_VERSION (3, 2, 0)
  return gtk_box_new (GTK_ORIENTATION_HORIZONTAL, spacing);
#else
  return gtk_hbox_new (FALSE, spacing);
#endif
}

static GtkWidget *
bacon_vbox_new (gint spacing)
{
#if GTK_CHECK_VERSION (3, 2, 0)
  return gtk_box_new (GTK_ORIENTATION_VERTICAL, spacing);
#else
  return gtk_vbox_new (FALSE, spacing);
#endif
}

static void
bacon_init_progress_window (BaconProgressType type)
{
  gint w;
  gint h;
  gchar *message;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *image;

  switch (type) {
  case BACON_PROGRESS_TYPE_LONG_ICON_CACHE:
    message = g_markup_escape_text (BACON_LONG_ICON_CACHE_PROGRESS_MESSAGE,
                                    -1);
    break;
  case BACON_PROGRESS_TYPE_ICON_CACHE:
    message = g_markup_escape_text (BACON_ICON_CACHE_PROGRESS_MESSAGE, -1);
    break;
  case BACON_PROGRESS_TYPE_DEVICE_LIST:
    message = g_markup_escape_text (BACON_DEVICE_LIST_PROGRESS_MESSAGE, -1);
    break;
  case BACON_PROGRESS_TYPE_ROM_LIST:
    message = g_markup_printf_escaped (BACON_ROM_LIST_PROGRESS_MESSAGE_FORMAT,
                                       s_device->codename);
    break;
  default:
    message = NULL;
    break;
  }

  s_progress_type = type;
  if (s_progress_window)
    gtk_widget_destroy (GTK_WIDGET (s_progress_window));

  s_progress_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title (s_progress_window, BACON_PROGRESS_WINDOW_TITLE);
  gtk_window_set_transient_for (s_progress_window, s_window);
  gtk_window_set_modal (s_progress_window, TRUE);
  gtk_window_set_decorated (s_progress_window, TRUE);
  gtk_window_set_position (s_progress_window, GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_window_set_deletable (s_progress_window, FALSE);
  gtk_window_set_default_size (s_progress_window,
                               BACON_PROGRESS_WINDOW_WIDTH,
                               BACON_PROGRESS_WINDOW_HEIGHT);

  vbox = bacon_vbox_new (0);
  gtk_container_add (GTK_CONTAINER (s_progress_window), vbox);

  hbox = bacon_hbox_new (0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  image = gtk_image_new_from_stock (GTK_STOCK_INFO, GTK_ICON_SIZE_DIALOG);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new (message);
  gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  s_progress_bar = GTK_PROGRESS_BAR (gtk_progress_bar_new ());
  gtk_progress_bar_set_show_text (s_progress_bar, TRUE);
  gtk_progress_bar_set_text (s_progress_bar, "");
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (s_progress_bar),
                      TRUE, FALSE, 0);

  g_free (message);
  gtk_widget_show_all (GTK_WIDGET (s_progress_window));
}

static void
bacon_finish_progress_window (void)
{
  gtk_widget_destroy (GTK_WIDGET (s_progress_window));
}

static void
bacon_update_progress_bar (gdouble total, gdouble current)
{
  gdouble fraction;

  fraction = (current / total);
  if (bacon_nan_value (fraction))
    return;
  gtk_progress_bar_set_fraction (s_progress_bar, fraction);
}

static void
bacon_init_icon_cache (void)
{
  guint x;
  guint icons_needed;
  guint icons_total;
  char *data;
  char *iconpath;
  BaconDeviceThumbRequestList *p;
  BaconDeviceThumbRequestList *thumbs;
  GtkWidget *progress_dialog;

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
  icons_needed = 0;
  icons_total = 0;

  for (p = thumbs; p; p = p->next) {
    bacon_add_thumb (p->filename);
    iconpath = bacon_full_icon_path (p->filename);
    icons_total++;
    if (!bacon_env_is_file (iconpath))
      icons_needed++;
    g_free (iconpath);
    if (!p->next)
      break;
  }

  g_message ("total icons: %i needed icons:%i", icons_total, icons_needed);

  if (icons_needed) {
    if (icons_needed >= (icons_total / 3))
      bacon_init_progress_window (BACON_PROGRESS_TYPE_LONG_ICON_CACHE);
    else
      bacon_init_progress_window (BACON_PROGRESS_TYPE_ICON_CACHE);
    x = 0;
    for (p = thumbs; p; p = p->next) {
      iconpath = bacon_full_icon_path (p->filename);
      if (!bacon_env_is_file (iconpath)) {
        g_message ("downloading icon `%s'\n", iconpath);
        if (bacon_net_init_for_device_icon_thumb (p->request, iconpath)) {
          if (!bacon_net_get_file ())
            g_warning ("failed to download icon from %s/%s\n",
                        BACON_DEVICE_ICON_THUMB_URL, p->request);
          x++;
          bacon_net_deinit ();
        }
        bacon_update_progress_bar ((gdouble) icons_needed, (gdouble) x);
        while (gtk_events_pending ())
          gtk_main_iteration ();
      }
      g_free (iconpath);
      if (!p->next)
        break;
    }
    bacon_finish_progress_window ();
  }
}

static void
bacon_init_data (void)
{
  gchar *iconpath;
  GdkPixbuf *i;
  GError *error;
  BaconData *dp;
  BaconDeviceList *p;
  BaconThumbList *tp;

  error = NULL;

  for (p = g_device_list; p; p = p->next) {
    bacon_list_append (BaconData, s_data, dp);
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
      dp->pixbuf = gdk_pixbuf_new_from_inline (-1, s_fallback_device_icon,
                                               FALSE, &error);
      if (error) {
        g_warning ("failed to create pixbuf from inline: %s", error->message);
        g_error_free (error);
        dp->pixbuf = NULL;
      }
    }
    dp->rom_list = NULL;
    bacon_list_rewind (s_data, dp);
    if (!p->next)
      break;
  }

  if (error)
    g_error_free (error);
}

static void
bacon_refresh_device_list (void)
{
  char *data;

  if (g_device_list)
    bacon_device_list_destroy (g_device_list);
  data = NULL;
  bacon_init_progress_window (BACON_PROGRESS_TYPE_DEVICE_LIST);
  if (bacon_net_gtk_init_for_device_list (s_progress_bar)) {
    data = bacon_net_get_page_data ();
    if (data)
      g_device_list = bacon_parse_for_device_list (data, false);
    bacon_net_deinit ();
  }
  bacon_finish_progress_window ();
}

static gint
bacon_entry_buffer_space_before_pos (gint pos)
{
  const gchar *text;
  gint x;

  if (pos == 0)
    return -1;

  text = gtk_entry_buffer_get_text (s_entry_buffer);
  for (x = pos - 2; x > 0; --x)
    if (g_ascii_isspace (text[x]))
      return x;
  return -1;
}

static gint
bacon_entry_buffer_space_after_pos (gint pos)
{
  const gchar *text;
  gint x;

  text = gtk_entry_buffer_get_text (s_entry_buffer);
  for (x = pos + 1; x < gtk_entry_buffer_get_length (s_entry_buffer); ++x)
    if (g_ascii_isspace (text[x]))
      return x;
  return -1;
}

static void
bacon_entry_buffer_insert_char (gint pos, gchar c)
{
  gchar buffer[2];

  buffer[0] = c;
  buffer[1] = '\0';
  gtk_entry_buffer_insert_text (s_entry_buffer, pos, buffer, 1);
}

static gboolean
bacon_confirm_quit (void)
{
  gint response;
  GtkWidget *dialog;

  dialog = gtk_message_dialog_new (s_window,
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

static void
bacon_set_icon_view_attributes (void)
{
  gtk_icon_view_set_row_spacing (s_icon_view, 0);
  gtk_icon_view_set_column_spacing (s_icon_view, 0);
  gtk_icon_view_set_margin (s_icon_view, 0);
  gtk_icon_view_set_item_width (s_icon_view, 200);
  gtk_icon_view_set_columns (s_icon_view, -1);
}

static void
bacon_set_model (void)
{
  gchar *display_name;
  gchar *label_markup;
  GtkTreeIter iter;
  GtkListStore *store;
  BaconData *p;
  BaconSearchTokenList *token_list;

  if (!g_device_list)
    bacon_refresh_device_list ();

  if (!s_data) {
    if (!s_thumbs) {
      gtk_window_set_auto_startup_notification (FALSE);
      bacon_init_icon_cache ();
      gtk_window_set_auto_startup_notification (TRUE);
    }
    bacon_init_data ();
  }

  display_name = NULL;
  store = gtk_list_store_new (N_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              G_TYPE_STRING,
                              GDK_TYPE_PIXBUF);
  s_device_count = 0;
  token_list =
    bacon_search_token_list_new (gtk_entry_buffer_get_text (s_entry_buffer));

  for (p = s_data; p; p = p->next) {
    if (token_list &&
        (!bacon_search (p->device->fullname, token_list) &&
         !bacon_search (p->device->codename, token_list)))
      continue;
    s_device_count++;
    display_name = g_markup_printf_escaped (BACON_DISPLAY_NAME_MARKUP_FORMAT,
                                            p->device->fullname,
                                            p->device->codename);
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        DEVICE_DISPLAY_NAME_COLUMN, display_name,
                        DEVICE_FULLNAME_COLUMN, p->device->fullname,
                        DEVICE_CODENAME_COLUMN, p->device->codename,
                        DEVICE_PIXBUF_COLUMN, p->pixbuf, -1);
    g_free (display_name);
    if (!p->next)
      break;
  }

  bacon_search_token_list_free (token_list);

  s_model = GTK_TREE_MODEL (store);
  if (s_icon_view) {
    gtk_icon_view_set_model (s_icon_view, s_model);
    bacon_set_icon_view_attributes ();
  }

  if (s_device_count_label) {
    label_markup = g_markup_printf_escaped (BACON_DEVICE_COUNT_MARKUP_FORMAT,
                                            s_device_count);
    gtk_label_set_markup (s_device_count_label, label_markup);
    g_free (label_markup);
  }
}

static GtkWidget *
bacon_create_table (void)
{
  GtkWidget *table;

  table = gtk_table_new (1, 2, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 12);
  gtk_table_set_col_spacing (GTK_TABLE (table), 0, 12);
  gtk_table_set_row_spacings (GTK_TABLE (table), 6);
  return table;
}

static GtkWidget *
bacon_new_table_row (GtkWidget *widget)
{
  GtkWidget *alignment;
  GtkWidget *box;

  box = bacon_hbox_new (0);

  alignment = gtk_alignment_new (0.0, 0.0, 0.0, 0.0);
  gtk_widget_set_size_request (alignment, 18, 0);
  gtk_box_pack_start (GTK_BOX (box), alignment, FALSE, FALSE, 0);

  if (GTK_IS_MISC (widget))
    gtk_misc_set_alignment (GTK_MISC (widget), 0.0, 0.5);
  if (GTK_IS_LABEL (widget))
    gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
  gtk_box_pack_start (GTK_BOX (box), widget, TRUE, TRUE, 0);
  return box;
}

static void
bacon_attach_table_row (GtkWidget *table, guint *row, GtkWidget *widget)
{
  GtkWidget *table_row;

  table_row = bacon_new_table_row (widget);
  gtk_table_attach (GTK_TABLE (table), table_row, 0, 2, *row, *row + 1,
                    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                    GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                    0, 0);
  ++*row;
}

static void
bacon_show_preferences_dialog (void)
{
  guint row;
  gchar *refresh_devices_label_text;
  GtkWidget *window;
  GtkWidget *table;
  GtkWidget *refresh_devices_box;
  GtkWidget *refresh_devices_label;
  GtkWidget *refresh_devices_button;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_transient_for (GTK_WINDOW (window), s_window);
  gtk_window_set_title (GTK_WINDOW (window), BACON_PREFERENCES_WINDOW_TITLE);
  gtk_window_set_modal (GTK_WINDOW (window), TRUE);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
  /*gtk_window_set_default_size (GTK_WINDOW (window),
                               BACON_PREFERENCES_WINDOW_WIDTH,
                               BACON_PREFERENCES_WINDOW_HEIGHT);*/

  row = 0;
  table = bacon_create_table ();
  gtk_container_add (GTK_CONTAINER (window), table);

  refresh_devices_box = bacon_hbox_new (12);

  refresh_devices_label_text =
    g_markup_printf_escaped (BACON_REFRESH_DEVICES_LABEL_TEXT_FORMAT,
                             bacon_device_list_total (g_device_list));
  refresh_devices_label = gtk_label_new (refresh_devices_label_text);
  gtk_label_set_use_markup (GTK_LABEL (refresh_devices_label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (refresh_devices_label), 0.0f, 0.5f);
  gtk_box_pack_start (GTK_BOX (refresh_devices_box),
                      refresh_devices_label, TRUE, TRUE, 0);

  refresh_devices_button =
    gtk_button_new_with_label (BACON_REFRESH_DEVICES_BUTTON_LABEL);
  gtk_box_pack_start (GTK_BOX (refresh_devices_box),
                      refresh_devices_button, FALSE, FALSE, 0);

  bacon_attach_table_row (table, &row, refresh_devices_box);
  gtk_widget_show_all (window);
}

static gboolean
bacon_on_main_window_delete_event (GtkWidget *widget, gpointer data)
{
  if (bacon_confirm_quit ()) {
    gtk_main_quit ();
    return FALSE;
  }
  return TRUE;
}

static void
bacon_on_menu_item_quit_activate (GtkMenuItem *menu_item, gpointer user_data)
{
  if (bacon_confirm_quit ())
    gtk_main_quit ();
}

static void
bacon_on_menu_item_preferences_activate (GtkMenuItem *menu_item,
                                         gpointer user_data)
{
  bacon_show_preferences_dialog ();
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
          BACON_PROGRAM_NAME " is a tool for viewing and downloading "
          "CyanogenMod ROMs for Android handsets\n"
          "For more information about the CyanogenMod project, visit "
          "http://www.cyanogenmod.org/\n"
          "For more information about Android, visit http://www.android.com/";


  gtk_show_about_dialog (s_window,
                         "program-name", BACON_PROGRAM_NAME,
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
  GtkTreeIter iter;

  if (event->type == GDK_BUTTON_PRESS) {
    path = gtk_icon_view_get_path_at_pos (GTK_ICON_VIEW (widget),
                                          event->button.x, event->button.y);
    if (path) {
      if (gtk_tree_model_get_iter (s_model, &iter, path)) {
        gtk_tree_model_get (s_model, &iter,
                            DEVICE_CODENAME_COLUMN, &name, -1);
        gtk_icon_view_item_activated (GTK_ICON_VIEW (widget), path);
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
  GtkTreePath *path;

  if (gtk_icon_view_get_tooltip_context (GTK_ICON_VIEW (widget),
                                         &x, &y, keyboard_mode,
                                         &s_model, &path, &iter))
  {
    gtk_tree_model_get (s_model, &iter,
                        DEVICE_CODENAME_COLUMN, &codename, -1);
    gtk_tooltip_set_text (tooltip, codename);
    gtk_icon_view_set_tooltip_item (GTK_ICON_VIEW (widget), tooltip, path);
    return TRUE;
  }
  return FALSE;
}

static gboolean
bacon_on_entry_key_press_event (GtkWidget *widget,
                                GdkEvent *event,
                                gpointer user_data)
{
  gchar buffer[2];
  const gchar *text;
  gint cursor_pos;
  gint space_pos;
  guint i;
  guint j;

  if (event->type != GDK_KEY_PRESS)
    return;

  g_object_get (G_OBJECT (widget), "cursor-position", &cursor_pos, NULL);

  switch (event->key.keyval) {
  case GDK_KEY_A: case GDK_KEY_B: case GDK_KEY_C: case GDK_KEY_D:
  case GDK_KEY_E: case GDK_KEY_F: case GDK_KEY_G: case GDK_KEY_H:
  case GDK_KEY_I: case GDK_KEY_J: case GDK_KEY_K: case GDK_KEY_L:
  case GDK_KEY_M: case GDK_KEY_N: case GDK_KEY_O: case GDK_KEY_P:
  case GDK_KEY_Q: case GDK_KEY_R: case GDK_KEY_S: case GDK_KEY_T:
  case GDK_KEY_U: case GDK_KEY_V: case GDK_KEY_W: case GDK_KEY_X:
  case GDK_KEY_Y: case GDK_KEY_Z:
  case GDK_KEY_a: case GDK_KEY_b: case GDK_KEY_c: case GDK_KEY_d:
  case GDK_KEY_e: case GDK_KEY_f: case GDK_KEY_g: case GDK_KEY_h:
  case GDK_KEY_i: case GDK_KEY_j: case GDK_KEY_k: case GDK_KEY_l:
  case GDK_KEY_m: case GDK_KEY_n: case GDK_KEY_o: case GDK_KEY_p:
  case GDK_KEY_q: case GDK_KEY_r: case GDK_KEY_s: case GDK_KEY_t:
  case GDK_KEY_u: case GDK_KEY_w: case GDK_KEY_x: case GDK_KEY_y:
  case GDK_KEY_z:
  case GDK_KEY_space: case GDK_KEY_exclam: case GDK_KEY_quotedbl:
  case GDK_KEY_numbersign: case GDK_KEY_dollar: case GDK_KEY_percent:
  case GDK_KEY_ampersand: case GDK_KEY_apostrophe: case GDK_KEY_parenleft:
  case GDK_KEY_parenright: case GDK_KEY_asterisk: case GDK_KEY_plus:
  case GDK_KEY_comma: case GDK_KEY_minus: case GDK_KEY_period:
  case GDK_KEY_slash:
  case GDK_KEY_0: case GDK_KEY_1: case GDK_KEY_2: case GDK_KEY_3:
  case GDK_KEY_4: case GDK_KEY_5: case GDK_KEY_6: case GDK_KEY_7:
  case GDK_KEY_8: case GDK_KEY_9:
  case GDK_KEY_colon: case GDK_KEY_semicolon: case GDK_KEY_less:
  case GDK_KEY_equal: case GDK_KEY_greater: case GDK_KEY_question:
  case GDK_KEY_bracketleft: case GDK_KEY_backslash: case GDK_KEY_bracketright:
  case GDK_KEY_asciicircum: case GDK_KEY_underscore: case GDK_KEY_quoteleft:
  case GDK_KEY_braceleft: case GDK_KEY_bar: case GDK_KEY_braceright:
  case GDK_KEY_asciitilde:
    bacon_entry_buffer_insert_char (cursor_pos, event->key.keyval);
    cursor_pos++;
    break;
  case GDK_KEY_v:
    if (event->key.state & GDK_CONTROL_MASK) {
      /* copy text from clipboard */;
    } else {
      bacon_entry_buffer_insert_char (cursor_pos, event->key.keyval);
      cursor_pos++;
    }
    break;
  case GDK_KEY_BackSpace:
    if (event->key.state & GDK_CONTROL_MASK) {
      space_pos = bacon_entry_buffer_space_before_pos (cursor_pos);
      if (space_pos != -1) {
        gtk_entry_buffer_delete_text (s_entry_buffer, space_pos,
                                      cursor_pos - space_pos);
        cursor_pos = space_pos;
      } else {
        gtk_entry_buffer_delete_text (s_entry_buffer, 0, cursor_pos);
        cursor_pos = 0;
      }
    } else {
      gtk_entry_buffer_delete_text (s_entry_buffer, cursor_pos - 1, 1);
      cursor_pos--;
    }
    break;
  case GDK_KEY_Delete:
    if (event->key.state & GDK_CONTROL_MASK) {
      space_pos = bacon_entry_buffer_space_after_pos (cursor_pos);
      if (space_pos != -1)
        gtk_entry_buffer_delete_text (s_entry_buffer, cursor_pos,
                                      space_pos - cursor_pos + 1);
      else
        gtk_entry_buffer_delete_text (s_entry_buffer, cursor_pos,
                                      gtk_entry_buffer_get_length (
                                        s_entry_buffer));
    } else
      gtk_entry_buffer_delete_text (s_entry_buffer, cursor_pos, 1);
    break;
  case GDK_KEY_Left:
    if (event->key.state & GDK_CONTROL_MASK) {
      space_pos = bacon_entry_buffer_space_before_pos (cursor_pos);
      if (space_pos != -1)
        cursor_pos = space_pos + 1;
      else
        cursor_pos = 0;
    } else
      cursor_pos--;
    break;
  case GDK_KEY_Right:
    if (event->key.state & GDK_CONTROL_MASK) {
      space_pos = bacon_entry_buffer_space_after_pos (cursor_pos);
      if (space_pos != -1)
        cursor_pos = space_pos;
      else
        cursor_pos = gtk_entry_buffer_get_length (s_entry_buffer);
    } else
      cursor_pos++;
    break;
  default:
    ;
  }

  gtk_entry_set_buffer (GTK_ENTRY (widget), s_entry_buffer);
  g_signal_emit_by_name (widget, "move-cursor",
                         GTK_MOVEMENT_VISUAL_POSITIONS,
                         cursor_pos, FALSE, NULL);
  bacon_set_model ();
}

static GtkWidget *
bacon_get_menu_bar (void)
{
  GtkWidget *menu_bar;
  GtkWidget *file_menu;
  GtkWidget *edit_menu;
  GtkWidget *help_menu;
  GtkWidget *file;
  GtkWidget *quit;
  GtkWidget *edit;
  GtkWidget *preferences;
  GtkWidget *help;
  GtkWidget *about;

  menu_bar = gtk_menu_bar_new ();
  file_menu = gtk_menu_new ();
  edit_menu = gtk_menu_new ();
  help_menu = gtk_menu_new ();

  file = gtk_menu_item_new_with_label (BACON_FILE_MENU_ITEM_LABEL);
  quit = gtk_menu_item_new_with_label (BACON_QUIT_MENU_ITEM_LABEL);
  edit = gtk_menu_item_new_with_label (BACON_EDIT_MENU_ITEM_LABEL);
  preferences =
    gtk_menu_item_new_with_label (BACON_PREFERENCES_MENU_ITEM_LABEL);
  help = gtk_menu_item_new_with_label (BACON_HELP_MENU_ITEM_LABEL);
  about = gtk_menu_item_new_with_label (BACON_ABOUT_MENU_ITEM_LABEL);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (file), file_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (file_menu), quit);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), file);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (edit), edit_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (edit_menu), preferences);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), edit);

  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help), help_menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (help_menu), about);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu_bar), help);

  g_signal_connect (G_OBJECT (quit), "activate",
                    G_CALLBACK (bacon_on_menu_item_quit_activate), NULL);
  g_signal_connect (G_OBJECT (preferences),
                    "activate",
                    G_CALLBACK (bacon_on_menu_item_preferences_activate),
                    NULL);
  g_signal_connect (G_OBJECT (about), "activate",
                    G_CALLBACK (bacon_on_menu_item_about_activate), NULL);
  return menu_bar;
}

static void
bacon_init_main_window (void)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *entry;
  GtkWidget *search_label;
  GtkWidget *scrolled_window;
  GtkWidget *menu_bar;

  /* setup s_window (contains everything) */
  s_window = GTK_WINDOW (gtk_window_new (GTK_WINDOW_TOPLEVEL));
  gtk_window_set_position (s_window, GTK_WIN_POS_CENTER);
  gtk_window_set_resizable (s_window, TRUE);
  gtk_window_set_title (s_window, BACON_PROGRAM_NAME);
  gtk_window_set_default_size (s_window,
                               BACON_WINDOW_DEFAULT_WIDTH,
                               BACON_WINDOW_DEFAULT_HEIGHT);

  g_signal_connect (G_OBJECT (s_window), "delete-event",
                    G_CALLBACK (bacon_on_main_window_delete_event), NULL);

  /* setup vbox (contains hbox, menu_bar, scrolled_window, and icon_view) */
  vbox = bacon_vbox_new (0);
  gtk_container_add (GTK_CONTAINER (s_window), vbox);

  /* setup menu_bar */
  menu_bar = bacon_get_menu_bar ();
  gtk_box_pack_start (GTK_BOX (vbox), menu_bar, FALSE, FALSE, 0);

  /* setup hbox (contains entry and search_label) */
  hbox = bacon_hbox_new (0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  /* setup entry (contains s_entry_buffer) */
  entry = gtk_entry_new ();
  s_entry_buffer = gtk_entry_buffer_new (NULL, -1);
  gtk_entry_set_buffer (GTK_ENTRY (entry), s_entry_buffer);

  /* setup search_label */
  search_label = gtk_label_new (BACON_SEARCH_LABEL);
  gtk_label_set_use_markup (GTK_LABEL (search_label), TRUE);

  s_device_count_label =
    GTK_LABEL (gtk_label_new (BACON_DEVICE_COUNT_LABEL_DEFAULT));
  gtk_label_set_use_markup (s_device_count_label, TRUE);

  gtk_box_pack_end (GTK_BOX (hbox), GTK_WIDGET (s_device_count_label),
                    FALSE, FALSE, 10);
  gtk_box_pack_end (GTK_BOX (hbox), entry, FALSE, FALSE, 10);
  gtk_box_pack_end (GTK_BOX (hbox), search_label, FALSE, FALSE, 10);

  g_signal_connect (G_OBJECT (entry), "key-press-event",
                    G_CALLBACK (bacon_on_entry_key_press_event), NULL);

  /* setup scrolled_window (contains s_icon_view) */
  scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window),
                                       GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);

  /* setup icon_view (contains s_model) */
  s_icon_view = GTK_ICON_VIEW (gtk_icon_view_new_with_model (s_model));
  gtk_icon_view_set_markup_column (s_icon_view, DEVICE_DISPLAY_NAME_COLUMN);
  gtk_icon_view_set_pixbuf_column (s_icon_view, DEVICE_PIXBUF_COLUMN);
  gtk_icon_view_set_selection_mode (s_icon_view, GTK_SELECTION_SINGLE);
  gtk_widget_set_has_tooltip (GTK_WIDGET (s_icon_view), TRUE);
  bacon_set_icon_view_attributes ();
  gtk_scrolled_window_add_with_viewport
    (GTK_SCROLLED_WINDOW (scrolled_window), GTK_WIDGET (s_icon_view));

  g_signal_connect (G_OBJECT (s_icon_view), "button-press-event",
                    G_CALLBACK (bacon_on_icon_view_button_press_event), NULL);
  g_signal_connect (G_OBJECT (s_icon_view), "query-tooltip",
                    G_CALLBACK (bacon_on_icon_view_query_tooltip), NULL);

  /* show everything in s_window */
  gtk_widget_show_all (GTK_WIDGET (s_window));
  gtk_widget_grab_focus (entry);

  bacon_set_model ();
}

void
bacon_gtk_main (int *argc, char ***argv)
{
  BaconThumbList *p;

  gtk_init (argc, argv);
  bacon_init_main_window ();
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

