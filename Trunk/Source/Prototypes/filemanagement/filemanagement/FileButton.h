class FileButton;

/*!	To learn how to track changes to a folder that you are looking at :
	look in MSDN under : FindFirstChangeNotification
*/

#ifndef __FileButtonH_
#define __FileButtonH_

#define UNKNOWNDRIVEICON	"File Icons\\UNKNOWNDRIVEICON.jpg"
#define REMOVABLEDRIVEICON	"File Icons\\REMOVABLEDRIVEICON.jpg"
#define FIXEDDRIVEICON		"File Icons\\FIXEDDRIVEICON.jpg"
#define NETWORKDRIVEICON	"File Icons\\NETWORKDRIVEICON.jpg"
#define CDROMDRIVEICON		"File Icons\\CDROMDRIVEICON.jpg"
#define RAMDISKDRIVEICON	"File Icons\\RAMDISKDRIVEICON.jpg"
#define MYCOMPUTERICON		"File Icons\\My Computer.jpg"
#define IMAGEFOLDERICON		"File Icons\\SubFolder.jpg"
#define FILEICONSIZEX		160					//80	// mwatkins changed for BB
#define FILEICONSIZEY		120					//60	// mwatkins changed for BB
#define	FILEBUTTON_DEFERRED_EDIT	34571057

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define FileButton_Changed_FileName		"FileButton_Changed_FileName"
#define FileButton_Changed_ViewMode		"FileButton_Changed_ViewMode"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum
{
	FileButton_ThreadRead_Unknown,
	FileButton_ThreadRead_Cached,
	FileButton_ThreadRead_Invalid,
	FileButton_ThreadRead_Reading,
	FileButton_ThreadRead_Completed,
	FileButton_ThreadRead_PreferedNo,
	FileButton_ThreadRead_PreferedYes,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
class PathLayout;
class FileManagementDLL FileButton :	public UtilLib_DragButton,
										public BaseLoadSave,
										public BaseLoadSave_UiConfig,
										public ColorTinting,
										public CopyPasteInterface,
										public Controls_FileButton,
										public DetailsView_Row,
										//this allows single edit key mode
										public KeyboardShortCut_Selective_Interface
{
	friend DirectoryLayout;
	friend PathLayout;

public:
	static void SetFileButtonPreviewInterfaceToUse(FileButton_PreviewInterface *ToUse);
	static void SetFileButtonMouseOverPreviewInterface(FileButton_MouseOverPreviewInterface *ToUse);

	////////////////////////////////////////////////
	//! Constructors & Destructors
	FileButton(void);
	~FileButton(void);	

	////////////////////////////////////////////////
	virtual void MouseLButtonDblClick(long Flags,long x,long y);
	virtual void MouseRButtonRelease(long Flags,long x,long y);
	virtual void MouseLButtonRelease(long Flags,long x,long y);
	virtual void MouseLButtonClick(long Flags,long x,long y);
	virtual void MouseEnter(void);
	virtual void MouseExit(void);
	virtual void InitialiseWindow(void);
	virtual void DestroyWindow(void);
	virtual void ShowWindow_Callback(long Flags);	

	virtual bool KeyboardShortCut_IsKeyboardConfigurationLocal(byte *m_KeyboardMap_256Elements);

	static int FileButton_MouseLButtonDblClick_compare( const void *arg1, const void *arg2 );

	virtual void DragAndDrop_Started(void);
	virtual void DragAndDrop_Finished(void);

	//! overload this if you do not want the animation playback on rollover
	virtual bool FileButton_ShouldIDisplayAnimations(void);

	////////////////////////////////////////////////
	//! BaseWindowClass
	virtual void	PaintWindow(HWND hWnd,HDC hdc);

	////////////////////////////////////////////////
	//! Layout manager hints
	virtual long	LayoutHints_GetPreferedXSize(void);
	virtual long	LayoutHints_GetPreferedYSize(void);

	////////////////////////////////////////////////
	//! View Modes
	int GetViewMode(void);

	//! Pass in an array, copies the entire path to the array
	const char*	GetFileName(void);		

	//! Pass in an array, copies only the directory to the array
	const char*	GetDirectory(void);


	//! Be able to access the properties
	FileProperties*	GetProperties(void);
	long GetStatus(void);
	
	//! Pass in an entire path, Does not move or copy the file
	void	ChangeFileName(const char* p_newFileName);	

	//! Pass in an entire path, return true iff the file was	successully Moved.
	bool	MoveFile(const char* p_newFileName);	

	//! Pass in an new directory, return true iff the file was	successully Moved.
	bool	MoveFileTo(const char* p_newDirectory);

	//! Pass in an entire path, return true iff the file was	successully Copied.
	bool	CopyFile(const char* p_newFileName);	

	//! Pass in an new directory, return true iff the file was	successully copied.
	bool	CopyFileTo(const char* p_newDirectory);

	/*! Copy the details of the button from a different one
		Does not copy over Style, color, view mode, etc.*/
	virtual void CopyTo(FileButton* p_CopyButton);

	////////////////////////////////////////////////
	//! From Control_DragNDrop
	virtual bool DragNDrop_ShouldIDragAndDrop(HWND hWnd,bool ActualDragOperation,bool MouseOperation);
	virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
	virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width,int Height,				//! Window size
									int MousePosnX,int MousePosnY,		//! Mouse posn
									WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
									WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									);

	// Deferred callback
	// This function works out the icon that is read on a seperate thread
	virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	////////////////////////////////////////////////
	//! Load save implementation
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	virtual bool BaseLoadSave_UiConfig_ShouldISaveOrLoad(void) 
		{
			return false;
		}

	virtual bool Persitance_ShouldBeUsed_Load(void);
	virtual bool Persitance_ShouldBeUsed_Save(void);

	virtual bool IgnoreCopyPaste(void) 
		{
			return true;
		}

	//! Get the icon to manually set its image or filename
	virtual ScreenObject* GetFileIcon(void)
		{
			return &m_icon;
		}

	////////////////////////////////////////////////////////
	//! View Modes
	/*	The internal ViewMode
		Note that this ViewMode ONLY affects the FileButton iff another ViewMode has not been assigned by a parent */
	UtilLib_FileButton_ViewMode		m_internalViewMode;

	/*	Accesses the view mode currently being used.
		Note that changing this view mode may also effect other FileButtons that may be using it	*/
	FileButton_ViewMode*			GetCurrentViewMode(void)
		{
			return m_currentViewMode;
		}

	FileButton_ViewMode_Parent*		GetViewModeParent(void)
		{
			return m_parent;
		}

	//! Use NULL to set the view mode to the internal view mode
	virtual bool SetCurrentViewMode(FileButton_ViewMode* newViewMode);

	void SetFileButtonParent(FileButton_ViewMode_Parent* newParent);

	/////////////////////////////////////
	//! ColorTinting
	virtual void		ColorTinting_SetTintColor(unsigned RGBA);
	virtual unsigned	ColorTinting_GetTintColor(void);

	virtual bool		ColorTinting_UseIfNotPopup(void)
		{
			return true;
		}

	virtual bool		ColorTinting_IsColorTintClear(void);
	virtual void		ColorTinting_ClearColorTint(void)
		{
			ColorTinting_SetTintColor(RGBA(255,255,255,0));
		}
	

	//! Call to programmatically execute a file
	//! This is called from the double click
	void ExecuteFile(void);

	//! Return false to this if you do not want your FileButtons showing a ContextMenu
	virtual bool FileButton_ShouldIShowStandardContextMenu(void)
		{
			return true;
		}

	//! Used by View Modes to grab the Alias
	virtual const char*	FileButton_GetAlias(bool getFileNameIfBlank = true);
	virtual void		FileButton_SetAlias(const char* newAlias);

	//! Stop any threaded preview 
	virtual void	MouseRButtonClick(long Flags,long x,long y);

	//! Used by Videw Modes to know whether to draw the Warning Border
	virtual bool	FileButton_ShouldDrawWarningBorder(void);

	//! Am I Thread Reading
	void	FB_CancelThreadReading(void);
	bool	FB_DoesUserPreferThreadRead(void);

	int		FB_GetThreadReadStatus(void)
		{
			return m_threadReadStatus;
		}

	void	FB_ThreadReadIcon(void);

	//! Refresh the File Icon reading, try to thread read again if possible
	void	FB_RefreshIcon( bool OnlyIfInvalid = false );

	//! Grab all of the selected FileButtons in order of selection from the parent
	void GrabSelectedButtons(tList<FileButton*>& Windows);

	//! This rect is set each time the FileButton is drawn, use it to know where the icon was drawn last
	RECT m_lastIconRect;


	//Begin DetailsView_Row---------------------------------------------------------------------------------------------------
		virtual DetailsView_Element* DVR_GetDetailsViewElement(unsigned index);
		virtual DetailsView_Element* DVR_GetDetailsViewElement(const char* columnName);
		virtual void DVR_Initialize(void);	// Added here as a callback to set up Asset Management on the props
		virtual void DVR_Cleanup( void );

		virtual POINT DVR_GetContextMenuClickPoint(void)
			{
				return m_contextMenuClickPoint;
			}

		virtual char* DVR_GetDetailsViewColumnName(POINT mousePoint);
		virtual void DVR_RemoveElement(char* columnName);
		virtual void DVR_AddElement(char* columnName);
		virtual void DVR_CopyElementFrom(DetailsView_Element* masterElement,unsigned CopiedIndexCount=-1);
		virtual void DVR_Refresh(void);

		// Starting to edit columns
		virtual void FB_DVR_StartEditingColumn(const char* columnName, POINT* clickPoint = NULL, const char *InitString=NULL,bool HighLightInitString=true);

		//This is made for derived classes to notify when a new button selection has occured from FB_SelectNextEditableElement()
		virtual void FB_SelectedThisButton(FileButton *) {}
		//Here is where we pass on the Editing Baton if the user hit Tabs or arrows
		virtual void FB_SelectNextEditableElement(const char *ColumnName,Popup_Escape direction);

		/*!	Used for tabbing forward and backwards on a row
			forward is true if tabbing forward, starting at adjoining this
			if adjoiningThis is NULL, get the first element if going forward, the last if not 
		*/
		virtual DetailsView_Element* DVR_GetNextEditableDVE(DetailsView_Element* adjoiningThis, bool forward);
		char *GetDetailsColumnSelection(void);
		void SetDetailsColumnSelection(const char *ElementName,bool refresh=false);

		POINT m_contextMenuClickPoint;
	//End DetailsView_Row---------------------------------------------------------------------------------------------------

protected:
	FileProperties				*m_properties;
	FileButton_ViewMode_Parent	*m_parent;

	//! A list of the base Details View Elements
	BaseFile_DetailsView_Element* m_baseFile_DetailsView_Elements[BasicFile_DVElem_NumItems];

	void SetFileButtonProperties(FileProperties* newProps, bool addReference = true, bool redraw = true);

	////////////////////////////////////////////////
	//! My callback on changed names, etc...
	virtual void FileButton_FileNameChanged(const char *NewFileName) 
		{
		}

	bool m_repaintMe;
	void RePaintMe(bool checkSize);

	////////////////////////////////////////////////
	virtual RECT GetRolloverAnimationPosition(void);
	virtual void StartRolloverAnimation(void);
	virtual void StopRolloverAnimation(bool WaitUntilStopped=false);

	//! The icon I am going to paint
	BitmapItem_Cache	m_icon;
	bool				m_iconLoaded;
	char*				m_waitingForImage;

	//! This is how the icon looks at the file properties
	virtual void FB_LoadIcon(FileProperties* fp);
	virtual void ThreadReadIconFile(FileProperties* fp, char* fileToRead);
	bool		 FB_ReadDefaultIcon(FileProperties* fp);

	//! Functions for reading icons
	static char* ReadDriveIconFile(FileProperties* fileProps);
	static char* ReadFolderIconFile(FileProperties* fileProps);
	static char* ReadInvalidIconFile(FileProperties* fileProps);
	static char* ReadSpecialIconFile(FileProperties* fileProps);
	static char* ReadDefaultIconFile(FileProperties* fileProps);
	//static bool  FB_IsFileReadable(FileProperties* fp);

	//! Keeping track of the Thread Read Status
	int		m_threadReadStatus;
	void	FB_SetThreadReadStatus(int newStatus);
	void	FB_StopThreadReading(void);

private:
	long m_Ref;
	FileButton_ViewMode*	m_currentViewMode;
	bool					m_haveIbeenSet;
	//This will let me know what I am painting to avoid repainting recursion
	bool					m_AmIPainting;

	// Are we being dragged
	bool	m_BeingDragged;

	//! A temporary buffer for holding filenames while they are being read;
	static char filePath[MAX_PATH];

	//! This is the file preview to use
	static FileButton_PreviewInterface			*m_FileButton_PreviewInterface;
	static FileButton_MouseOverPreviewInterface *m_FileButton_MouseOverPreviewInterface;
};

// A Helper function for dropping items into something that represents a FileProperty (DirectoryLayout / FileButton)
extern void DropOntoProperties(FileProperties* fp, WindowLayout_Item *ObjectsDropped, BaseWindowClass* DroppedOn);
extern bool CanItemBeDroppedOnProperties(FileProperties* fp, Control_DragNDrop_DropInfo *Dropped, BaseWindowClass* DroppedOn);

// A simple internal file button implementation
struct FileButton_NoMenu : public FileButton
{	// No menus
	void MouseRButtonRelease(long Flags,long x,long y) {};
	void MouseRButtonClick(long Flags,long x,long y) {};
};

#endif


