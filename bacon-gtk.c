#include <gtk/gtk.h>

#include "bacon.h"
#include "bacon-device.h"
#include "bacon-env.h"
#include "bacon-gtk.h"
#include "bacon-hash.h"
#include "bacon-net.h"
#include "bacon-parse.h"
#include "bacon-rom.h"

#define BACON_DISPLAY_NAME         "Bacon"
#define BACON_CONFIRM_QUIT_TITLE   "Quit..."
#define BACON_CONFIRM_QUIT_MESSAGE "Are you sure you want to exit?"

static gboolean
bacon_gtk_confirm_quit (GtkWidget *parent)
{
  gint response;
  GtkWidget *dialog;

  dialog =
    gtk_message_dialog_new (GTK_WINDOW (parent),
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
bacon_gtk_on_main_window_delete_event (GtkWidget *widget, gpointer data)
{
  if (bacon_gtk_confirm_quit (widget))
    gtk_main_quit ();
  return TRUE;
}

static GtkWidget *
bacon_gtk_get_main_window (void)
{
  GtkWidget *win;
  GtkWidget *vbox;

  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (win), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size (GTK_WINDOW (win), 400, 300);
  gtk_window_set_title (GTK_WINDOW (win), BACON_DISPLAY_NAME);
  gtk_widget_show (win);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add (GTK_CONTAINER (win), vbox);
  gtk_widget_show (vbox);

  g_signal_connect (G_OBJECT (win),
                    "delete-event",
                    G_CALLBACK (bacon_gtk_on_main_window_delete_event),
                    win);

  return win;
}

void
bacon_gtk_main (int *argc, char ***argv)
{
  GtkWidget *window;

  gtk_init (argc, argv);
  window = bacon_gtk_get_main_window ();
  gtk_widget_show (window);
  gtk_main ();
}

