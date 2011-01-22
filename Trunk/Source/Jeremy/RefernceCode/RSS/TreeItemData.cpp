#include "StdAfx.hpp"
#include "TreeItemData.hpp"

namespace RssReader
{
	FeedFolderNodeItemData::FeedFolderNodeItemData(const RssId_t feedFolderId) :
		FeedFolderId(feedFolderId)
	{
	}

	FeedNodeItemData::FeedNodeItemData(const std::string& feedUrl) :
		FeedUrl(feedUrl)
	{
	}
}
