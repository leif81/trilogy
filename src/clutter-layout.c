/* 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either 
 * version 2 of the License, or (at your option) any later 
 * version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <clutter/clutter.h>
#include <glib.h>

/* input handler */
void input_cb (ClutterStage *stage, ClutterEvent *event, gpointer data)
{
	if (event->type == CLUTTER_BUTTON_PRESS)
	{
		ClutterButtonEvent *button_event;
		ClutterActor *e;
		gint x, y;

		clutter_event_get_coords (event, &x, &y);

		button_event = (ClutterButtonEvent *) event;
		g_print ("*** button press event (button:%d) ***\n",
				button_event->button);

		e = clutter_stage_get_actor_at_pos (stage, x, y);

		if (e)
		{
			//			clutter_actor_hide (e);
		}

	}
	else if (event->type == CLUTTER_KEY_RELEASE)
	{
		static gint fullscreen = TRUE;

		ClutterKeyEvent *kev = (ClutterKeyEvent *) event;

		g_print ("*** key press event (key:%c) ***\n",
				clutter_key_event_symbol (kev));

		switch (clutter_key_event_symbol (kev))
		{
			case CLUTTER_q:
				clutter_main_quit ();
				break;

			case CLUTTER_f:
				if( fullscreen )
				{
					clutter_stage_unfullscreen( CLUTTER_STAGE (data) );
				}
				else
				{
					clutter_stage_fullscreen( CLUTTER_STAGE (data) );
				}
				fullscreen != fullscreen; 
				break;
		}
	}
}



ClutterActor * create_title( const gchar * title )
{
	ClutterActor * label;
	ClutterColor    text_color = { 255, 255, 255, 255 };

	label = clutter_label_new_with_text ( "Sans Bold 24", title );
	clutter_label_set_color ( CLUTTER_LABEL (label), &text_color );
	clutter_actor_show ( label );

	return label;
}

int main (int argc, char *argv[])
{
	ClutterTimeline *timeline;

	ClutterActor    *stage, *hbox;
	ClutterColor    bg_color = { 0, 0, 0, 255 };


	clutter_init (&argc, &argv);

	stage = clutter_stage_get_default ();
	clutter_stage_fullscreen( CLUTTER_STAGE (stage) );
	clutter_stage_set_color( CLUTTER_STAGE (stage), &bg_color );

	hbox = clutter_hbox_new ();
	clutter_actor_set_position( hbox, 0, 0 );
	clutter_box_set_default_padding ( CLUTTER_BOX (hbox), 10, 10, 10, 10 );
	clutter_actor_show (hbox);
	clutter_container_add_actor ( CLUTTER_CONTAINER (stage), hbox );

	// Setup the list of movie titles
	{
		ClutterActor *vbox_left;
		vbox_left = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (vbox_left), 10, 10, 10, 10 );

		clutter_box_pack_defaults ( CLUTTER_BOX (vbox_left), create_title ("dawn of the dead") );
		clutter_box_pack_defaults ( CLUTTER_BOX (vbox_left), create_title ("zombie killer" ) );

		clutter_actor_show ( vbox_left );
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), vbox_left );
	}

	// Show the movie cover for the selected movie title
	{
		ClutterActor * actor, *vbox_right;

		vbox_right = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (vbox_right), 10, 10, 10, 10 );

		GdkPixbuf       *pixbuf;
		pixbuf = gdk_pixbuf_new_from_file ("../share/dawn_of_the_dead_ver2.jpg", NULL);
		if (!pixbuf)
		{
			g_error("pixbuf load failed");
		}
		else
		{
			actor  = clutter_texture_new_from_pixbuf (pixbuf);
			clutter_actor_set_position (actor, 0, 0);
			clutter_actor_show ( actor );
			clutter_container_add_actor ( CLUTTER_CONTAINER (vbox_right), actor );
		}

		clutter_actor_show ( vbox_right );
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), vbox_right );
	}


	clutter_actor_show_all (stage);

	g_signal_connect (stage, "button-press-event", G_CALLBACK (input_cb), stage);
	g_signal_connect (stage, "key-release-event", G_CALLBACK (input_cb), stage);


	clutter_main();

	return 0;
}
