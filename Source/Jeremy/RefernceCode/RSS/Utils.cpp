#include "StdAfx.hpp"
#include "Utils.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <xercesc/util/XMLString.hpp>

#ifdef _WIN32
#include <shlobj.h>
#endif

namespace RssReader
{
	RssId_t GenerateRssId()
	{
		using namespace ::boost::posix_time;
		using namespace ::boost::gregorian;

		return (microsec_clock::universal_time() - ptime(date(1970, 1, 1))).total_milliseconds();
	}

	std::string GetSettingsPath()
	{
#ifdef _WIN32
		std::vector<char> pathBuffer(MAX_PATH + 1, '\0');
		SHGetFolderPath(0, CSIDL_PERSONAL , 0, SHGFP_TYPE_CURRENT, &pathBuffer.front());

		std::string path(&pathBuffer.front());
		path += "/feeds.rrf";
		return path;
#else
		return std::string(".RssReaderFeeds");
#endif
	}

	std::string XmlCharsToStdString(const XMLCh* const xmlChars)
	{
		char* stdChars = xercesc_2_7::XMLString::transcode(xmlChars);
		const std::string result = std::string(stdChars);
		xercesc_2_7::XMLString::release(&stdChars);
		return result;
	}

	FeedUrlPredicate::FeedUrlPredicate(const std::string& url) :
		_url(url)
	{
	}

	bool FeedUrlPredicate::operator()(const Feed& feed) const
	{
		return boost::algorithm::iequals(_url, feed.GetUrl());
	}

	EntryUniqueIdPredicate::EntryUniqueIdPredicate(const std::string& uniqueId) :
		_uniqueId(uniqueId)
	{
	}

	bool EntryUniqueIdPredicate::operator()(const Entry& entry) const
	{
		return boost::algorithm::iequals(_uniqueId, entry.UniqueId);
	}
}
