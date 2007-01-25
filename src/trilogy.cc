/*
 * Trilogy
 *
 * @author Leif Gruenwoldt
 */

#include <stdio.h>
#include <stdlib.h>

#include <SDL_opengl.h>
#include "SDL.h"
#include <SDL/SDL_image.h>

#include <math.h>
#include <string.h> // for memcpy

#include <string>
#include <vector>
#include <iostream>

#include "list_files.h"

using namespace std;

/* screen width, height, and bit depth */
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     0
#define FULL_SCREEN_WIDTH 1680
#define FULL_SCREEN_HEIGHT 1050


/* Set up some booleans */
#define TRUE  1
#define FALSE 0

#define SQR(A)                (A * A)
#define NORMALIZE3(A)          {double l=1.0/sqrt( SQR(A.x) + SQR(A.y) + SQR(A.z) ); A.x*=l; A.y*=l; A.z*=l;}
#define SET_VECTOR3(V,X,Y,Z)  {V.x=X; V.y=Y; V.z=Z;}
#define SET_VERTEX3(V,X,Y,Z)  (SET_VECTOR3(V,X,Y,Z))

#define NORMALIZE2(A)          {double l=1.0/sqrt( SQR(A.x) + SQR(A.y) ); A.x*=l; A.y*=l;}
#define SET_VECTOR2(V,X,Y)  {V.x=X; V.y=Y;}
#define SET_VERTEX2(V,X,Y)  (SET_VECTOR2(V,X,Y))

/* This is our SDL surface */
SDL_Surface *surface;

GLfloat xrot; /* X Rotation ( NEW ) */
GLfloat yrot; /* Y Rotation ( NEW ) */
GLfloat zrot; /* Z Rotation ( NEW ) */

GLuint texture[1]; /* Storage For One Texture ( NEW ) */

int g_img_width = 0;
int g_img_height = 0;

vector<string> g_files; // name of all possible movie covers to cycle through
string g_texture; // name of current texture to be drawn

// TODO there should be a struct like this defined somewhere in SDL, SDL_Color is close but contains alpha too
typedef struct
{
	Uint8 r;
	Uint8 g;
	Uint8 b;
} MyRGB;

typedef struct
{
	double x;
	double y;
	double z;
} vertex3_t, vector3_t;

typedef struct
{
	double x;
	double y;
} vertex2_t;

/* Flags to pass to SDL_SetVideoMode */
int videoFlags;


/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
	/* clean up the window */
	SDL_Quit( );

	/* and exit appropriately */
	exit( returnCode );
}

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 * http://docs.mandragor.org/files/Common_libs_documentation/SDL/SDL_Documentation_project_en/guidevideo.html#AEN90
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
		case 1:
			return *p;

		case 2:
			return *(Uint16 *)p;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;

		case 4:
			return *(Uint32 *)p;

		default:
			return 0;       /* shouldn't happen, but avoids warnings */
	}
}


/* function to load in bitmap as a GL texture */
int LoadGLTextures( const string & image_name )
{
	/* Status indicator */
	int Status = FALSE;

	/* Create storage space for the texture */
	SDL_Surface *TextureImage[1]; 

	/* Load The image, Check For Errors, If image's Not Found Quit */
	if ( ( TextureImage[0] = IMG_Load( image_name.c_str() ) ) )
	{
		g_img_width = TextureImage[0]->w;
		g_img_height = TextureImage[0]->h;

		if( g_img_width >= 1024 || g_img_height >= 1024 )
		{
			cout << "image is too big" << endl;
			return FALSE;
		}

		Uint32 buffer[1024][1024];

		// HACK - pad image with black to fit in 1024x1024 texture
		{
			// set to black
			memset( buffer, 0, 1024*1024*sizeof(Uint32) );

			for( int x=0; x<TextureImage[0]->w; ++x)
			{
				for( int y=0; y<TextureImage[0]->h; ++y )
				{
					buffer[y][x] = getpixel( TextureImage[0], x, y );  // not sure why but we reverse x and y here
				}
			}
		}

		/* Set the status to true */
		Status = TRUE;

		/* Create The Texture */
		glGenTextures( 1, &texture[0] );

		/* Typical Texture Generation Using Data From The Bitmap */
		glBindTexture( GL_TEXTURE_2D, texture[0] );

		/* Generate The Texture */

		GLint level_of_detail;
		level_of_detail = 0; /* highest quality	*/

		//glTexImage2D( GL_TEXTURE_2D, level_of_detail, GL_RGB, TextureImage[0]->w, TextureImage[0]->h, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
		glTexImage2D( GL_TEXTURE_2D, level_of_detail, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );

		/* Linear Filtering */
		// change GL_LINEAR to GL_NEAREST to make faster, but looks kinda ugly when image is stretch in fullscreen
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		fprintf( stderr, "Failed to load image %s\n", image_name.c_str() );
	}

	/* Free up any memory we may have used */
	if ( TextureImage[0] )
		SDL_FreeSurface( TextureImage[0] );

	return Status;
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
	/* Height / width ration */
	GLfloat ratio;

	/* Protect against a divide by zero */
	if ( height == 0 )
		height = 1;

	ratio = ( GLfloat )width / ( GLfloat )height;

	/* Setup our viewport. */
	glViewport( 0, 0, ( GLint )width, ( GLint )height );

	/*
	 * change to the projection matrix and set
	 * our viewing volume.
	 */
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );

	/* Set our perspective */
	gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

	/* Make sure we're chaning the model view and not the projection */
	glMatrixMode( GL_MODELVIEW );

	/* Reset The View */
	glLoadIdentity( );

	return( TRUE );
}

