#include "StdAfx.hpp"
#include "Feed.hpp"
#include "Utils.hpp"
#include "FeedValidator.hpp"
#include "Curl.hpp"

#include <boost/date_time/special_defs.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

namespace RssReader
{
	Feed::Feed(const std::string& url) :
		_etag(),
		_title(),
		_url(url),
		_entries(),
		_lastChecked(boost::date_time::min_date_time),
		Format(Unknown),
		Valid(false)
	{
		_entries.reserve(20);
	}

	const std::string& Feed::GetTitle() const
	{
		return _title;
	}

	const std::string& Feed::GetUrl() const
	{
		return _url;
	}

	const std::vector<Entry>& Feed::GetEntries() const
	{
		return _entries;
	}

	const boost::posix_time::ptime& Feed::GetLastChecked() const
	{
		return _lastChecked;
	}

	void Feed::CheckFeed()
	{
		_lastChecked = boost::posix_time::microsec_clock::universal_time();
		cURL::EasyInterface easy(_url, _etag);
		switch (easy.PerformRequest())
		{
		case 200:
			{
				_etag = easy.GetResponseEtag();
				FeedValidator validator(easy.GetResponseData(), *this);
				const std::string feedData = validator.Validate();
				if (Valid)
					CreateEntries(feedData);
			} return;
		case 304:
			Valid = true;
			return;
		default:
			_entries.clear();
			Valid = false;
		}
	}

	void Feed::CreateEntries(const std::string& feedData)
	{
		const xercesc_2_7::MemBufInputSource input(reinterpret_cast<const XMLByte*>(feedData.c_str()), feedData.size(), "");
		xercesc_2_7::XercesDOMParser parser;
		parser.setValidationScheme(xercesc_2_7::XercesDOMParser::Val_Never);
		parser.parse(input);

		const xercesc_2_7::DOMNodeList* const children = parser.getDocument()->getChildNodes()->item(0)->getChildNodes();
		for (XMLSize_t i = 0, listLength = children->getLength(); i < listLength; ++i)
		{
			const xercesc_2_7::DOMNode* const node = children->item(i);
			const std::string nodeName = XmlCharsToStdString(node->getNodeName());

			if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "entry"))
			{
				Entry::AddEntryToList(node, _entries);
				continue;
			}

			const std::string textContent = XmlCharsToStdString(node->getTextContent());
			if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "title"))
				_title = textContent;
		}
	}

	template<typename Archive_t>
	void Feed::serialize(Archive_t& archive, const unsigned int version)
	{
		archive & _url;
		archive & _title;
		archive & _etag;
		archive & _entries;
		archive & _lastChecked;
		archive & Format;
		archive & Valid;
	}
	template void Feed::serialize<InputArchive_t>(InputArchive_t&, const unsigned int);
	template void Feed::serialize<OutputArchive_t>(OutputArchive_t&, const unsigned int);
}
