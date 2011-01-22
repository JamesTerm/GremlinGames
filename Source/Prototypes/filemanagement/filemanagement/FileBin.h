class StretchyFileBin;
class DirectoryLayout_ContextMenu;

#ifndef __FileBinH_
#define __FileBinH_

/*	FileBin
*/

enum FileBin_ButtonVars
{
	FileBin_NoButton,
	FileBin_Cut,
	FileBin_Copy,
	FileBin_Paste,
	FileBin_Delete,
	FileBin_Undo,
	FileBin_NewFolder,
	FileBin_Edit,
	FileBin_Back,
	FileBin_Forward,
	FileBin_Favorites,
	FileBin_AddFav,
	FileBin_DeleteFav,
	FileBin_OptSize,
	FileBin_ButtonViewMenu
};

//! Control Colors
#define FileBin_Color_Cut			RGBA(255,0,222)
#define FileBin_Color_Copy			RGBA(255,43,222)
#define FileBin_Color_Paste			RGBA(255,71,222)
#define FileBin_Color_Delete		RGBA(255,100,222)
#define FileBin_Color_NewFolder		RGBA(255,233,233)
#define FileBin_Color_Edit			RGBA(255,138,222)
#define FileBin_Color_Back			RGBA(255,182,222)
#define FileBin_Color_Forward		RGBA(255,222,222)
#define FileBin_Color_ListIcon		RGBA(103,233,0)
#define FileBin_Color_SmallIcon		RGBA(141,233,0)
#define FileBin_Color_LargeIcon		RGBA(179,233,0)
#define FileBin_Color_Favorites		RGBA(235,99,110)
#define FileBin_Color_AddFav		RGBA(163,144,110)
#define FileBin_Color_DeleteFav		RGBA(163,187,110)
#define FileBin_Color_ScrollBar		RGBA(128,155,163)
#define FileBin_Color_ScrollBarUp	RGBA(128,193,209)
#define FileBin_Color_ScrollBarDn	RGBA(182,20,83)
#define FileBin_Color_Scrollwindow	RGBA(209,255,0)
#define FileBin_Color_PathWindow	RGBA(117,146,222)
#define FileBin_Color_ButtonViewMenu RGBA(156, 179, 0)
#define FileBin_Color_ColumnHeader	RGBA(255,149,71)
#define FileBin_Color_FavTabPanel	RGBA(152,48,48)

class FileManagementDLL	StretchyFileBin :	public AutoSkinControl,
											public Control_DragNDrop_Remap,
											public CopyPasteInterface	//! I don't want module copy/paste.  Makes the UI too confusing
{
protected:
	//Use BaseLoadSave version of Details view info
	BLS_DetailsView_Info m_BLS_DetailsView_Info;
	
	///////////////////////////////
	//! Dynamic variables
	Dynamic<int>	m_button;
	Dynamic<int>	m_viewMode;
	Dynamic<float>	m_scrollBarVar;
	Dynamic<float>	m_scrollBarMin;
	Dynamic<float>	m_scrollBarMax;
	Dynamic<float>	m_scrollBarWidth;

	///////////////////////////////
	//! ScreenObjects we own for controls
	ScreenObject_StretchedBitmap	m_sliderBitmaps[4];
	ScreenObject_StretchedBitmap	m_DetailsViewBtnBitmaps[4];

	// Scroll bar for the Details View
	Dynamic<float>	m_tabScrollBarVar;
	Dynamic<float>	m_tabScrollBarMin;
	Dynamic<float>	m_tabScrollBarMax;
	Dynamic<float>	m_tabScrollBarWidth;
	Dynamic<unsigned> m_textSpacing;

	//This will retain the column header pointer to safely close the details info on destroy window
	DetailsView_ColumnLayout *m_columnHeader;
	///////////////////////////////
	//! The controls we need to keep handles on
	DirectoryLayout*		m_fileLayout;
	UtilLib_Slider*			m_slider;
	PathLayout*				m_pathLayout;
	UtilLib_ButtonLabel*	m_viewModeLabelButton;
	TextItem				m_buttonLabelText[4];

	//! So we are getting the appropriate DirectoryLayout
	virtual const char* GetDirectoryLayoutType();

	//! Override in derived classes to specify the default directory for this filebin type
	virtual const char* GetDefaultDirectory();

	///////////////////////////////
	//! The list of favorites directories
	static unsigned s_favoritesRefCt;
	virtual FavoriteFileList* SFB_GetFavoriteFileList()
		{	return FavoriteFileList::GetInstance();
		}
	friend class FileBin_FavoritesList_TabPanel;

	//! My very own stretchy screen resource
	Auto_StretchySkin	*m_ScreenObject_StretchySkin;

	//! My AutoSkinControl Callbacks
	virtual void InitializeSubControls();
	virtual void SkinChangedCallback(char* newSkinPath);

	//! Set this before initialization to initialize to this directory rather than "My Computer"
	char m_initializeWithDirectory[MAX_PATH];

	//! A friend
	DirectoryLayout *GetFileLayout(void) { return m_fileLayout; }
	friend class DirectoryLayout_ContextMenu;

	//! Get the class for teh filebin favourites
	virtual const char *GetFileBinFavouritesType( void )
		{	return "FileBin_FavoritesList_TabPanel";
		}

public:
	///////////////////////////////
	//! Constructors & Destructors
	StretchyFileBin();
	~StretchyFileBin();

	static void OnProcessAttach();
	static void OnProsessDetach();

	/////////////////////////////////////////
	//! From DynamicListener
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	//! This is the initialisation routine
	virtual void InitialiseWindow(void);
	virtual void DestroyWindow(void);

	//! Get a FileButton child with a particular FileName
	FileButton* GetFileButton(const char* fileName)
		{	return m_fileLayout->GetFileButton(fileName);
		}

	//If passive to BLS=true then it will only set if nothing else has already changed the path
	bool		StretchyFileBin_InitDirectory(const char* p_dir,bool PassiveToBLS=false);
	const char*	StretchyFileBin_GetDirectory(char* p_dir = NULL);
	void		StretchyFileBin_SetDirectory(const char* p_dir);

	////////////////////////////////////////////////
	//! From Control_DragNDrop
	bool DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped);
	WindowLayout_Item *DragNDrop_DropItemsHere
							(	int Width,int Height,				//! Window size
								int MousePosnX,int MousePosnY,		//! Mouse posn
								WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
								WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
								bool Resizing,bool DroppedHere
								);

	//! We have some Control over what kinds of files we want to show
	void	ShowSystem(bool showSystem){if (m_fileLayout) m_fileLayout->ShowSystem(showSystem);};
	void	ShowHidden(bool showHidden){if (m_fileLayout) m_fileLayout->ShowHidden(showHidden);};
	void	ShowEncrypted(bool showEncrypted){if (m_fileLayout) m_fileLayout->ShowEncrypted(showEncrypted);};

	//! Scripting Commands
	virtual bool ExecuteCommand(	char *FunctionName,						//! The string representation of the command to execute
									ScriptExecutionStack &InputParameters,	//! The set of input parameters to your function
									ScriptExecutionStack &OutputParameters,	//! You can use this to access variables as well
									char *&Error);							//! If you supported the command but failed for some reason
																			//! return a user string here

	////////////////////////////////////////////////
	//! Load save implementation
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	//! Member fynctions for Cut/Copy/Paste/Delete
	bool FBin_Cut();
	bool FBin_Copy();
	bool FBin_Paste();
	bool FBin_Delete();

	//! CopyPasteInterface
	virtual bool IgnoreCopyPaste(void) { return true; }	// No module clipboard.  Not very useful.

};

#endif