void draw_next_image( bool forward = true )
{
	static vector<string>::const_iterator it = --g_files.begin(); // HAC go back one because on first seek we go fwd one, this fixes the annoying next/prev algorithm below

	while(1)
	{
		if( forward )
		{
			if( it == g_files.end() || ++it == g_files.end() )
			{
				cout << "no more images" << endl;
				return;
			}

			g_texture = *it;
		}
		else // backwards
		{
			if( it == g_files.begin() )
			{
				cout << "no more images" << endl;
				return;
			}

			if( it == g_files.end() )
			{
				--it;
			}

			--it;
			g_texture = *it;;
		}

		cout << "drawing " << g_texture << endl;

		if ( !LoadGLTextures( g_texture ) )
		{
			cout << "problem loading texture " << g_texture << endl;
		}
		else
		{
			break;
		}
	}

}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
	switch ( keysym->sym )
	{
		case SDLK_ESCAPE:
			/* ESC key was pressed */
			Quit( 0 );
			break;
		case SDLK_F1:
			/* F1 key was pressed
			 * this toggles fullscreen mode
			 */

			static int fullscreen_on = false;
			int width, height;

			if( !fullscreen_on )
			{
				fprintf( stdout, "going fullscreen\n");

				width = FULL_SCREEN_WIDTH;
				height = FULL_SCREEN_HEIGHT;

				fullscreen_on = 1;

				if ( !SDL_SetVideoMode( width, height, SCREEN_BPP, videoFlags ) ) // 1
				{
					fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
					Quit( 1 );
				}

				resizeWindow( width, height ); // 2

				SDL_WM_ToggleFullScreen( surface ); // 3
			}
			else
			{
				fprintf( stdout, "leaving fullscreen\n");

				width = SCREEN_WIDTH;
				height = SCREEN_HEIGHT;

				fullscreen_on = 0;

				// do in opposite order of going full screen
				SDL_WM_ToggleFullScreen( surface ); // 1

				resizeWindow( width, height ); // 2

				if ( !SDL_SetVideoMode( width, height, SCREEN_BPP, videoFlags ) ) // 3
				{
					fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
					Quit( 1 );
				}
			}

			break;
		case SDLK_RIGHT:

			draw_next_image();

			break;
		case SDLK_LEFT:

			draw_next_image(false);

			break;
		default:
			break;
	}

	return;
}

/* general OpenGL initialization function */
int initGL( GLvoid )
{
	printf ("OpenGL version: %s\n", glGetString (GL_VERSION));
	printf ("OpenGL vendor: %s\n", glGetString (GL_VENDOR));
	printf ("OpenGL renderer: %s\n", glGetString (GL_RENDERER));

	/* Load in the texture */
	if ( !LoadGLTextures( g_texture ) )
		return FALSE;

	/* Enable Texture Mapping ( NEW ) */
	glEnable( GL_TEXTURE_2D );

	/* Enable smooth shading */
	glShadeModel( GL_FLAT );

	/* Set the background black */
	glClearColor( 0.0f, 0.0f, 0.0f, 0.5f );

	/* Depth buffer setup */
	glClearDepth( 1.0f );

	/* Enables Depth Testing */
	glEnable( GL_DEPTH_TEST );

	/* The Type Of Depth Test To Do */
	glDepthFunc( GL_LEQUAL );

	/* Really Nice Perspective Calculations */
	//glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ); // slower and little benefit for flat texture
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );

	return( TRUE );
}

