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

	private:

		std::vector<MediaItem> m_mediaItems;
};

#endif
