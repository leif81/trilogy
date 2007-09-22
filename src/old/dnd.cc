/* TestDnD - main.c : Simple tutorial for GTK+ Drag-N-Drop
 * Copyright (C) 2005 Ryan McDougall.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <gtk/gtk.h>
#include <string.h>

/******************************************************************************/
#define _BYTE   8
#define _WORD   16
#define _DWORD  32

/******************************************************************************/
/* Define a list of data types called "targets" that a destination widget will
 * accept. The string type is arbitrary, and negotiated between DnD widgets by 
 * the developer. An enum or GQuark can serve as the integer target id. */
enum {
	TARGET_INT32,
	TARGET_STRING,
	TARGET_ROOTWIN
};

/* datatype (string), restrictions on DnD (GtkTargetFlags), datatype (int) */
static GtkTargetEntry target_list[] = {
	{ "INTEGER",    0, TARGET_INT32 },
	{ "STRING",     0, TARGET_STRING },     
	{ "text/plain", 0, TARGET_STRING },
	{ "application/x-rootwindow-drop", 0, TARGET_ROOTWIN }
};

static guint n_targets = G_N_ELEMENTS (target_list);



/******************************************************************************/
/* Signal receivable by destination */

/* Emitted when the data has been received from the source. It should check 
 * the GtkSelectionData sent by the source, and do something with it. Finally
 * it needs to finish the operation by calling gtk_drag_finish, which will emit
 * the "data-delete" signal if told to. 
 */
static void drag_data_received_handl (GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time, gpointer data)
{
	char **uris;
	char  *file;
	int    i;

	g_print("hello world, we are recving data\n");

	if (selection_data->format != 8 || selection_data->length == 0) {
		g_warning ("URI list dropped on run dialog had wrong format (%d) or length (%d)\n",
				selection_data->format,
				selection_data->length);
		return;
	}

	uris = g_uri_list_extract_uris ((const char *)selection_data->data);

	if (!uris) {
		gtk_drag_finish (context, FALSE, FALSE, time);
		return;
	}

	for (i = 0; uris [i]; i++) 
	{
		if (!uris [i] || !uris [i][0])
			continue;

		g_print ("got uri '%s'\n", uris[i]);

		/*
		file = gnome_vfs_get_local_path_from_uri (uris [i]);

		// FIXME: I assume the file is in utf8 encoding if coming from a URI? 
		if (file) 
		{
			g_print("got file %s\n", file );
			g_free (file);
		}
		*/
	}

	g_strfreev (uris);
	gtk_drag_finish (context, TRUE, FALSE, time);
}

/* Emitted when a drag is over the destination */
static gboolean drag_motion_handl (GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint t, gpointer user_data)
{
	// Fancy stuff here. This signal spams the console something horrible.
	//const gchar *name = gtk_widget_get_name (widget);
	//g_print ("%s: drag_motion_handl\n", name);
	return  FALSE;
}

/* Emitted when a drag leaves the destination */
static void drag_leave_handl (GtkWidget *widget, GdkDragContext *context, guint time, gpointer user_data)
{
	const gchar *name = gtk_widget_get_name (widget);
	g_print ("%s: drag_leave_handl\n", name);
}

/* Emitted when the user releases (drops) the selection. It should check that
 * the drop is over a valid part of the widget (if its a complex widget), and
 * itself to return true if the operation should continue. Next choose the 
 * target type it wishes to ask the source for. Finally call gtk_drag_get_data
 * which will emit "drag-data-get" on the source. 
 */ 
static gboolean drag_drop_handl (GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer user_data)
{
	gboolean        is_valid_drop_site;
	GList *target;
	GtkTargetList *list;

	const gchar *name = gtk_widget_get_name (widget);
	g_print ("%s: drag_drop_handl\n", name);

	/* Check to see if (x,y) is a valid drop site within widget */
	is_valid_drop_site = TRUE;

	for (target = context->targets; target != NULL; target = target->next) 
	{
		g_print("checking target\n");
		guint info;
		GdkAtom target_atom = GDK_POINTER_TO_ATOM (target->data);

		gtk_drag_get_data (GTK_WIDGET (widget), context, target_atom, time);

		is_valid_drop_site = TRUE;
	}

	return  is_valid_drop_site;
}


int main (int argc, char **argv)
{
	GtkWidget       *window;
	GtkWidget       *hbox;
	GtkWidget       *directions_label;
	guint           win_xsize       = 450;
	guint           win_ysize       = 50;
	guint           spacing         = 5;


	/* Always start GTK+ first! */
	gtk_init (&argc, &argv);


	/* Create the widgets */
	window  = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	hbox    = gtk_hbox_new (FALSE, spacing);


	directions_label = gtk_label_new ("drag file here");

	/* Pack the widgets */
	gtk_container_add (GTK_CONTAINER (window), hbox);

	gtk_container_add (GTK_CONTAINER (hbox), directions_label);


	/* Make the window big enough for some DnD action */
	gtk_window_set_default_size (GTK_WINDOW(window), win_xsize, win_ysize);


	/* Make the "well label" a DnD destination. */
	gtk_drag_dest_set ( window, (GtkDestDefaults) (GTK_DEST_DEFAULT_MOTION|GTK_DEST_DEFAULT_HIGHLIGHT), NULL, 0, GDK_ACTION_COPY );

	gtk_drag_dest_add_uri_targets (window);

	/* Connect the signals */
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	/* All possible destination signals */
	g_signal_connect (window, "drag-data-received", 
			G_CALLBACK(drag_data_received_handl), NULL);

	g_signal_connect (window, "drag-leave",
			G_CALLBACK (drag_leave_handl), NULL);

	g_signal_connect (window, "drag-motion",
			G_CALLBACK (drag_motion_handl), NULL);

	g_signal_connect (window, "drag-drop",
			G_CALLBACK (drag_drop_handl), NULL);

	/* Show the widgets */
	gtk_widget_show_all (window);

	/* Start the even loop */
	gtk_main ();

	return 0;
}