/* Here goes our drawing code */
int drawGLScene( GLvoid )
{
	//fprintf( stdout, "starting drawGLScene\n" );

	/* These are to calculate our fps */
	static GLint T0     = 0;
	static GLint Frames = 0;

	/* Clear The Screen And The Depth Buffer */
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	/* Move Into The Screen */
	glLoadIdentity( );
	glTranslatef( 0.0f, 0.0f, -2.5f );

#ifdef SPIN
	//	glRotatef( xrot, 1.0f, 0.0f, 0.0f); /* Rotate On The X Axis */
	//	glRotatef( yrot, 0.0f, 1.0f, 0.0f); /* Rotate On The Y Axis */
	glRotatef( zrot, 0.0f, 0.0f, 1.0f); /* Rotate On The Z Axis */
#endif

	/* Select Our Texture */
	glBindTexture( GL_TEXTURE_2D, texture[0] );

	const int half_width = g_img_width /2;
	const int half_height = g_img_height / 2;

	/* NOTE:
	 *   The x coordinates of the glTexCoord2f function need to inverted
	 * for SDL because of the way SDL_LoadBmp loads the data. So where
	 * in the tutorial it has glTexCoord2f( 1.0f, 0.0f ); it should
	 * now read glTexCoord2f( 0.0f, 0.0f );
	 */
	glBegin(GL_QUADS);
	/* Front Face */
	/* Bottom Left Of The Texture and Quad */
	{
		glTexCoord2f( 0.0f, (double)g_img_height/1024 ); 

		{
			vertex3_t bot_left;
			SET_VERTEX3( bot_left, -1.0f * half_width, -1.0f * half_height,1.0f );
			NORMALIZE3( bot_left );
			glVertex3f( bot_left.x, bot_left.y, bot_left.z );
		}
	}

	/* Bottom Right Of The Texture and Quad */
	{
		glTexCoord2f( (double)g_img_width/1024, (double)g_img_height/1024 ); 

		{
			vertex3_t bot_right;
			SET_VERTEX3( bot_right, 1.0f * half_width, -1.0f * half_height, 1.0f );
			NORMALIZE3( bot_right );
			glVertex3f( bot_right.x, bot_right.y, bot_right.z );
		}
	}

	/* Top Right Of The Texture and Quad */
	{
		glTexCoord2f( (double)g_img_width/1024, 0.0f ); 

		{
			vertex3_t top_right;
			SET_VERTEX3( top_right, 1.0f * half_width,  1.0f * half_height, 1.0f );
			NORMALIZE3( top_right );
			glVertex3f( top_right.x, top_right.y, top_right.z ); 
		}
	}

	/* Top Left Of The Texture and Quad */
	{

		glTexCoord2f( 0.0f, 0.0f ); // this coord does not have to be offset b/c it's 0

		vertex3_t top_left;
		SET_VERTEX3( top_left, -1.0f * half_width,  1.0f * half_height, 1.0f );
		NORMALIZE3( top_left );
		glVertex3f( top_left.x, top_left.y, top_left.z );
	}

#ifdef DRAW_CUBE
	/* Back Face */
	/* Bottom Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
	/* Top Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
	/* Top Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
	/* Bottom Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );

	/* Top Face */
	/* Top Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
	/* Bottom Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
	/* Bottom Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
	/* Top Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );

	/* Bottom Face */
	/* Top Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
	/* Top Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
	/* Bottom Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
	/* Bottom Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

	/* Right face */
	/* Bottom Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );
	/* Top Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );
	/* Top Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );
	/* Bottom Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );

	/* Left Face */
	/* Bottom Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
	/* Bottom Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );
	/* Top Right Of The Texture and Quad */
	glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
	/* Top Left Of The Texture and Quad */
	glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
#endif

	glEnd( );

	/* Draw it to the screen */
	SDL_GL_SwapBuffers( );

	/* Gather our frames per second */
	Frames++;
	{
		GLint t = SDL_GetTicks();
		if (t - T0 >= 5000) {
			GLfloat seconds = (t - T0) / 1000.0;
			GLfloat fps = Frames / seconds;
			printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
			T0 = t;
			Frames = 0;
		}
	}

#ifdef SPIN
	xrot += 0.3f; /* X Axis Rotation */
	yrot += 0.2f; /* Y Axis Rotation */
	zrot += 0.4f; /* Z Axis Rotation */
#endif

	//	fprintf( stdout, "done drawGLScene\n" );

	return( TRUE );
}


