class DirectoryLayout;
class DirectoryLayout_EditList;

#ifndef __DirectoryLayoutH_
#define __DirectoryLayoutH_

// Pre-Declarations
class PathLayout;
class StretchyFileBin;
class Details_ViewMode;

/*	DirectoryLayout
A UtilLib_SimpleGrid that is tied to a directory in the file system.  When a directory is chosen,
All of the current children are deleted and new FileButton children are added that represent
each file in the directory.  OnTimer also resets the directory.
  */

#define DirectoryLayout_SpacingX		3
#define DirectoryLayout_SpacingY		3

//! This is how many milliseconds we wait between looking for directory changes
#define DirectoryLayour_ScanForFolderChanges	500

//! This is the changed message that gets send when the directory is changed
#define DirectoryLayout_DirectoryChanged "DirectoryLayout::DirectoryChanged"

//! The Temporary file used to save BLS items
#define TEMPBLSFILE "Temp_BLS_FILE!@#$"


struct DirectoryLayout_WindowLayout_Item
{
	WindowLayout_Item*	window;
	FileProperties*		m_file;
	FileButton*			m_fileButton;
};

#define DirectoryLayout_NewFolder "DirectoryLayout_NewFolder"

enum e_NewDirectoryType
{
	DirectoryLayout_InvalidDirectory = 0,
	DirectoryLayout_RefreshCurrent	 = 1,
	DirectoryLayout_NewDirectory	 = 2
};

class FileManagementDLL DirectoryLayout :	public UtilLib_SimpleGrid,
											public FileButton_ViewMode_Parent,
											public DetailsView_RowCollection
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! My friends
	friend FileButton;
	friend StretchyFileBin;
	friend PathLayout;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! My data
private:
	unsigned			m_willIBeAdjusting;		// TODO: Get rid of this!!!
	bool				m_destroying;			// TODO: If I get rid of the m_willIBeAdjusting, I can get rid of this automatically
	bool				m_amIUsingHistory;		// TODO: I hate this f'ing state shizzy

	char				m_tempDirectory[MAX_PATH];
	bool				m_sortNextLayout;

protected:
	tList<FileProperties*>	m_history;
	unsigned				m_currentPosition;

	//! This is used to scan for changes in the folder
	DirectoryMonitor		m_DirectoryMonitor;

	bool	m_showHidden, m_showEncrypted, m_showSystem;	//! Are we showing system, encrypted, and hidden files
	bool	m_showFiles, m_showFolders;						//! Are we showing files folders or both?

	PathLayout*	m_layoutToInform;
	
	tList<FileButton*>		m_recycledButtons;
	tList<FileButton*>		m_activeButtons;

	// My View Mode
	Details_ViewMode*		m_viewMode;

	// This is needed in Sort Properties, which is used by Qsort
	static DirectoryLayout* s_sortingDetailsLayout;
	static int SortProperties(const void *arg1, const void *arg2);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! Implementation

	//! Checks to be sure all children are valid and deletes the ones that are not.
	//! Returns true iff there were any deleted
	bool ValidateChildren(tList<FileProperties*>* fileList);

	//! Checks to see if file still exists and has same directory
	bool ValidateFileButton(FileButton* p_fileButton, tList<FileProperties*>* fileList);

	//! Where the actual directory is set, called by ReceiveDeferredMessage();
	//! Returns true if the current directory changes
	bool DeferredSetDirectory(const char *p_directory);

	virtual bool DisplayThisFile(WIN32_FIND_DATA findData, char* filename);

	void RecycleAllChildren();
	void RecycleChild(FileButton* fb);
	bool IsActiveButton(HWND fb);
	FileButton* GetRecycledFileButton(FileProperties* pf);

	bool CreateDirectoryListing(	tList<FileProperties*>* fileList, 
									FileProperties* searchProps, 
									const char* filter, 
									bool recursive, 
									bool includeFolders=true,
									bool includeFiles=true);

	// Returns bitflags defined in e_NewDirectoryType
	e_NewDirectoryType ValidateNewDirectory(const char* newDirectory);

	virtual bool GetRecursiveSearch(){return false;}

	//! From UtilLib_SimpleGrid to be sure recycled files are out of the way
	virtual void LayoutManager_GetChildExtents
		(WindowLayout_Item *Children, long windowWidth, long windowHeight,
		long & MinX, long & MaxX, long & MinY, long & MaxY);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//! Interface
