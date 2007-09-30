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

#include "MediaLoader.h"
#include "MediaItem.h"

using namespace std;

typedef struct LabelItem
{
   ClutterActor     *actor;
   ClutterBehaviour *rotate_behave, *opacity_behave, *scale_behave;
};

typedef struct App
{
	ClutterTimeline *timeline;
	ClutterAlpha    *alpha_sine_inc, *alpha_ramp;
	ClutterEffectTemplate *effect_template;

	vector<MediaItem> items;
	//vector<MediaItem>::const_iterator selected_item;
	int selected_item;
	ClutterActor	*cover;
	ClutterActor	*stage;
	ClutterActor	*vbox_left;
	vector<LabelItem *> labels;
};



ClutterColor highlight_color = { 255, 0, 0, 255 };
ClutterColor normal_color = { 255, 255, 255, 255 };

void adjust_selection( App * app, int step )
{
/*
	if (i == app->selected_index)
   {
     g_object_set (item->opacity_behave,
         "opacity-start", 0x66,
         "opacity-end", 0xff,
         NULL);
     g_object_set (item->scale_behave,
         "scale-begin", 0.6,
         "scale-end", 1.0,
         NULL);
   }
*/

	app->selected_item += step;
	if( app->selected_item < 0 )
	{
		app->selected_item = 0;
	}
	else if ( app->selected_item >= app->items.size() )
	{
		app->selected_item = app->items.size() - 1;
	}

	ClutterKnot knot[2];
	
	for( int i=0; i < app->labels.size(); ++i )
	{
		ClutterActor * actor = app->labels[i]->actor;

		if( i == app->selected_item )
		{
			GdkPixbuf       *pixbuf;
			MediaItem & item = app->items[i];

			pixbuf = gdk_pixbuf_new_from_file ( item.getCoverPath().c_str(), NULL);
			if (!pixbuf)
			{
				g_error("pixbuf load failed '%s'", item.getCoverPath().c_str());
			}
			else
			{
				clutter_texture_set_pixbuf( CLUTTER_TEXTURE (app->cover), pixbuf, NULL);
			}

			clutter_label_set_color( CLUTTER_LABEL( actor ), &highlight_color );	
		}
		else
		{
			clutter_label_set_color( CLUTTER_LABEL(actor), &normal_color );	
		}

		// TODO shift the list when we get near the bottom or top
		/*
		knot[0].x = clutter_actor_get_x(actor);
		knot[0].y = clutter_actor_get_y(actor);
		knot[1].x = clutter_actor_get_x(actor);
		knot[1].y = clutter_actor_get_y(actor) + 50 * step;

		clutter_effect_move ( app->effect_template,
			actor,
			knot,
			2,
			NULL,
			NULL);
		*/
	}
		

	//	clutter_timeline_start (app->timeline);
}


void play_selection( App * app )
{
	MediaItem media = app->items[app->selected_item];
	const string cmd = "xdg-open " + media.getFilePath();
   if( system( cmd.c_str() ) == - 1 )
   {
      throw string("can't open file");
   }
}


/* input handler */
void input_cb (ClutterStage *stage, ClutterEvent *event, gpointer data)
{

	App *app = (App*)data;

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
			case CLUTTER_Escape:
			case CLUTTER_q:
				clutter_main_quit ();
				break;

			case CLUTTER_f:
				// FIXME this doesn't work - problem with clutter?
				if( fullscreen )
				{
					clutter_stage_unfullscreen( CLUTTER_STAGE (app->stage) );
				}
				else
				{
					clutter_stage_fullscreen( CLUTTER_STAGE (app->stage) );
				}
				fullscreen != fullscreen; 
				break;

			case CLUTTER_Up:

				adjust_selection( app, -1 );

				break;
			
			case CLUTTER_Down:

				adjust_selection( app, 1 );

				break;

			case CLUTTER_Page_Up:

				break;

			case CLUTTER_Page_Down:

				break;

			case CLUTTER_Home:
				
				break;

			case CLUTTER_End:

				break;

			case CLUTTER_Return:

				play_selection( app );
				
				break;
		}
	}
}


