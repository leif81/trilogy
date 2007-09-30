#include "MediaItem.h"

using namespace std;

MediaItem::MediaItem( const string & name, const string & cover_path, const string & file_path ) :
	m_name( name ),
	m_coverPath( cover_path ),
	m_filePath( file_path )
{
}
