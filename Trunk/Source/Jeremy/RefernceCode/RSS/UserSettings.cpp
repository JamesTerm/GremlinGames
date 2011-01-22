#include "StdAfx.hpp"
#include "UserSettings.hpp"

#include <fstream>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace RssReader
{
	UserSettings::UserSettings() :
		_feedFolders()
	{
	}

	const FeedFolder& UserSettings::operator[](RssId_t id) const
	{
		return _feedFolders.find(id)->second;
	}

	FeedFolder& UserSettings::operator[](RssId_t id)
	{
		return _feedFolders.find(id)->second;
	}

	UserSettings UserSettings::Deserialize(const std::string& settingsFilePath)
	{
		std::ifstream settingsFile(settingsFilePath.c_str(), std::ios_base::in | std::ios_base::binary);
		UserSettings userSettings;

		if (!settingsFile.good())
		{
			userSettings._feedFolders.insert(FeedFolderDictionary_t::value_type(0, FeedFolder("All Feeds")));
			return userSettings;
		}

		InputArchive_t arch(settingsFile);
		arch >> userSettings;
		return userSettings;
	}

	void UserSettings::Serialize(const std::string& settingsFilePath) const
	{
		std::ofstream settingsFile(settingsFilePath.c_str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
		OutputArchive_t arch(settingsFile);
		arch << *this;
	}

	const FeedFolder& UserSettings::AddFeedFolder(const FeedFolder& folder)
	{
		return _feedFolders.insert(FeedFolderDictionary_t::value_type(folder.FolderId, folder)).first->second;
	}

	void UserSettings::DeleteFeedFolder(RssId_t folderId)
	{
		const std::vector<const FeedFolder*> children = GetChildren(folderId);
		for (std::vector<const FeedFolder*>::const_iterator iter = children.begin(); iter != children.end(); ++iter)
			DeleteFeedFolder((*iter)->FolderId);
		_feedFolders.erase(folderId);
	}

	std::vector<const FeedFolder*> UserSettings::GetChildren(RssId_t parentId) const
	{
		std::vector<const FeedFolder*> feedFolders;
		feedFolders.reserve(_feedFolders.size());

		for (FeedFolderDictionary_t::const_iterator iter = _feedFolders.begin(); iter != _feedFolders.end(); ++iter)
			if (iter->first != parentId && iter->second.ParentId == parentId)
				feedFolders.push_back(&iter->second);

		return feedFolders;
	}

	template<typename Archive_t>
	void UserSettings::serialize(Archive_t& archive, const unsigned int version)
	{
		archive & _feedFolders;
	}
	template void UserSettings::serialize<InputArchive_t>(InputArchive_t&, const unsigned int);
	template void UserSettings::serialize<OutputArchive_t>(OutputArchive_t&, const unsigned int);
}