public:
		
	//! Constructors & Destructors
	DirectoryLayout();
	~DirectoryLayout();	

	//! Creating our own context menus
	void MouseRButtonRelease(long Flags,long x,long y);

	//! Watching out for our childern leaving
	virtual void Child_AboutToLeave(HWND hWnd, HWND NewParent);	//! This is caused by DragNDrop draging something outside this window

	//! Be sure I am not receiving Deferred Messages after I die
	virtual void DestroyWindow();

	////////////////////////////////////////////////
	
	//! Accessing the Current directory
	const char* GetDirectory( char *p_directory = NULL);	//! Returns the current directory
	FileProperties* GetDirectoryProps(){return m_history[m_currentPosition];}	//! Returns the file properties for the current directory

	bool SetInitialDirectory(const char *p_directory);

	void SetDirectory(const char *p_directory, bool defer = true);

	tList<FileProperties*>* GetHistory();

	unsigned GetCurrentPosition();
	unsigned SetCurrentPosition(unsigned p_newPosition);	//! returns the position after being set

	////////////////////////////////////////////////
	
	//! Retrieve a FileButton child with p_filename, return NULL if none exists
	FileButton* GetFileButton(const char* p_filename);

	////////////////////////////////////////////////
	
	//! From DragNDrop
	virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
	virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width,int Height,				//! Window size
									int MousePosnX,int MousePosnY,		//! Mouse posn
									WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
									WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									);

	////////////////////////////////////////////////
	
	//! From DynamicListener
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	////////////////////////////////////////////////
	
	//! Quickly Create a new folder
	bool	DirLayout_CreateNewFolder(char* p_folderName = NULL, bool p_moveSelectedInto = false); // mwatkins - changed def param val to false

	//! Deferred callback
	virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

	//! Local versions that grab all of the selected children in order
	bool	EditCut();
	bool	EditCopy();
	bool	EditPaste();
	bool	EditDelete();
	void	EditProperties(long XPos = -1, long YPos = -1);

	//! Adds all of the FileNames of the selected FileButtons
	void	GetSelectedFileNames(tList<FileProperties*>* fileList);

	/////////////////////////////

	//! We have some Control over what kinds of files we want to show
	void	ShowSystem(bool showSystem);
	//! We have some Control over what kinds of files we want to show
	void	ShowHidden(bool showHidden);
	//! We have some Control over what kinds of files we want to show
	void	ShowEncrypted(bool showEncrypted);
	void	ShowFiles(bool showFiles);
	void	ShowFolders(bool ShowFolders);

	//! This is called when the window is create
	virtual void InitialiseWindow(void);
	//! You can setup timers (see above) then this call will get called each time
	virtual void OnTimer(unsigned long TimerID);

	//! My internal view mode that I use to set all of my children
	FileButton_ViewMode* Get_FileButton_ViewMode(FileButton* fileButton);
	virtual void ViewModeSizeChanged(FileButton_ViewMode* viewMode);
	virtual void FileButton_DeletedProperties(FileButton* fb);
	virtual bool ShouldFileButtonResizeOnViewModeChange(FileButton* fb)	{return false;}

	// Setting the columnheader, be sure to set its details info to be the same as view modes
	virtual void DVRC_SetColumnHeader(DetailsView_ColumnLayout *detailsViewHeader);

	//! From UtilLib_SimpleGrid to be sure recycled files are out of the way
	virtual WindowLayout_Item *Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height);

	//! Be sure selection order does not look at recycled buttons
	virtual void GetChildrenInSelectionOrder(tList<HWND> &ChildrenList);

	//! Do we want to use any kind of filter?
	virtual char* GetFileFilter(){return "*";}

// DetailsView_RowCollection
public:
	// Iteratively a list of all of the available rows, returns NULL when there are no more
	virtual void DVRC_GetDetailsViewRowList(tList<DetailsView_Row*>& rowList);
};
#endif
