#include "stdafx.h"

PathLayout::PathLayout()
{
	m_dirLayout = NULL;
	m_amISetting = false;
	m_viewMode.m_parent = this;
	m_viewMode.m_parentWindow = this;
	m_viewMode.m_matchHeight = true;
	m_viewMode.m_matchWidth = false;
	m_viewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_NameOnly);
	m_viewMode.FileButton_ViewMode_SetSortString(FileButton_Naming_AltName);
	UtilLib_SimpleGrid_PutSpacingX(PathLayout_SpacingX);
}
////////////////////////////////////////////////////////////////////////////////////////////////
void PathLayout::SetNewPath(const char* p_newDirectory, DirectoryLayout* p_dirLayout)
{
	if (!p_newDirectory) return;
	// We don't want the FileButton's we are creating to set the DirLayout when selected
	m_amISetting = true;

	// Keep from laying out until we are done
	MultipleLayouts_Start();

	// Set the directory layout we are interested in
	if (p_dirLayout)
		m_dirLayout = p_dirLayout;

	// We need to add to the reference count
	FileProperties* prop = FileProperties::GetHandle(p_newDirectory);
	unsigned buttonIndex = 0;
	unsigned numButtons = m_pathButtons.NoItems();

	while (prop)
	{
		FileButton* thisButton = NULL;
		if (buttonIndex < numButtons)
			thisButton = m_pathButtons[buttonIndex++];
		else
		{
			// Create a new FileButton with this path at position 0;
			HWND hwnd = this->OpenChild("FileButton", -200, -200);
			thisButton = GetWindowInterface<FileButton>(hwnd);
			if (!thisButton) _throw ("PathLayout::SetNewPath() could not create a FileButton");
			// thisButton->AddDependant(p_dirLayout, (long)thisButton);
			m_pathButtons.Add(thisButton);
		}
		thisButton->SetFileButtonProperties(prop, false);
		prop = prop->GetParentHandle();
	}

	// Get rid of any extra FileButtons I may have grabbed
	for (unsigned i = buttonIndex; i < numButtons; i++)
	{
		NewTek_Delete(m_pathButtons[buttonIndex]);
		m_pathButtons.DeleteEntry(buttonIndex);
	}

	// We can do the layout now.
	MultipleLayouts_Finished();

	// My File Button Children can now set when selected
	m_amISetting = false;
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool PathLayout::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{
	// Do not allow if not a FileButton.
	FileButton* thisButton = GetWindowInterface<FileButton>(Dropped->hWnd);
	if (thisButton)
	{
		FileProperties* fp = thisButton->GetProperties();
		if (fp)
		{
			// Only return true for drives, My Computer and folders
			long type = fp->GetFileStatus();
			if (type >= FileProperties_Drive)
				return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////
bool PathLayout::DragNDrop_ShouldIRubberBandSelect(HWND hWnd)
{
	return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////
WindowLayout_Item* PathLayout::DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									)
{
	if (DroppedHere)
	{
		// Set the directory of the first element dropped
		if (m_dirLayout)
		{
			WindowLayout_Item *Scan=ObjectsDropped;
			FileButton *FB=GetWindowInterface<FileButton>(Scan->hWnd);
			if (FB)
			{
				static char newPath[MAX_PATH];
				strcpy(newPath,FB->GetFileName());
				DeferredMessage((long)newPath);
			}
		}

		// We scan through all the items and delete them ALL!
		WindowLayout_Item *Scan=ObjectsDropped;
		while(Scan)
		{	
			BaseWindowClass* bwc = GetWindowInterface<BaseWindowClass>(Scan->hWnd);
			NewTek_Delete<BaseWindowClass>(bwc);
			Scan->hWnd = NULL;
			Scan=Scan->Next;
		}
	}
	return UtilLib_SimpleGrid::DragNDrop_DropItemsHere
		(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);
}
/////////////////////////////////////////////////////////////////////////////////////////
WindowLayout_Item *PathLayout::Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height)
{	// Start at the top left of the dialog box
	long CurrentXPosn=GetWindowWidth()-1;

	// Look at all children
	WindowLayout_Item *Item=Children;
	WindowLayout_Item *lastItem = NULL;
	while(Item)
	{	
		// Reposition this item
		CurrentXPosn-=(Item->XSize+SpacingX);
		Item->XPosn=CurrentXPosn;
		Item->YPosn=0;

		// Look at the next child
		lastItem = Item;
		Item=Item->Next;
	}

	// Move all of the children back to the beginning if we are too far right
	if (lastItem)
	{
		long XOffset = lastItem->XPosn;
		if (XOffset > 0)
		{
			Item = Children;
			while(Item)
			{
				Item->XPosn -= XOffset;
				Item=Item->Next;
			}
		}
	}
	
	// There is never a change in order
	return Children;
}
//////////////////////////////////////////////////////////////////////////////////////////
DirectoryLayout* PathLayout::GetDirectoryLayout()
{
	return m_dirLayout;
}
//////////////////////////////////////////////////////////////////////////////////////////
void PathLayout::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{
	if (m_dirLayout)
	{
		const char* myPath = (const char*)ID1;
		if (myPath)
		{
			if (m_dirLayout->DeferredSetDirectory(myPath))
				return;
		}
		myPath = m_dirLayout->GetDirectory();
		SetNewPath(myPath, m_dirLayout);
	}
	UtilLib_SimpleGrid::ReceiveDeferredMessage(ID1,ID2);
}
//////////////////////////////////////////////////////////////////////////////////////////
void PathLayout::Child_AboutToLeave(HWND hWnd, HWND NewParent)
{
	FileButton* thisButton = GetWindowInterface<FileButton>(hWnd);
	if (thisButton && m_pathButtons.Exists(thisButton))
		DeferredMessage();
}
///////////////////////////////////////////////////////////////////////////////////////
void PathLayout::MouseRButtonRelease(long Flags,long x,long y)
{
	// Create a Context Menu
	tList<BaseWindowClass*>	Windows;
	Windows.Add(this);
	unsigned OldNoPopups=BWC_GetNoPopupWindows();
	NewTek_ExecuteContextMenu(&Windows);
	if (OldNoPopups==BWC_GetNoPopupWindows())
		SetKBFocus();
}
//////////////////////////////////////////////////////////////////////////////////////////

