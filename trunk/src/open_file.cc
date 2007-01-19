/**
 Open a file with the users preferred application

 Requires xdg-utils package
*/

#include <stdlib.h>
#include <iostream>

using namespace std;

void open_file( const string & file )
{
	const string cmd = "xdg-open " + file;
	if( system( cmd.c_str() ) == - 1 )
	{
		throw string("can't open file");
	}
}

int main( int argc, char ** argv )
{
	if( argc < 2 )
	{
		cout << "USAGE: " << argv[0] << " file" << endl;
		return -1;
	}

	try
	{
		open_file( argv[1] );
	}
	catch( const string & err )
	{
		cout << err << endl;
		return -1;
	}

	return 0;
}
