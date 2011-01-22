#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_UTILS_
#define _RSSREADER_UTILS_

#include <string>
#include <boost/cstdint.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "Feed.hpp"

namespace RssReader
{
	typedef ::boost::int64_t RssId_t;
	typedef ::boost::archive::text_iarchive InputArchive_t;
	typedef ::boost::archive::text_oarchive OutputArchive_t;

	RssId_t GenerateRssId();
	::std::string GetSettingsPath();

	::std::string XmlCharsToStdString(const XMLCh* const xmlChars);

	class FeedUrlPredicate sealed
	{
		const ::std::string& _url;

	public:
		FeedUrlPredicate(const ::std::string& url);

		bool operator()(const Feed& feed) const;
	};

	class EntryUniqueIdPredicate sealed
	{
		const ::std::string& _uniqueId;

	public:
		EntryUniqueIdPredicate(const ::std::string& uniqueId);

		bool operator()(const Entry& entry) const;
	};
}

#endif
