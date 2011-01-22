#include "StdAfx.hpp"
#include "RssReaderFrame.hpp"
#include "Utils.hpp"
#include "TreeItemData.hpp"
#include "Entry.hpp"

#include "folder.xpm"
//#include "folder_new.xpm"
#include "text_code.xpm"

#include <wx/utils.h>
#include <wx/imaglist.h>
#include <wx/xrc/xmlres.h>

namespace RssReader
{
	RssReaderFrame::RssReaderFrame() :
		_settingsFilePath(GetSettingsPath()),
		_userSettings(UserSettings::Deserialize(_settingsFilePath)),
		_contextMenu(),
		_selectedFolderNodeId(0),
		_selectedFeedFolder(nullptr),
		_selectedFeed(nullptr)
	{
		wxXmlResource::Get()->LoadFrame(this, nullptr, wxT("ID_RSSREADER"));
		SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
		SetClientSize(900, 600);

		_feedUrlTextBox = XRCCTRL(*this, "ID_TEXTCTRL1", wxTextCtrl);
		_treeView = XRCCTRL(*this, "ID_TREECTRL1", wxTreeCtrl);
		_listView = XRCCTRL(*this, "ID_LISTCTRL1", wxListCtrl);
		_htmlWindow = XRCCTRL(*this, "ID_HTMLWINDOW1", wxHtmlWindow);
		_statusBar = XRCCTRL(*this, "ID_STATUSBAR1", wxStatusBar);

		_contextMenu.Append(0, wxT("Mark all Entries Read"));
		_contextMenu.Append(1, wxT("Mark all Entries Unread"));
		_contextMenu.Append(2, wxT("Check Now"));

		{
			wxListItem titleColumn;
			titleColumn.SetText(wxT("Title"));
			titleColumn.SetWidth(400);
			_listView->InsertColumn(0, titleColumn);
		}

		{
			wxListItem authorColumn;
			authorColumn.SetText(wxT("Author"));
			authorColumn.SetWidth(100);
			_listView->InsertColumn(1, authorColumn);
		}

		{
			wxListItem publishedColumn;
			publishedColumn.SetText(wxT("Published"));
			publishedColumn.SetWidth(150);
			_listView->InsertColumn(2, publishedColumn);
		}

		InitTreeView();
	}

	void RssReaderFrame::InitTreeView()
	{
		wxImageList* treeImages = new wxImageList(16, 16, true);
		treeImages->Add(wxIcon(folder_xpm));
		treeImages->Add(wxIcon(text_code_xpm));
		_treeView->AssignImageList(treeImages);

		const wxTreeItemId rootNodeId = _treeView->AddRoot(wxString(_userSettings[0].Name.c_str(), wxConvUTF8), 0, 0, new FeedFolderNodeItemData(RssId_t(0)));
		CreateFolderStructure(_userSettings[0], rootNodeId);
		_treeView->SelectItem(rootNodeId);
		_treeView->ExpandAll();
		_selectedFeedFolder = &_userSettings[0];
		_selectedFolderNodeId = rootNodeId;
	}

	void RssReaderFrame::CreateFolderStructure(const FeedFolder& parentFolder, const wxTreeItemId parentNodeId)
	{
		typedef std::vector<const FeedFolder*> ffpv_t;

		const ffpv_t children = _userSettings.GetChildren(parentFolder.FolderId);
		for (ffpv_t::const_iterator i = children.begin(); i != children.end(); ++i)
		{
			const FeedFolder& child = **i;
			const wxTreeItemId childNodeId = _treeView->AppendItem(parentNodeId, wxString(child.Name.c_str(), wxConvUTF8), 0, 0, new FeedFolderNodeItemData(child.FolderId));
			CreateFolderStructure(child, childNodeId);
		}

		for (std::vector<Feed>::const_iterator i = parentFolder.Feeds.begin(); i != parentFolder.Feeds.end(); ++i)
			AddFeedNode(*i, parentNodeId, false);
	}

	void RssReaderFrame::AddFeedNode(const Feed& feed, wxTreeItemId parentNodeId, bool select)
	{
		const wxTreeItemId feedNodeId = _treeView->AppendItem(parentNodeId, wxString(!feed.GetTitle().empty() ? feed.GetTitle().c_str() : feed.GetUrl().c_str(), wxConvUTF8), 1, 1, new FeedNodeItemData(feed.GetUrl()));
		_treeView->Expand(parentNodeId);
		if (select)
			_treeView->SelectItem(feedNodeId);
	}

