#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_FEED_
#define _RSSREADER_FEED_

#include <vector>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking_enum.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>

#include <boost/date_time/posix_time/ptime.hpp>

#include "Entry.hpp"

namespace RssReader
{
	enum FeedFormat
	{
		Unknown = 0,
		Rss20 = 1,
		Atom10 = 2,
		RDF = 3,
		Atom03 = 4
	};

	class Feed sealed
	{
		friend class ::boost::serialization::access;

		::std::string _etag;
		::std::string _title;
		::std::string _url;
		::std::vector<Entry> _entries;
		::boost::posix_time::ptime _lastChecked;

	public:
		FeedFormat Format;
		bool Valid;

	public:
		explicit Feed(const ::std::string& url = ::std::string());

	public:
		const ::std::string& GetTitle() const;
		const ::std::string& GetUrl() const;
		const ::std::vector<Entry>& GetEntries() const;
		const ::boost::posix_time::ptime& GetLastChecked() const;

	public:
		void CheckFeed();

	private:
		void CreateEntries(const ::std::string& feedData);

		template<typename Archive_t>
		void serialize(Archive_t& archive, const unsigned int version);
	};
}

BOOST_CLASS_VERSION(::RssReader::Feed, 0)
BOOST_CLASS_IMPLEMENTATION(::RssReader::Feed, ::boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(::RssReader::Feed, ::boost::serialization::track_never)

#endif
