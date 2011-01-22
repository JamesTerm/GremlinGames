#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_RSSREADERFRAME_
#define _RSSREADER_RSSREADERFRAME_

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/html/htmlwin.h>

#include "Feed.hpp"
#include "FeedFolder.hpp"
#include "UserSettings.hpp"

namespace RssReader
{
	class RssReaderFrame sealed : public wxFrame
	{
		const ::std::string _settingsFilePath;
		UserSettings _userSettings;
		wxMenu _contextMenu;
		wxTreeItemId _selectedFolderNodeId;
		FeedFolder* _selectedFeedFolder;
		Feed* _selectedFeed;
		wxTextCtrl* _feedUrlTextBox;
		wxTreeCtrl* _treeView;
		wxListCtrl* _listView;
		wxHtmlWindow* _htmlWindow;
		wxStatusBar* _statusBar;

	public:
		RssReaderFrame();

	private:
		void InitTreeView();
		void CreateFolderStructure(const FeedFolder& parentFolder, const wxTreeItemId parentNodeId);
		void AddFeedNode(const Feed& feed, wxTreeItemId parentNodeId, bool select);
		void PopulateEntriesListView(const Feed* const feed);
		void SetStatusBarText();

	private:
		void OnClose(wxCloseEvent& event);
		void OnAddFeedButtonClick(wxCommandEvent& event);
		void OnCheckFeedButtonClick(wxCommandEvent& event);
		void OnAddFolderButtonClick(wxCommandEvent& event);
		void OnDeleteFolderButtonClick(wxCommandEvent& event);
		void OnDeleteFeedButtonClick(wxCommandEvent& event);
		void OnTreeNodeSelected(wxTreeEvent& event);
		void OnNodeEditBegin(wxTreeEvent& event);
		void OnNodeEditComplete(wxTreeEvent& event);
		void OnTreeViewRightClick(wxTreeEvent& event);
		void OnContextMenuItemSelected(wxCommandEvent& event);
		void OnListViewItemSelected(wxListEvent& event);
		void OnHtmlWindowLinkClicked(wxHtmlLinkEvent& event);

		DECLARE_EVENT_TABLE()
	};
}

#endif