void
on_timeline_new_frame (ClutterTimeline *timeline, gint frame_num, App *app)
{
	// for each actor move 5 pixels ?
	

	if (frame_num == clutter_timeline_get_n_frames (timeline)/2)
	{
		printf("half way\n");
	}
}



LabelItem * create_label( const gchar * title )
{
	LabelItem * item = g_new0(LabelItem, 1);

	item->actor = clutter_label_new_with_text ( "Sans Bold 24", title );
	clutter_label_set_color ( CLUTTER_LABEL (item->actor), &normal_color );
	clutter_actor_show ( item->actor );

	return item;
}




int main (int argc, char *argv[])
{
	App	*app;

	ClutterActor    *stage, *hbox;
	ClutterColor    bg_color = { 0, 0, 0, 255 };

	clutter_init (&argc, &argv);

	stage = clutter_stage_get_default ();
	clutter_stage_fullscreen( CLUTTER_STAGE (stage) );
	clutter_stage_set_color( CLUTTER_STAGE (stage), &bg_color );


	// Initialize the app properties
	app = g_new0(App, 1);
	app->timeline = clutter_timeline_new ( 15, 90);
	app->alpha_ramp = clutter_alpha_new_full (
			app->timeline, CLUTTER_ALPHA_SINE_HALF, NULL, NULL);
	app->items = MediaLoader("/home/leif/Pictures/images/covers/movies").getMediaItems();
	app->selected_item = 0;
	app->effect_template
		 = clutter_effect_template_new ( app->timeline, CLUTTER_ALPHA_RAMP_INC);

	hbox = clutter_hbox_new ();
	clutter_actor_set_position( hbox, 0, 0 );
	clutter_box_set_default_padding ( CLUTTER_BOX (hbox), 10, 10, 10, 10 );
	clutter_actor_show (hbox);
	clutter_container_add_actor ( CLUTTER_CONTAINER (stage), hbox );


	// Setup the list of movie titles
	{
		app->vbox_left = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (app->vbox_left), 10, 10, 10, 10 );

		vector<MediaItem>::const_iterator it = app->items.begin();
		for( ; it < app->items.end(); ++it )
		{
			const MediaItem item = *it;

			LabelItem * label = create_label ( item.getName().c_str() );
			app->labels.push_back(label);
			clutter_box_pack_defaults ( CLUTTER_BOX (app->vbox_left), label->actor );
		}

		clutter_actor_show ( app->vbox_left );
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), app->vbox_left );
	}

	// Show the movie cover for the selected movie title
	{
		ClutterActor *vbox_right;
		GdkPixbuf       *pixbuf;

		vbox_right = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (vbox_right), 10, 10, 10, 10 );
		MediaItem item = app->items[app->selected_item];

		pixbuf = gdk_pixbuf_new_from_file ( item.getCoverPath().c_str(), NULL);
		if (!pixbuf)
		{
			g_error("pixbuf load failed '%s'", item.getCoverPath().c_str() );
		}
		else
		{
			app->cover  = clutter_texture_new_from_pixbuf (pixbuf);
		}

		clutter_actor_set_position ( app->cover, 0, 0);
		clutter_actor_show ( app->cover );

		clutter_container_add_actor ( CLUTTER_CONTAINER (vbox_right), app->cover );

		clutter_actor_show ( vbox_right );
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), vbox_right );
	}



	g_signal_connect (stage, "button-press-event", G_CALLBACK (input_cb), app);
	g_signal_connect (stage, "key-release-event", G_CALLBACK (input_cb), app);

	g_signal_connect (app->timeline,
				"new-frame",
				G_CALLBACK(on_timeline_new_frame),
				app);

	//clutter_timeline_start (app->timeline);

	adjust_selection( app, 0 );

	clutter_actor_show_all (stage);

	clutter_main();

	return 0;
}
