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
#include <glib/gprintf.h>

#include "MediaLoader.h"
#include "MediaItem.h"

#include <iostream>

using namespace std;

class LabelItem
{
	public:
		LabelItem( MediaItem & mi ) : media(mi), visible(true) {};

		MediaItem media;
		bool visible;
		ClutterActor     *actor;
		ClutterBehaviour *rotate_behave, *opacity_behave, *scale_behave;
};

typedef struct App
{
	ClutterTimeline *timeline;
	ClutterAlpha    *alpha_sine_inc, *alpha_ramp;
	ClutterEffectTemplate *effect_template;

	ClutterActor	*cover;
	ClutterActor	*stage;
	ClutterActor	*vbox_left;
	ClutterActor   *vbox_right;
	ClutterActor   *filter;

	vector<LabelItem *> labels;
	vector<LabelItem *>::iterator selected_item;

	bool playing;
};



/**
 * Change the selected label item by the step amount
 *
 * @brief step	move this many locations from the current selection
 */
vector<LabelItem*>::iterator find_label(int step, App * app)
{
	vector<LabelItem*>::iterator find = app->selected_item;

	while( step != 0 )
	{
		if( step > 0 )
		{
			vector<LabelItem*>::iterator prev = find;
			find++;
			if( find == app->labels.end() )
			{
				return prev;
			}

			LabelItem * label = *find;
			if( label->visible )
			{
				step--;
			}
		}
		else if( step < 0 )
		{
			if( find == app->labels.begin() )
			{
				return find;
			}

			find--;
			LabelItem * label = *find;
			if( label->visible )
			{
				step++;
			}
		}
	}
	return find;
}



bool search_mode =false;
string search_string;

ClutterColor highlight_color = { 255, 255, 0, 255 };
ClutterColor normal_color = { 255, 255, 255, 155 };

// shift the list when we get near the bottom or top
void shift_list( App * app, int step )
{
	ClutterKnot knot[2];

	gint top_padding, bottom_padding;

	clutter_box_get_default_padding( CLUTTER_BOX(app->vbox_left), &top_padding, NULL, &bottom_padding, NULL );

	ClutterBoxChild * child;
	
	for( int i=0; clutter_box_query_nth_child( CLUTTER_BOX(app->vbox_left), i, child ); ++i )
	{
		ClutterActor * actor = CLUTTER_ACTOR( child );

		knot[0].x = clutter_actor_get_x(actor);
		knot[0].y = clutter_actor_get_y(actor);
		knot[1].x = clutter_actor_get_x(actor);
		knot[1].y = clutter_actor_get_y(actor) - (clutter_actor_get_height(actor) + top_padding + bottom_padding) * step;

		clutter_effect_move ( app->effect_template,
			actor,
			knot,
			2,
			NULL,
			NULL);
	}
}


LabelItem * select_label( vector<LabelItem*>::iterator it, App *app )
{
	app->selected_item = it;
	LabelItem * label = *it;

	GdkPixbuf       *pixbuf;

	pixbuf = gdk_pixbuf_new_from_file ( label->media.getCoverPath().c_str(), NULL);

	if (!pixbuf)
	{
		//g_error("pixbuf load failed '%s'", item.getCoverPath().c_str());

		// Just create a 1x1 transparent pixbuf and draw that 
		pixbuf = gdk_pixbuf_new ( GDK_COLORSPACE_RGB, 1, 8, 1, 1);
	}

	clutter_texture_set_pixbuf( CLUTTER_TEXTURE (app->cover), pixbuf, NULL);

	// HACK until we can do right justify properly for the contents of the vbox
	clutter_actor_set_position( app->vbox_right, 
			CLUTTER_STAGE_WIDTH() - 100 - gdk_pixbuf_get_width(pixbuf), 
			CLUTTER_STAGE_HEIGHT() / 2 - gdk_pixbuf_get_height(pixbuf) / 2 );

	clutter_label_set_color( CLUTTER_LABEL( label->actor ), &highlight_color );	

#ifdef SCALE_LABEL
	g_object_set (label->scale_behave,
		"scale-begin", 1.0,
		"scale-end", 1.05,
		NULL);
#endif

	return label;
}

