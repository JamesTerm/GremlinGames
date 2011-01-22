class PathLayout;

#ifndef __PathLayoutH_
#define __PathLayoutH_

/*	PathLayout
Used in a FileBin to represent the associated DirectoryLayout path.  File Buttons in the PathLayout
represent each drive/directory along the path and have the DirectoryLayout as a listener.  
When any button is double-clicked, the DirectoryLayout is switched to represent that part of the path.
*/

#define PathLayout_SpacingX		0

class FileManagementDLL PathLayout :	public UtilLib_SimpleGrid,
										public FileButton_ViewMode_Parent
{
	friend DirectoryLayout;
	friend FileButton;
	friend StretchyFileBin;
private:
	bool m_amISetting;
protected:

	DirectoryLayout*	m_dirLayout;

	void SetNewPath(const char* p_newDirectory, DirectoryLayout* p_dirLayout);

	tList<FileButton*>	m_pathButtons;

public:
	PathLayout();

	////////////////////////////////////////////////
	//! From DragNDrop
	virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
	virtual bool DragNDrop_ShouldIRubberBandSelect(HWND hWnd);
	virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width,int Height,				//! Window size
									int MousePosnX,int MousePosnY,		//! Mouse posn
									WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
									WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									);

	virtual WindowLayout_Item *Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height);
	
	DirectoryLayout* GetDirectoryLayout();
	virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);
	virtual void Child_AboutToLeave(HWND hWnd, HWND NewParent);
	
	//! Creating our own context menus
	void MouseRButtonRelease(long Flags,long x,long y);

	//! My internal view mode that I use to set all of my children
	HeightWidth_ViewMode m_viewMode;
	FileButton_ViewMode* Get_FileButton_ViewMode(FileButton* fileButton){return &m_viewMode;}
	bool ShouldFileButtonExecuteOnSingleClick(FileButton* fb){return !m_amISetting;}
	virtual void ViewModeSizeChanged(FileButton_ViewMode* viewMode){}	//! We never want to change our size
	virtual void FileButton_DeletedProperties(FileButton* fb){fb->RePaint();}
	virtual bool ShouldFileButtonResizeIndividually(FileButton* fb) {return true;}
};

#endif //#ifndef __PathLayoutH_
