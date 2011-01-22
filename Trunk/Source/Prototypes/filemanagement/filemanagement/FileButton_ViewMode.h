#ifndef __FileButton_ViewModeH_
#define __FileButton_ViewModeH_

//! Use this changed message to force all FileButtons interested in this mode to resize themselves
//! Be sure to not call another changes message to avoid double re-paints
//! Before sending the message, call m_parent->ViewModeSizeChanged(this);
#define FileButton_ViewMode_SIZE_Changed	"FileButton_ViewMode_SIZE_Changed"

//! Use these messages for the four button changed states
//! Only buttons in the right state will be re-painted
#define FileButton_ViewMode_State0_Changed	"FileButton_ViewMode_State0_Changed"
#define FileButton_ViewMode_State1_Changed	"FileButton_ViewMode_State1_Changed"
#define FileButton_ViewMode_State2_Changed	"FileButton_ViewMode_State2_Changed"
#define FileButton_ViewMode_State3_Changed	"FileButton_ViewMode_State3_Changed"

//! All other changed messages sent will cause the FileButtons to re-paint themselves
const unsigned FBVM_SizeBeforeWiderEdge = 32;

// Strings Used to show what will be displayed in a File Button, and in the Menu
#define FileButton_Naming_AltName	TL_GetString( "FileBin::Naming_AltName", "Alias" )
#define FileButton_Naming_ExtOnly	TL_GetString( "Extension Only" )
#define FileButton_Naming_NameOnly	TL_GetString( "Name w/o Extension" )
#define FileButton_Naming_FullName	TL_GetString( "Name with Extension" )
#define FileButton_Naming_Path		TL_GetString( "Full Path" )
#define FileButton_Naming_PathOnly	TL_GetString( "Path Only" )
#define FileButton_Naming_Type		TL_GetString( "File Type" )

class FileButton_ViewMode_Parent;
class FileManagementDLL FileButton_ViewMode : public DynamicTalker
{
public:
	FileButton_ViewMode(){m_parent = NULL;}
	virtual ~FileButton_ViewMode(){}

	//! The interface for obtaining the proper size
	virtual long FileButton_ViewMode_GetPreferedXSize(FileButton* fb = NULL)=0;
	virtual long FileButton_ViewMode_GetPreferedYSize(FileButton* fb = NULL)=0;

	//! Get the RECTS for where the ICON and text are, based on the entire controlRect
	virtual void	FileButton_ViewMode_GetRects(RECT & iconRect, RECT & nameRect, RECT controlRect)=0;
	
	//! For Details Editing, return a string here for the column that might be edited
	virtual char*	FileButton_ViewMode_GetColumnAtPoint(POINT point, FileButton* fb)
		{	return NULL;
		}

	virtual bool	FileButton_ViewMode_GetEditingRect(RECT& rect,const char *columnName, FileButton* fb)
		{	return false;
		}

	virtual char*		FileButton_ViewMode_GetColumnAtIndex(unsigned index, FileButton* fb)
		{	return NULL;
		}

	virtual unsigned	FileButton_ViewMode_GetColumnIndex(const char *columnName, FileButton* fb)
		{	return (unsigned)-1;
		}

	virtual unsigned	FileButton_ViewMode_GetNumColumns(FileButton* fb){return 0;}
	