	void RssReaderFrame::PopulateEntriesListView(const Feed* const feed)
	{
		_listView->DeleteAllItems();
		_htmlWindow->SetPage(wxT(""));
		if (!feed)
			return;

		long index = -1;
		const std::vector<Entry>& entries = feed->GetEntries();
		for (std::vector<Entry>::const_iterator i = entries.begin(); i != entries.end(); ++i)
		{
			wxListItem item;
			item.SetData(const_cast<Entry*>(&(*i)));
			item.SetText(wxString(i->Title.c_str(), wxConvUTF8));
			item.SetId(++index);
			item.SetImage(0);
			if (!i->Read)
			{
				wxFont font(item.GetFont());
				font.SetWeight(wxFONTWEIGHT_BOLD);
				item.SetFont(font);
			}
			_listView->InsertItem(item);
			_listView->SetItem(index, 1, wxString(i->Author.c_str(), wxConvUTF8));
			_listView->SetItem(index, 2, wxString(boost::posix_time::to_simple_string(i->Published).c_str(), wxConvUTF8));
		}
	}

	void RssReaderFrame::SetStatusBarText()
	{
		if (_selectedFeed)
			_statusBar->SetStatusText(wxString::Format(wxT("Feed has %d entries"), _selectedFeed->GetEntries().size()));
		else
			_statusBar->SetStatusText(wxString::Format(wxT("Folder has %d feeds"), _selectedFeedFolder->Feeds.size()));
	}

	void RssReaderFrame::OnClose(wxCloseEvent& event)
	{
		_userSettings.Serialize(_settingsFilePath);
		wxWindow::Destroy();
	}

	void RssReaderFrame::OnAddFeedButtonClick(wxCommandEvent& event)
	{
		const wxString url = _feedUrlTextBox->GetValue();
		if (url.empty())
			return;

		_selectedFeedFolder->Feeds.push_back(Feed(std::string(url.mb_str())));
		AddFeedNode(_selectedFeedFolder->Feeds.back(), _selectedFolderNodeId, true);
		_feedUrlTextBox->SetValue(wxT(""));
	}

	void RssReaderFrame::OnCheckFeedButtonClick(wxCommandEvent& event)
	{
		if (!_selectedFeed)
			return;

		_selectedFeed->CheckFeed();
		SetStatusBarText();
		if (_selectedFeed->Valid)
		{
			PopulateEntriesListView(_selectedFeed);
			_treeView->SetItemText(_treeView->GetSelection(), wxString(_selectedFeed->GetTitle().c_str(), wxConvUTF8));
		}
	}

	void RssReaderFrame::OnAddFolderButtonClick(wxCommandEvent& event)
	{
		const FeedFolder& folder = _userSettings.AddFeedFolder(FeedFolder(*_selectedFeedFolder, "New Folder"));
		wxTreeItemId newNode = _treeView->AppendItem(_selectedFolderNodeId, wxString(folder.Name.c_str(), wxConvUTF8), 0, 0, new FeedFolderNodeItemData(folder.FolderId));
		_selectedFolderNodeId = newNode;
		_treeView->SelectItem(_selectedFolderNodeId);
		_selectedFeedFolder = &_userSettings[folder.FolderId];
		_treeView->EditLabel(newNode);
	}

	void RssReaderFrame::OnDeleteFolderButtonClick(wxCommandEvent& event)
	{
		if (_selectedFeed || !_selectedFeedFolder->FolderId || wxMessageBox(wxT("Are you sure you want to delete this folder and its children?"), wxT("Delete Confirm"), wxYES_NO) != wxYES)
			return;

		_userSettings.DeleteFeedFolder(_selectedFeedFolder->FolderId);
		_treeView->Delete(_selectedFolderNodeId);
	}

	void RssReaderFrame::OnDeleteFeedButtonClick(wxCommandEvent& event)
	{
		if (!_selectedFeed || wxMessageBox(wxT("Are you sure you want to delete the selected feed?"), wxT("Delete Confirm"), wxYES_NO) != wxYES)
			return;

		_selectedFeedFolder->Feeds.erase(std::find_if(_selectedFeedFolder->Feeds.begin(), _selectedFeedFolder->Feeds.end(), FeedUrlPredicate(_selectedFeed->GetUrl())));
		_treeView->Delete(_treeView->GetSelection());
	}