static bool first_movement = true;

void make_selection( App * app, vector<LabelItem*>::iterator it_selected )
{
//	cout << "moving" << endl;

	if( !first_movement )
	{
		LabelItem *prev = *app->selected_item;
#ifdef SCALE_LABEL
		g_object_set (prev->scale_behave,
			"scale-begin", 1.05,
			"scale-end", 1.0,
			NULL);
#endif
		clutter_label_set_color( CLUTTER_LABEL(prev->actor), &normal_color );	
	}
	else
	{
		first_movement = false;
	}



	// FIXME this whole query thing doesn't seem to work at all
#ifdef SCALE_LABEL
	ClutterBoxChild * child;
	for( int i=0; clutter_box_query_nth_child( CLUTTER_BOX(app->vbox_left), i, child ); ++i )
	{
		ClutterActor * actor = CLUTTER_ACTOR( child );
		g_object_set (actor->scale_behave,
			"scale-begin", 1.0,
			"scale-end", 1.0,
			NULL);
	}
#endif
	
	select_label( it_selected, app);

#if SCROLL_LIST
	// Handle selected item
		{

			LabelItem * label = *app->selected_item;

			// check if we need to shift list to stay on screen
			{
				gint x,y;
				clutter_actor_get_abs_position( label->actor, &x, &y );
				const int margin = 10; // pixels from edge of screen that triggers shift

				if( step > 0 )
				{
					if( y + clutter_actor_get_height(label->actor) +  margin  > CLUTTER_STAGE_HEIGHT() )
					{
						shift_list(app, step);
					}
				}
				else
				{
					if( y - margin  < 0 )
					{
						shift_list(app, step);
					}
				}
			}

		}
#endif

//	clutter_timeline_start (app->timeline);
}


void make_selection( App * app, int step )
{
	make_selection( app, find_label( step, app ) );
}


void redraw_list( const string & filter, App * app )
{

	clutter_box_remove_all( CLUTTER_BOX (app->vbox_left ) );

	bool first_label = true;

	vector<LabelItem*>::iterator it = app->labels.begin();
	for( ; it != app->labels.end(); ++it )
	{
		LabelItem * label = *it;

		const string text = clutter_label_get_text( CLUTTER_LABEL(label->actor) );
		if( filter.empty() || text.find( filter ) != string::npos )
		{
			clutter_box_pack_defaults ( CLUTTER_BOX (app->vbox_left), label->actor );
			label->visible = true;

			// Highlight the first item in the list when searching
			if( search_mode && first_label )
			{
				first_label = false;	
				make_selection(app, it);
			}
		}
		else
		{
			label->visible = false;
		}
	}
}


void loadMedia( App * app, const string & catalog_path )
{
	try
	{
		app->labels.clear();
		clutter_box_remove_all( CLUTTER_BOX (app->vbox_left ) );

		MediaLoader loader(catalog_path);
		vector<MediaItem> items = loader.getMediaItems();
		vector<MediaItem>::iterator it = items.begin();
		for( ; it < items.end(); ++it )
		{
			MediaItem &item = *it;

			LabelItem * label = new LabelItem(item);
			label->actor = clutter_label_new_with_text ( "Sans Bold 24", item.getName().c_str() );
			clutter_label_set_color ( CLUTTER_LABEL (label->actor), &normal_color );
			clutter_actor_show ( label->actor );
			label->scale_behave = clutter_behaviour_scale_new ( 
				app->alpha_sine_inc,
				1.0, 1.0,
				CLUTTER_GRAVITY_CENTER);
			clutter_behaviour_apply (label->scale_behave, label->actor);

			clutter_box_pack_defaults ( CLUTTER_BOX (app->vbox_left), label->actor );
			app->labels.push_back(label);
		}
	}
	catch( const string & e )
	{
		cout << "Problem loading media:"	<< e << endl;
		exit(1);
	}
}


void play_selection( App * app )
{
	MediaItem & media = dynamic_cast<LabelItem*>(*app->selected_item)->media;

	// Quote it incase there are white spaces in the file path
	//const string cmd = "xdg-open \"" + media.getFilePath() + "\"";
	const string cmd = "mplayer \"" + media.getFilePath() + "\"";
	if( system( cmd.c_str() ) == - 1 )
	{
		throw string("can't open file");
	}

	app->playing = true;
}


