#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_ENTRY_
#define _RSSREADER_ENTRY_

#include <vector>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/level.hpp>
#include <boost/serialization/tracking_enum.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>

#include <boost/date_time/posix_time/ptime.hpp>

#include <xercesc/dom/DOMNode.hpp>

namespace RssReader
{
	class Entry sealed
	{
		friend class ::boost::serialization::access;

	public:
		::std::string UniqueId;
		::std::string Title;
		::std::string Url;
		::std::string Author;
		::std::string Body;
		::boost::posix_time::ptime Published;
		bool IsLive;
		bool Read;

	public:
		Entry();

	public:
		static void AddEntryToList(const ::xercesc_2_7::DOMNode* const entryNode, ::std::vector<Entry>& entries);

	private:
		template<typename Archive_t>
		void serialize(Archive_t& archive, const unsigned int version);
	};
}

BOOST_CLASS_VERSION(::RssReader::Entry, 0)
BOOST_CLASS_IMPLEMENTATION(::RssReader::Entry, ::boost::serialization::object_serializable)
BOOST_CLASS_TRACKING(::RssReader::Entry, ::boost::serialization::track_never)

#endif
