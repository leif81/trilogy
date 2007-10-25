#ifndef MEDIA_LOADER_H
#define MEDIA_LOADER_H

#include <vector>
#include <string>

class MediaItem;

class MediaLoader
{
	public:

		/**
		 * Import new media items found in this directory path
		 */
		MediaLoader( const std::string & dir_path, bool recursive = false );

		/** TODO
		 * Import media items as found in this config file
		 */
		// MediaLoader( const std::string & config_path );

		std::vector<MediaItem> getMediaItems() const;

		static bool isdirectory( const std::string & path);

	private:

		bool isVideo( const std::string & path );
		bool isImage( const std::string & path );

		bool hasExtension( const std::string & filename, const std::string & extension ) const;

		std::vector<MediaItem> m_mediaItems;
};

#endif