void init( const string & path )
{

	// c'mon, do it!
	try
	{
		g_files = get_files( path );
		draw_next_image();
	}
	catch( const string & err )
	{
		cout << err << endl;
	}
}

int main( int argc, char **argv )
{
	if( argc < 2 )
	{
		cout << "USAGE: " << argv[0] << " directory" << endl;
		return -1;
	}

	/* main loop variable */
	int done = FALSE;
	/* used to collect events */
	SDL_Event event;
	/* this holds some info about our display */
	const SDL_VideoInfo *videoInfo;
	/* whether or not the window is active */
	int isActive = TRUE;

	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		fprintf( stderr, "Video initialization failed: %s\n", SDL_GetError( ) );
		Quit( 1 );
	}

	char video_driver[32];

	SDL_VideoDriverName( video_driver, sizeof(video_driver) );
	fprintf( stdout, "video driver: %s\n", video_driver );

	/* Fetch the video info */
	videoInfo = SDL_GetVideoInfo( );

	if ( !videoInfo )
	{
		fprintf( stderr, "Video query failed: %s\n", SDL_GetError( ) );
		Quit( 1 );
	}

	/* the flags to pass to SDL_SetVideoMode */
	videoFlags  = SDL_OPENGL;          /* Enable OpenGL in SDL */
	videoFlags |= SDL_GL_DOUBLEBUFFER; /* Enable double buffering */
	videoFlags |= SDL_HWPALETTE;       /* Store the palette in hardware */
	videoFlags |= SDL_RESIZABLE;       /* Enable window resizing */
	videoFlags |= SDL_ANYFORMAT;

	/* This checks to see if surfaces can be stored in memory */
	if ( videoInfo->hw_available )
	{
		fprintf( stdout, "2d hardware accelerated\n" );
		videoFlags |= SDL_HWSURFACE;
	}
	else
	{
		fprintf( stdout, "no 2d hardware acceleration found\n" );
		videoFlags |= SDL_SWSURFACE;
	}

	/* This checks if hardware blits can be done */
	if ( videoInfo->blit_hw )
	{
		fprintf( stdout, "using hardware blits\n" );
		videoFlags |= SDL_HWACCEL;
	}

	/* Sets up OpenGL double buffering */
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	/* get a SDL surface */
	surface = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, videoFlags );

	/* Verify there is a surface */
	if ( !surface )
	{
		fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
		Quit( 1 );
	}

	init( argv[1] );

	/* initialize OpenGL */
	initGL( );

	/* resize the initial window */
	resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );

	/* wait for events */
	while ( !done )
	{
		/* handle the events in the queue */

		while ( SDL_PollEvent( &event ) )
		{
			switch( event.type )
			{
				case SDL_ACTIVEEVENT:
					/* Something's happend with our focus
					 * If we lost focus or we are iconified, we
					 * shouldn't draw the screen
					 */
					if ( event.active.gain == 0 )
						isActive = FALSE;
					else
						isActive = TRUE;
					break;			    
				case SDL_VIDEORESIZE:
					/* handle resize event */
					surface = SDL_SetVideoMode( event.resize.w, event.resize.h, 16, videoFlags );
					if ( !surface )
					{
						fprintf( stderr, "Could not get a surface after resize: %s\n", SDL_GetError( ) );
						Quit( 1 );
					}
					resizeWindow( event.resize.w, event.resize.h );
					break;
				case SDL_KEYDOWN:
					/* handle key presses */
					handleKeyPress( &event.key.keysym );
					break;
				case SDL_QUIT:
					/* handle quit requests */
					done = TRUE;
					break;
				default:
					break;
			}
		}

		/* draw the scene */
		//if ( isActive )
		drawGLScene( );
	}

	/* clean ourselves up and exit */
	Quit( 0 );

	/* Should never get here */
	return( 0 );
}