void stop_search( App * app )
{
	search_mode = false;
	search_string.clear();
	redraw_list( search_string, app ); // unfilter

	clutter_actor_hide ( app->filter );
	clutter_label_set_text( CLUTTER_LABEL(app->filter), "filter: " );
}

void begin_search( int letter, App * app )
{
	if( letter == CLUTTER_BackSpace )
	{
		if( !search_string.empty() )
		{
			search_string.erase( --search_string.end() );
		}
	}
	else
	{
		search_string += letter; 
	}

	redraw_list( search_string, app );

	string filter = "filter: " + search_string;
	clutter_label_set_text( CLUTTER_LABEL(app->filter), filter.c_str() );
}


/* input handler */
void input_cb (ClutterStage *stage, ClutterEvent *event, gpointer data)
{
	App *app = (App*)data;

	// HACK to throw away the character that caused the player to quit (i.e. 'q' )
	if( app->playing )
	{
		app->playing = false;
		return;
	}

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
	else if( event->type == CLUTTER_SCROLL )
	{

		// TODO handling mouse wheel scrolling
		// ClutterScrollDirection can be CLUTTER_SCROLL_UP CLUTTER_SCROLL_DOWN
		cout << "scroll is unhandled currently" << endl;
	}
	else if (event->type == CLUTTER_KEY_PRESS)
	{
		static gint fullscreen = TRUE;

		ClutterKeyEvent *kev = (ClutterKeyEvent *) event;

		// g_print ("*** key press event (key:%c) ***\n", clutter_key_event_symbol (kev));

		switch (clutter_key_event_symbol (kev))
		{
			case CLUTTER_Shift_L:
			case CLUTTER_Shift_R:

				break;

			case CLUTTER_Escape:

				if( search_mode )
				{
					stop_search(app);
				}
				else
				{
					clutter_main_quit ();
				}
				break;


			case CLUTTER_f:

				if( search_mode )
				{
					begin_search( clutter_key_event_symbol(kev), app );
				}
				else
				{
					// FIXME this doesn't work - problem with clutter?
					if( fullscreen )
					{
						cout << "setting windowed mode" << endl;
						clutter_stage_unfullscreen( CLUTTER_STAGE (app->stage) );
					}
					else
					{
						cout << "setting fullscreen mode" << endl;
						clutter_stage_fullscreen( CLUTTER_STAGE (app->stage) );
					}
					fullscreen = !fullscreen; 
				}
				break;

			case CLUTTER_Up:

				make_selection( app, -1 );

				break;
			
			case CLUTTER_Down:

				make_selection( app, 1 );

				break;

			case CLUTTER_Page_Up:

				make_selection( app, 5 );

				break;

			case CLUTTER_Page_Down:

				make_selection( app, 5 );

				break;

			case CLUTTER_Left:

				// TODO go up a directory
				
				break;

			case CLUTTER_Home:
				
				break;

			case CLUTTER_End:

				break;

			case CLUTTER_Return:

				//stop_search(app);
				play_selection( app );
				
				break;
		
			case CLUTTER_slash:

				if( search_mode )
				{
					begin_search( clutter_key_event_symbol(kev), app );
				}
				else
				{
					clutter_actor_show ( app->filter );
					search_mode = true;
				}
				
				break;

			case CLUTTER_BackSpace:

				if( search_mode )
				{
					begin_search( clutter_key_event_symbol(kev), app );
				}

				break;

			default:

				if( search_mode )
				{
					begin_search( clutter_key_event_symbol(kev), app );
				}
		}
	}
	else if (event->type == CLUTTER_KEY_RELEASE)
	{
		// ignore
	}
	else
	{
		cout << "other unhandled input type" << endl;
	}
}


void
on_timeline_new_frame (ClutterTimeline *timeline, gint frame_num, App *app)
{
	//cout << "new frame" << endl;
	// for each actor move 5 pixels ?
	

	/*
	if (frame_num == clutter_timeline_get_n_frames (timeline)/2)
	{
		// printf("half way\n");
	}
	*/
}