	//! Drawing the FileButton
	virtual void FileButton_ViewMode_Draw
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)=0;

	//! Does this viewmode support animations ?
	virtual bool FileButton_ViewMode_IsAnimated(FileButton* fileButton)=0;

	//! We could show relative paths here
	//virtual char*	FileButton_ViewMode_GetRelativePath(){return NULL;}

	FileButton_ViewMode_Parent* m_parent;
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FileManagementDLL FileButton_ViewMode_Parent
{
public:
	//! Return NULL to allow the FileButton to use its own internal view mode
	//! This function is called when a FileButton is dropped onto your window
	virtual FileButton_ViewMode* Get_FileButton_ViewMode(FileButton* fileButton)
		{	return NULL;
		}
	
	//! This function is called when the File tied to fb is deleted
	//! The default implementation deletes the FileButton
	virtual void FileButton_DeletedProperties(FileButton* fb);

	//! As the parent of this FileButton, you can control execution
	virtual bool ShouldFileButtonExecuteOnSingleClick(FileButton* fb)	
		{	return false;
		}

	virtual bool ShouldFileButtonExecuteOnDoubleClick(FileButton* fb)	
		{	return true;
		}
	
	//! Do you want the button to show the context menu that has cut-copy-paste into?
	virtual bool ShouldIShowCutCopyPasteContextMenu(FileButton* fb)		
		{	return true;
		}

	//! Do you want the FileButton to handle its own resizing when the View Mode changes?
	//! If you return false, be sure to handle the size change somewhere else, like ViewModeSizeChanged()
	virtual bool ShouldFileButtonResizeOnViewModeChange(FileButton* fb)	
		{	return true;
		}

	//! Do you want the FileButton to handle its own resizing when it becomes your child?
	//! If you return false, be sure you manage the sizing of the child somewhere, like DragNDrop
	virtual bool ShouldFileButtonResizeOnParentChange(FileButton* fb)	
		{	return true;
		}

	//! Do you want different file buttons to have different sizes?
	//! for instance, based on the length of their name?
	virtual bool ShouldFileButtonResizeIndividually(FileButton* fb) 
		{	return false;
		}

	//! This function is called by UtilLib_FileButton_ViewMode whenever the size is changed
	//! If you create a FileButton_ViewMode that does NOT inherit from UtilLib_FileButton_ViewMode,
	//! Be sure to call this function whenever your size changes
	//! This function allows you to change the sizes of all of the your FileButtons at once and re-layout the view
	//! If you handle the resizing, be sure to return false on ShouldFileButtonResizeOnViewModeChange()
	virtual void ViewModeSizeChanged(FileButton_ViewMode* viewMode)		
		{
		}

	//! Do we want FileButtons Saving Themselves out with bls persistance?
	virtual bool Persitance_ShouldBeUsed_Load(FileButton* fb) 
		{	return false;
		}

	virtual bool Persitance_ShouldBeUsed_Save(FileButton* fb) 
		{	return false;
		}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! These messages are only sent of NO FileButton_ViewMode_SIZE_Changed is sent
#define FileButton_ViewMode_ViewMode_Changed		"FileButton_ViewMode_ViewMode_Changed"
#define FileButton_ViewMode_IconAspect_Changed		"FileButton_ViewMode_IconAspect_Changed"
#define FileButton_ViewMode_NameMode_Changed		"FileButton_ViewMode_NameMode_Changed"
#define FileButton_ViewMode_Loaded					"FileButton_ViewMode_Loaded"

enum eFileButton_ViewMode
{
	FileButton_ViewMode_NameOnly = 1,
	FileButton_ViewMode_SmallIcon = 2,
	FileButton_ViewMode_IconOnly = 4,
	FileButton_ViewMode_LargeIcon = 5,
	FileButton_ViewMode_SuperLargeIcon = 6,	// mwatkins
	FileButton_ViewMode_Details = 8,
	FileButton_ViewMode_DetailsIcon = 10
};

#define FileButton_Size_IconLargeX			75
#define FileButton_Size_IconLargeY			60
#define FileButton_Size_IconSmallX			32
#define FileButton_Size_IconSmallY			24
#define FileButton_Size_ControlListX		150
#define FileButton_Size_FontSize			TL_GetFloat("VTNTAPI::FileButtonFontSize",13)
#define FileButton_Size_ControlSuperLargeX	FileButton_Size_IconLargeX + FileButton_Size_ControlListX
#define FileButton_Size_ControlSuperLargeY	FileButton_Size_IconLargeY
#define FileButton_Size_ControlLargeX		FileButton_Size_IconLargeX
#define FileButton_Size_ControlLargeY		FileButton_Size_IconLargeY + FileButton_Size_FontSize + 2
#define FileButton_Size_ControlSmallX		FileButton_Size_IconSmallX + FileButton_Size_ControlListX
#define FileButton_Size_ControlSmallY		FileButton_Size_IconSmallY

const float ICON_ASPECT_4x3 = 1.3333333333f;
const float ICON_ASPECT_16x9 =1.7777777778f;
const int ICON_RES_WIDTH = 13;  //Use 30 to Left Justify

class FileManagementDLL UtilLib_FileButton_ViewMode :	public FileButton_ViewMode,
														public DynamicListener,
														public BaseLoadSave
{
protected:
	//! My TextItems
	TextItem		m_textItemLocal[4];
	TextItem*		m_textItemToUse[4];

	//! This is the TextItem that is actually painted
	//! The text is set here with the proper colors etc. as to not affect the other text items
	//! Each time a FileButton is drawn
	TextItem		m_textItemPainted;

	//! My naming information, view mode, and size
	//char*	m_relativePath;
	char*	m_sortString;
	int		m_viewMode;
	long	m_XSize, m_YSize;
	float	m_iconAspect;

	//! The icon used for the cut !
	//static HICON s_loadingIcon;

	//! Override this function to modify textItem before it is painted to the screen
	//! This is called AFTER all ofthe other values, (like color tinting & name) have been set
	virtual void TweakTextItem(FileButton* fileButton, TextItem* textItem, bool missingRef){}

	// Getting the default sizes for the different buttons
	long GetTextAreaHeight();
	virtual void UFBVM_SendSizeChanges();

public:
	// Helper Functions for drawing
	void UFBVM_DrawName
		(HWND hWnd, HDC hdc, FileButton* fileButton, char* text, bool missingRef, 
		int buttonState, unsigned ColorTint, RECT nameRect);
	void UFBVM_DrawIcon
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT iconRect);

	//! Thse functions are public so that selection can look teh same across the board	
	static void UFBVM_DrawBorder_Default
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect);
	//! This version has slightly rounded edges (as in the Storyboard)
	static void UFBVM_DrawBorder_Rounded
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect);
	//! This allows you to overload how selection appears
	virtual void UFBVM_DrawBorder
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
	{	// Call the default window selection display routine
		UFBVM_DrawBorder_Default(hWnd,hdc,fileButton,buttonState,ColorTint,controlRect);
	}
	static void UFBVM_DrawWarning
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect);	

	UtilLib_FileButton_ViewMode();
	virtual ~UtilLib_FileButton_ViewMode();

	////////////////////////////////////
	//! FileButton_ViewMode
	virtual long FileButton_ViewMode_GetPreferedXSize(FileButton* fb = NULL);
	virtual long FileButton_ViewMode_GetPreferedYSize(FileButton* fb = NULL);

	//! Get the RECTS for where the ICON and text are, based on the entire controlRect
	virtual void	FileButton_ViewMode_GetRects(RECT & iconRect, RECT & nameRect, RECT controlRect);

	//! Drawing the FileButton
	virtual void FileButton_ViewMode_Draw
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect);

	//! Grabbing the 4 text items
	TextItem*	FileButton_ViewMode_GetTextItem(unsigned buttonState){return m_textItemToUse[buttonState%4];}
	void		FileButton_ViewMode_SetTextItem(TextItem* textItem, unsigned buttonState);
	
	//! Setting up the naming modes
	//virtual char*	FileButton_ViewMode_GetRelativePath(){return m_relativePath;}
	//virtual void	FileButton_ViewMode_SetRelativePath(char* relativePath = NULL);
		//! if relativePath != NULL, nameMode will always be set to include the PATH

	//! Setting up the viewing modes and sizes
	int			FileButton_ViewMode_GetViewMode(){return m_viewMode;}
	virtual void FileButton_ViewMode_SetViewMode(int viewMode, long XSize = -1, long YSize = -1);
		//! Use -1 for XSize and YSize to use the default sizes listed above

	//! We can actually change the aspect ratio of the icon! (this might change the size based on what mode we are in!)
	float		FileButton_ViewMode_GetIconAspect(){return m_iconAspect;}
	void		FileButton_ViewMode_SetIconAspect(float iconAspect);

	//! We listen for changes in our text items and pass those changes on to you!
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	//! The default is to be animated
	bool FileButton_ViewMode_IsAnimated(FileButton *FB) { return true; }

	////////////////////////////////////////////////
	//! Load save implementation
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	virtual void UFBVM_AlterTextItemInProject(bool inProject, TextItem* ti);

	//! Do you want the FileButton to listen to its own icon changes
	virtual bool	FileButton_ListenToIconChanges(FileButton* fb){
		return ( (m_viewMode == FileButton_ViewMode_LargeIcon) || 
			     (m_viewMode == FileButton_ViewMode_SmallIcon) ||
				 (m_viewMode == FileButton_ViewMode_IconOnly) ||
				 (m_viewMode == FileButton_ViewMode_SuperLargeIcon)	// mwatkins
				 );
	}

	////////////////////////////////////////////////
	//! The Text that goes on the FileButton
	virtual unsigned GetTextFromFileButton(char *DestBuffer,unsigned LengthOfBuffer,FileButton* fb, char* sortString = NULL, bool allowBlankIfNoAsset = false);
	virtual unsigned GetSortTextFromFilename
		(char *DestBuffer,unsigned LengthOfBuffer,FileButton* fb, bool & missingAsset, double & sortValue, 
			char* sortString = NULL, bool allowBlankIfNoAsset = false);

	static void OnProcessAttach(void);
	static void OnProcessDetach(void);

	void CopyTextItemAttributes(TextItem* textItem, int buttonState, unsigned ColorTint);

	//! Setting the Sort String
	void	FileButton_ViewMode_SetSortString(char* sortString);

	// This returns m_sortString or Alt Name if the point is in the name rect
	virtual char*	FileButton_ViewMode_GetColumnAtPoint(POINT point, FileButton* fb);
	virtual bool	FileButton_ViewMode_GetEditingRect(RECT& rect,const char *columnName, FileButton* fb);
	virtual char*		FileButton_ViewMode_GetColumnAtIndex(unsigned index, FileButton* fb);
	virtual unsigned	FileButton_ViewMode_GetColumnIndex(const char *columnName, FileButton* fb);
	virtual unsigned	FileButton_ViewMode_GetNumColumns(FileButton* fb);
};