	void RssReaderFrame::OnTreeNodeSelected(wxTreeEvent& event)
	{
		const wxTreeItemId currentItemId = event.GetItem();
		switch (_treeView->GetItemImage(currentItemId))
		{
		case 0: // folder
			{
				const FeedFolderNodeItemData& feedFolderData = *reinterpret_cast<FeedFolderNodeItemData*>(_treeView->GetItemData(currentItemId));
				_selectedFolderNodeId = currentItemId;
				_selectedFeedFolder = &_userSettings[feedFolderData.FeedFolderId];
				_selectedFeed = nullptr;
			} break;
		case 1: // feed
			{
				_selectedFolderNodeId = _treeView->GetItemParent(currentItemId);
				const FeedFolderNodeItemData& parentFolderData = *reinterpret_cast<FeedFolderNodeItemData*>(_treeView->GetItemData(_selectedFolderNodeId));
				_selectedFeedFolder = &_userSettings[parentFolderData.FeedFolderId];
				const FeedNodeItemData& feedData = *reinterpret_cast<FeedNodeItemData*>(_treeView->GetItemData(currentItemId));
				const std::vector<Feed>::iterator iter = std::find_if(_selectedFeedFolder->Feeds.begin(), _selectedFeedFolder->Feeds.end(), FeedUrlPredicate(feedData.FeedUrl));
				_selectedFeed = &(*iter);
			} break;
		}
		SetStatusBarText();
		PopulateEntriesListView(_selectedFeed);
	}

	void RssReaderFrame::OnNodeEditBegin(wxTreeEvent& event)
	{
		if (!_selectedFeedFolder->FolderId)
			event.Veto();
	}

	void RssReaderFrame::OnNodeEditComplete(wxTreeEvent& event)
	{
		if (!event.GetLabel().empty())
			_selectedFeedFolder->Name = event.GetLabel().mb_str();
	}

	void RssReaderFrame::OnTreeViewRightClick(wxTreeEvent& event)
	{
		const wxTreeItemId nodeId = event.GetItem();

		//don't show popup menus for folders just yet
		if (!_treeView->GetItemImage(nodeId))
			return;

		const wxPoint loc = event.GetPoint();
		_treeView->PopupMenu(&_contextMenu, loc);
	}

	void RssReaderFrame::OnContextMenuItemSelected(wxCommandEvent& event)
	{
		const int id = event.GetId();
		switch (id)
		{
		case 0:
			//mark all read
			return;
		case 1:
			//mark all unread
			return;
		case 2:
			//check now
			return;
		}
	}

	void RssReaderFrame::OnListViewItemSelected(wxListEvent& event)
	{
		Entry& entry = *reinterpret_cast<Entry*>(event.GetData());
		const wxListItem& item = event.GetItem();

		if (!entry.Read)
		{
			wxFont font(item.GetFont());
			font.SetWeight(wxFONTWEIGHT_NORMAL);
			_listView->SetItemFont(item, font);
			entry.Read = true;
		}

		_htmlWindow->SetPage(wxString(entry.Body.c_str(), wxConvUTF8));
	}

	void RssReaderFrame::OnHtmlWindowLinkClicked(wxHtmlLinkEvent& event)
	{
		wxLaunchDefaultBrowser(event.GetLinkInfo().GetHref());
	}

	BEGIN_EVENT_TABLE(RssReaderFrame, wxFrame)
		EVT_CLOSE(RssReaderFrame::OnClose)
		EVT_MENU(0, RssReaderFrame::OnContextMenuItemSelected)
		EVT_MENU(1, RssReaderFrame::OnContextMenuItemSelected)
		EVT_MENU(2, RssReaderFrame::OnContextMenuItemSelected)
		EVT_TEXT_ENTER(XRCID("ID_TEXTCTRL1"), RssReaderFrame::OnAddFeedButtonClick)
		EVT_BUTTON(XRCID("ID_BUTTON1"), RssReaderFrame::OnAddFeedButtonClick)
		EVT_BUTTON(XRCID("ID_BUTTON2"), RssReaderFrame::OnCheckFeedButtonClick)
		EVT_BUTTON(XRCID("ID_BUTTON3"), RssReaderFrame::OnAddFolderButtonClick)
		EVT_BUTTON(XRCID("ID_BUTTON4"), RssReaderFrame::OnDeleteFolderButtonClick)
		EVT_BUTTON(XRCID("ID_BUTTON5"), RssReaderFrame::OnDeleteFeedButtonClick)
		EVT_TREE_SEL_CHANGED(XRCID("ID_TREECTRL1"), RssReaderFrame::OnTreeNodeSelected)
		EVT_TREE_BEGIN_LABEL_EDIT(XRCID("ID_TREECTRL1"), RssReaderFrame::OnNodeEditBegin)
		EVT_TREE_END_LABEL_EDIT(XRCID("ID_TREECTRL1"), RssReaderFrame::OnNodeEditComplete)
		EVT_TREE_ITEM_RIGHT_CLICK(XRCID("ID_TREECTRL1"), RssReaderFrame::OnTreeViewRightClick)
		EVT_LIST_ITEM_SELECTED(XRCID("ID_LISTCTRL1"), RssReaderFrame::OnListViewItemSelected)
		EVT_HTML_LINK_CLICKED(XRCID("ID_HTMLWINDOW1"), RssReaderFrame::OnHtmlWindowLinkClicked)
	END_EVENT_TABLE()
}
