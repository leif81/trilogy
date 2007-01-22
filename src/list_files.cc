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

#include <vector>
#include <iostream>

#include "list_files.h"

#include <algorithm>

#include <ctype.h>


using namespace std;

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

		if( !isdirectory(name) )
		{
			files.push_back( dir_path + "/" + name );	
		}
	}

	if( closedir( dip ) == -1 )
	{
		//const string err = strerror[errno];
		throw string("error closing directory");
	}

	sort( files.begin(), files.end(), string_cmp );
	
	return files; // return a copy, meh who cares
}

#ifdef TEST_LIST_FILES

int main( int argc, char ** argv)
{
	if( argc < 2 )
	{
		cout << "USAGE: " << argv[0] << " directory" << endl;
		return -1;
	}

	vector<string> files = get_files( argv[1] );

	vector<string>::const_iterator it = files.begin();
	for( ; it < files.end(); ++it )
	{
		cout << *it << endl;
	}

	return 0;
}

#endif
