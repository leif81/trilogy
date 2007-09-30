#ifndef MEDIA_ITEM_H
#define MEDIA_ITEM_H

#include <string>

class MediaItem
{
	public:

		MediaItem( const std::string & name, const std::string & cover_path, const std::string & file_path  );

		void setName( const std::string & name ) { m_name = name; }
		void setFilePath( const std::string & file_path ) { m_filePath = file_path; }
		void setCoverPath( const std::string & cover_path ) { m_coverPath = cover_path; }

		std::string getName() const { return m_name; }
		std::string getFilePath() const { return m_filePath; }
		std::string getCoverPath() const { return m_coverPath; }

	private:

		std::string m_name;
		std::string m_filePath;	
		std::string m_coverPath;
};

#endif
