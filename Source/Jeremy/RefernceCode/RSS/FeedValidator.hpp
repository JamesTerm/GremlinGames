#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_FEEDVALIDATOR_
#define _RSSREADER_FEEDVALIDATOR_

#include <string>
#include <sstream>

#include "Feed.hpp"

namespace RssReader
{
	class FeedValidator sealed
	{
		const ::std::string& _feedData;
		Feed& _feed;
		bool _validationValid;

	public:
		FeedValidator(const ::std::string& feedData, Feed& feed);

	public:
		::std::string Validate();

	private:
		::std::string ValidateFeedFormat(const ::std::string& xslPath, FeedFormat feedFormat);
		::std::string TransformFeed(::std::stringstream& xslStream);
		bool ValidateFeed(const ::std::string& feed) const;
	};
}

#endif
