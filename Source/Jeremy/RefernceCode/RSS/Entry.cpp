#include "StdAfx.hpp"
#include "Entry.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <boost/date_time/special_defs.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>

namespace RssReader
{
	Entry::Entry() :
		UniqueId(),
		Title(),
		Url(),
		Author(),
		Body(),
		Published(boost::date_time::min_date_time),
		IsLive(true),
		Read(false)
	{
	}

	void Entry::AddEntryToList(const xercesc_2_7::DOMNode* const entryNode, std::vector<Entry>& entries)
	{
		Entry entry;

		const xercesc_2_7::DOMNodeList* const children = entryNode->getChildNodes();
		for (XMLSize_t i = 0, listLength = children->getLength(); i < listLength; ++i)
		{
			const xercesc_2_7::DOMNode* const node = children->item(i);
			const std::string nodeName = XmlCharsToStdString(node->getNodeName());
			const std::string textContent = XmlCharsToStdString(node->getTextContent());

			if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "entryUniqueId"))
				entry.UniqueId = textContent;
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "url"))
				entry.Url = textContent;
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "title"))
				entry.Title = textContent;
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "content"))
				entry.Body = textContent;
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "authorName"))
			{
				if (entry.Author.empty())
					entry.Author = textContent;
			}
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "authorEmail"))
			{
				if (entry.Author.empty())
					entry.Author = textContent;
			}
			else if (xercesc_2_7::XMLString::equals(nodeName.c_str(), "publishDateTime"))
			{
				//entry.Published = boost::posix_time::time_from_string(textContent);
				entry.Published = boost::posix_time::microsec_clock::universal_time();
			}
		}

		if (entry.UniqueId.empty())
			entry.UniqueId = entry.Url;

		const std::vector<Entry>::iterator findResult = std::find_if(entries.begin(), entries.end(), EntryUniqueIdPredicate(entry.UniqueId));
		if (findResult == entries.end())
			entries.push_back(entry);
		else
			*findResult = entry;
	}

	template<typename Archive_t>
	void Entry::serialize(Archive_t& archive, const unsigned int version)
	{
		archive & UniqueId;
		archive & Title;
		archive & Url;
		archive & Author;
		archive & Body;
		archive & Published;
		archive & IsLive;
		archive & Read;
	}
	template void Entry::serialize<InputArchive_t>(InputArchive_t&, const unsigned int);
	template void Entry::serialize<OutputArchive_t>(OutputArchive_t&, const unsigned int);
}