//! This little class helps if you want a FileButton to always fit the width OR height of your window
//! Fitting to Height works regardless of view mode.
//! Fitting to width only happens in FileButton_ViewMode_NameOnly and FileButton_ViewMode_SmallIcon
class FileManagementDLL HeightWidth_ViewMode : public UtilLib_FileButton_ViewMode
{
private:
	long m_oldSpacerX;
	
public:
	bool m_matchWidth, m_matchHeight;
	BaseWindowClass* m_parentWindow;

	virtual ~HeightWidth_ViewMode();
	HeightWidth_ViewMode();

	void SetMatchingParentWindow(BaseWindowClass* parentWindow);

	virtual bool ShouldIMatchHeight(FileButton* fb);
	virtual bool ShouldIMatchWidth(FileButton* fb);

	virtual long FileButton_ViewMode_GetPreferedXSize(FileButton* fb = NULL);
	virtual long FileButton_ViewMode_GetPreferedYSize(FileButton* fb = NULL);
	void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);
	
	// When setting the view mode to something wide, set the spacing to 0
	virtual void FileButton_ViewMode_SetViewMode(int viewMode, long XSize = -1, long YSize = -1);
		//! Use -1 for XSize and YSize to use the default sizes listed above
};

#endif	//! #ifndef __FileButton_ViewModeH_