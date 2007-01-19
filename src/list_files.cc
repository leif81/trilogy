/* 
 List the files in a directory using standard C system api.
 Jan 2007
 Leif
*/

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

#include <vector>
#include <iostream>

#include "list_files.h"

using namespace std;


vector<string> get_files( const string & dir_path )
{

	DIR * dip = opendir( dir_path.c_str() );
	if( dip == NULL )
	{
		//const char * e = strerror[errno];
		throw string( "error opening directory" );
	}

	vector<string> files;
	struct dirent *	dit;

	for( int i=0; (dit = readdir(dip) ) != NULL; ++i )
	{
		// check if it's a file or directory
		const string name = dit->d_name;
		if( opendir( name.c_str() ) == NULL )
		{
			files.push_back( dir_path + "/" + name );	
		}
	}

	if( closedir( dip ) == -1 )
	{
		//const string err = strerror[errno];
		throw string("error closing directory");
	}
	
	return files; // return a copy, meh who cares
}


int test_main( int argc, char ** argv)
{
	if( argc < 2 )
	{
		cout << "USAGE: " << argv[0] << " directory" << endl;
		return -1;
	}

	return 0;
}
