#include <clutter/clutter.h>

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
			clutter_actor_hide (e);
		}

	}
	else if (event->type == CLUTTER_KEY_RELEASE)
	{
		ClutterKeyEvent *kev = (ClutterKeyEvent *) event;

		g_print ("*** key press event (key:%c) ***\n",
				clutter_key_event_symbol (kev));

		if (clutter_key_event_symbol (kev) == CLUTTER_q)
		{
			clutter_main_quit ();
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

	g_signal_connect (stage, "button-press-event", G_CALLBACK (input_cb), NULL);
	g_signal_connect (stage, "key-release-event", G_CALLBACK (input_cb), NULL);


	clutter_main();

	return 0;
}