int main (int argc, char *argv[])
{
	App	*app;
	app = g_new0(App, 1);

	ClutterActor    *hbox;
	ClutterColor    bg_color = { 0, 0, 0, 255 };

	clutter_init (&argc, &argv);

	string catalog_path = "../share/catalog";

	if( argc > 1 )
	{
		catalog_path = argv[1];
	}

	app->playing = false;

	app->stage = clutter_stage_get_default ();
	clutter_stage_fullscreen( CLUTTER_STAGE (app->stage) );
	clutter_stage_set_color( CLUTTER_STAGE (app->stage), &bg_color );

	// Initialize the app properties
	app->timeline = clutter_timeline_new ( 1, 90);
	app->alpha_ramp = clutter_alpha_new_full (
			app->timeline, CLUTTER_ALPHA_SINE_HALF, NULL, NULL);


	app->effect_template = clutter_effect_template_new ( 
			app->timeline, CLUTTER_ALPHA_RAMP_INC);
	app->alpha_sine_inc = clutter_alpha_new_full (
			app->timeline, CLUTTER_ALPHA_SINE_INC, NULL, NULL);

	hbox = clutter_hbox_new ();
	clutter_actor_set_position( hbox, 0, 0 );
	clutter_box_set_default_padding ( CLUTTER_BOX (hbox), 10, 10, 10, 10 );
	clutter_actor_show (hbox);
	clutter_container_add_actor ( CLUTTER_CONTAINER (app->stage), hbox );



	// Setup the list of movie titles
	{
		app->vbox_left = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (app->vbox_left), 10, 10, 10, 50 );


		clutter_actor_show ( app->vbox_left );
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), app->vbox_left );
	}


	// Show the movie cover for the selected movie title
	{
		app->vbox_right = clutter_vbox_new ();
		clutter_box_set_default_padding ( CLUTTER_BOX (app->vbox_right), 10, 10, 10, 10 );

		// setup dummy cover
		app->cover  = clutter_texture_new();

		clutter_actor_set_position ( app->cover, 0, 0);
		clutter_actor_show ( app->cover );

		clutter_container_add_actor ( CLUTTER_CONTAINER (app->vbox_right), app->cover );


#ifdef CLUTTER_JUSTIFY_EXISTS 
		clutter_actor_show ( app->vbox_right );
		// FIXME we need a way to add to hbox but to justify far-right. The problem is that right now
		// the width of the left column of media labels is 0 so when we pack the cover in it doesn't get
		// pushed over any so we see the cover "undernearth" the labels. Maybe instead of putting it in an hbox
		// we could just put it on the stage on 10 pixels from the right?
		clutter_box_pack_defaults ( CLUTTER_BOX (hbox), app->vbox_right );
#else
		clutter_container_add_actor ( CLUTTER_CONTAINER (app->stage), app->vbox_right );
#endif
	}

	// Filter text prompt
	{
			app->filter = clutter_label_new_with_text ( "Sans Italic 18", "filter: " );
			clutter_label_set_color ( CLUTTER_LABEL (app->filter), &normal_color );

			clutter_label_set_line_wrap( CLUTTER_LABEL(app->filter), FALSE );

			clutter_actor_set_position( app->filter, 100, CLUTTER_STAGE_HEIGHT() - 100 ); 

			clutter_container_add_actor ( CLUTTER_CONTAINER (app->stage), app->filter );
	}

	loadMedia( app, catalog_path );

	make_selection( app, app->labels.begin() );

	g_signal_connect ( app->stage, "button-press-event", G_CALLBACK (input_cb), app);
	g_signal_connect ( app->stage, "key-press-event", G_CALLBACK (input_cb), app);
	g_signal_connect ( app->stage, "key-release-event", G_CALLBACK (input_cb), app);
	g_signal_connect ( app->stage, "scroll-event", G_CALLBACK (input_cb), app);

	g_signal_connect (app->timeline, "new-frame", G_CALLBACK(on_timeline_new_frame), app);

	//clutter_timeline_start (app->timeline);

	clutter_actor_show_all (app->stage);
	clutter_actor_hide ( app->filter );

	clutter_main();

	return 0;
}
