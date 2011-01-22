#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_USERSETTINGS_
#define _RSSREADER_USERSETTINGS_

#include <vector>
#include <string>
#include <map>

#include <boost/serialization/access.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking_enum.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>

#include "Utils.hpp"
#include "FeedFolder.hpp"

namespace RssReader
{
	typedef ::std::map<RssId_t, FeedFolder> FeedFolderDictionary_t;

	class UserSettings sealed
	{
		friend class ::boost::serialization::access;

		FeedFolderDictionary_t _feedFolders;

	private:
		UserSettings();

	public:
		const FeedFolder& operator[](RssId_t id) const;
		FeedFolder& operator[](RssId_t id);

	public:
		static UserSettings Deserialize(const ::std::string& settingsFilePath);

	public:
		void Serialize(const ::std::string& settingsFilePath) const;
		const FeedFolder& AddFeedFolder(const FeedFolder& folder);
		void DeleteFeedFolder(RssId_t folderId);
		::std::vector<const FeedFolder*> GetChildren(RssId_t parentId) const;

	private:
		template<typename Archive_t>
		void serialize(Archive_t& archive, const unsigned int version);
	};
}

BOOST_CLASS_VERSION(::RssReader::UserSettings, 0)
BOOST_CLASS_IMPLEMENTATION(::RssReader::UserSettings, ::boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(::RssReader::UserSettings, ::boost::serialization::track_never)

#endif
