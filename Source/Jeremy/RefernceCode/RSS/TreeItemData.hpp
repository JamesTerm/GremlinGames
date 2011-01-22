#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_TREEITEMDATA_
#define _RSSREADER_TREEITEMDATA_

#include <string>
#include <wx/treectrl.h>

#include "Utils.hpp"

namespace RssReader
{
	class FeedFolderNodeItemData sealed : public wxTreeItemData
	{
	public:
		const RssId_t FeedFolderId;

	public:
		FeedFolderNodeItemData(const RssId_t feedFolderId);
	};

	class FeedNodeItemData sealed : public wxTreeItemData
	{
	public:
		const ::std::string FeedUrl;

	public:
		FeedNodeItemData(const ::std::string& feedUrl);
	};
}

#endif
