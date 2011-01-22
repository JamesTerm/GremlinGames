#include "StdAfx.hpp"
#include "FeedFolder.hpp"

namespace RssReader
{
	FeedFolder::FeedFolder(const std::string& name) :
		ParentId(0),
		FolderId(0),
		Name(name),
		Feeds()
	{
	}

	FeedFolder::FeedFolder(const FeedFolder& parent, const std::string& name) :
		ParentId(parent.FolderId),
		FolderId(GenerateRssId()),
		Name(name),
		Feeds()
	{
	}

	template<typename Archive_t>
	void FeedFolder::serialize(Archive_t& archive, const unsigned int version)
	{
		archive & const_cast<RssId_t&>(ParentId);
		archive & const_cast<RssId_t&>(FolderId);
		archive & Name;
		archive & Feeds;
	}
	template void FeedFolder::serialize<InputArchive_t>(InputArchive_t&, const unsigned int);
	template void FeedFolder::serialize<OutputArchive_t>(OutputArchive_t&, const unsigned int);
}
