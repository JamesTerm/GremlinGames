#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_FEEDFOLDER_
#define _RSSREADER_FEEDFOLDER_

#include <vector>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking_enum.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>

#include "Utils.hpp"
#include "Feed.hpp"

namespace RssReader
{
	class FeedFolder sealed
	{
		friend class ::boost::serialization::access;

	public:
		const RssId_t ParentId;
		const RssId_t FolderId;
		::std::string Name;
		::std::vector<Feed> Feeds;

	public:
		explicit FeedFolder(const ::std::string& name = ::std::string());
		FeedFolder(const FeedFolder& parent, const ::std::string& name);

	private:
		template<typename Archive_t>
		void serialize(Archive_t& archive, const unsigned int version);
	};
}

BOOST_CLASS_VERSION(::RssReader::FeedFolder, 0)
BOOST_CLASS_IMPLEMENTATION(::RssReader::FeedFolder, ::boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(::RssReader::FeedFolder, ::boost::serialization::track_never)

#endif
