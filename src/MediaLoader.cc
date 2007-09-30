/* 
 List the files in a directory using standard C system api.
 Jan 2007
 Leif
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include <libgen.h> // for basename

#include <vector>
#include <iostream>

#include "MediaLoader.h"
#include "MediaItem.h"

#include <algorithm>

#include <ctype.h>


using namespace std;

namespace 
{
	struct lowercase : public unary_function<char, void> 
	{
		void operator() (char & c) 
		{
			c = tolower(c);
		}
	};

	bool string_cmp( string a, string b )
	{
		for_each( a.begin(), a.begin() + a.length(), lowercase() ); 	
		for_each( b.begin(), b.begin() + b.length(), lowercase() ); 	

		return a < b;
	}

	bool isdirectory( const string & path) 
	{
		struct stat statbuf;

		if( stat( path.c_str(), &statbuf) == -1)
		{
			return false;
		}
		else
		{
			return S_ISDIR(statbuf.st_mode);
		}
	}
}


MediaLoader::MediaLoader( const string & dir_path, bool recursive )
{
	DIR * dip = opendir( dir_path.c_str() );
	if( dip == NULL )
	{
		//const char * e = strerror[errno];
		throw string( "error opening directory" );
	}

	vector<string> files;
	struct dirent *	dit;

	// TODO allow for recursive search
	for( int i=0; (dit = readdir(dip) ) != NULL; ++i )
	{
		// check if it's a file or directory
		const string name = dit->d_name;

		if( !isdirectory(name) )
		{
			// TODO check the mime type to make sure it's a video file
			files.push_back( dir_path + "/" + name );	
		}
	}

	if( closedir( dip ) == -1 )
	{
		//const string err = strerror[errno];
		throw string("error closing directory");
	}

	sort( files.begin(), files.end(), string_cmp );

	vector<string>::const_iterator it = files.begin();
	for( ; it < files.end(); ++it )
	{
		const string file_path = *it;
		
		char * path_copy = strdup( file_path.c_str() );
		char * name = basename( path_copy );

		m_mediaItems.push_back( MediaItem( name, file_path, file_path ) );

		free(path_copy);
	}
	
}

vector<MediaItem> MediaLoader::getMediaItems() const
{
	return m_mediaItems; // return a copy, meh who cares for now
}




#ifdef TEST_LIST_FILES

int main( int argc, char ** argv)
{
	if( argc < 2 )
	{
		cout << "USAGE: " << argv[0] << " directory" << endl;
		return -1;
	}

	vector<MediaItem> files = MediaLoader( argv[1] ).getMediaItems();

	vector<MediaItem>::const_iterator it = files.begin();
	for( ; it < files.end(); ++it )
	{
		const MediaItem item = *it;
		cout << item.getFilePath() << endl;
	}

	return 0;
}

#endif
