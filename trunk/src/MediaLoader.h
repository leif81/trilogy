#ifndef MEDIA_LOADER_H
#define MEDIA_LOADER_H

#include <vector>
#include <string>

class MediaItem;

// TODO rename MediaCatalog, MediaIndex?
class MediaLoader
{
	public:

		/**
		 * Import media items as found in this config file
		 */
		MediaLoader( const std::string & config = "~/.trilogy/catalog" );

		/**
		 * Import new media items found in this directory path
		 */
		void scan( const std::string & dir_path, bool recursive = false );

		std::vector<MediaItem> getMediaItems() const;

		static bool isdirectory( const std::string & path);

	private:

		bool isVideo( const std::string & path );
		bool isImage( const std::string & path );

		bool hasExtension( const std::string & filename, const std::string & extension ) const;

		std::vector<MediaItem> m_mediaItems;

		std::string m_config;
};

#endif
