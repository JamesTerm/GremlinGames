class FilePropertiesDlg;

#ifndef __FilePropertiesDlgH_
#define __FilePropertiesDlgH_

/*	FilePropertiesDlg
The File Properties Dialog Box.  This Stretchy Control has everything set up for you.  Classes
that inherit from FilePropertiesDlg can set the member m_scriptName as the full path to where
the script is for their dialog box.  the default one is in the FileManagement directory.
*/

enum FilePropertiesDlg_ButtonVars
{
	FilePropertiesDlg_NoButton,
	FilePropertiesDlg_Add,
	FilePropertiesDlg_Delete,
	FilePropertiesDlg_Previous,
	FilePropertiesDlg_Next,
	FilePropertiesDlg_OK,
	FilePropertiesDlg_Cancel,
	FilePropertiesDlg_Close,
};

//! Use these for the proper layer loading
enum FilePropertiesDlg_Layers
{
	FilePropertiesDlg_Up	=	Controls_Button_UnSelected,
	FilePropertiesDlg_Dn	=	Controls_Button_Selected,
	FilePropertiesDlg_UpRo	=	Controls_Button_MouseOver,
	FilePropertiesDlg_DnRo	=	Controls_Button_MouseOverDn,
	FilePropertiesDlg_UpDis	=	Controls_Button_MouseOverDn + 1,
	FilePropertiesDlg_DnDis	=	SkinControl_SubControl_Button_Disabled + 1,
};

#define FileProps_Color_ScrollBar	RGBA(25,73,76)
#define FileProps_Color_ScrollBarUp	RGBA(25,100,76)
#define FileProps_Color_ScrollBarDn	RGBA(25,125,76)
#define FileProps_Color_AssetList	RGBA(73,52,157)
#define FileProps_Color_WidthSlider	RGBA(25,52,52)
#define FileProps_Color_Add			RGBA(25,27,100)
#define FileProps_Color_Delete		RGBA(25,52,100)
#define FileProps_Color_PathEdit	RGBA(25,255,0)
#define FileProps_Color_NameEdit	RGBA(25,100,25)
#define FileProps_Color_AliasEdit	RGBA(25,150,25)
#define FileProps_Color_Icon		RGBA(25,225,27)
#define FileProps_Color_Previous	RGBA(25,174,25)
#define FileProps_Color_Next		RGBA(25,255,27)
#define FileProps_Color_OK			RGBA(25,125,100)
#define FileProps_Color_Cancel		RGBA(25,100,100)
#define FileProps_Color_Group		RGBA(222,232,0)

#define FileProperties_SkinRoot		"AssetManagement\\FileProps"

class FileButton;
class DirectoryLayout;
class FileManagementDLL FilePropertiesDlg : 
						public AutoSkinControl, 
						public Control_DragNDrop,
						public AssetButtonClient, 
						public FileButton_ViewMode_Parent
{
	friend FileCollection;
protected:
	//! My local skin
	Auto_StretchySkin*	m_skinLocal;

	//! Handles for internal Controls
	AssetListLayout*	m_layout;
	UtilLib_Edit*		m_pathEdit;
	UtilLib_Edit*		m_fileNameEdit;
	UtilLib_Edit*		m_aliasEdit;
	FileButton*			m_fileIcon;
	SkinControl_SubControl_ToggleButton*
						m_groupToggle;
	SkinControl_SubControl_Button *m_addButton;
	SkinControl_SubControl_Button *m_deleteButton;
	SkinControl_SubControl_Button *m_previousButton;
	SkinControl_SubControl_Button *m_nextButton;
	SkinControl_SubControl_Button *m_OKButton;
	SkinControl_SubControl_Button *m_cancelButton;
	
	//! The Text Item used for the PropsWidthSlider
	TextItem m_propsWidthSliderText;

	///////////////////////////////
	//! Dynamic variables
	Dynamic<int>	m_button;
	Dynamic<int>	m_group;

	Dynamic<float>	m_scrollBarVar;
	Dynamic<float>	m_scrollBarMin;
	Dynamic<float>	m_scrollBarMax;
	Dynamic<float>	m_scrollBarWidth;

	Dynamic<float>	m_widthBarVar;

	///////////////////////////////
	//! ScreenObjects we own for controls
	ScreenObject_BitmapFile	m_sliderBitmaps[4];
	ScreenObject_BitmapFile	m_widthBitmaps[4];

	FileCollection*	m_files;						//! the collection we are looking at now.
	FileProperties* m_currentFile;					//! The current file we are looking at
	void EditNewFile(	const char* p_newFile	);	//! Deals with all of the controls

	//! We make ourselves the client of a newly created button
	virtual void CreateNewAssetButton(SharedElement* p_sharedElement, AssetListLayout* p_layout);

	//! My AutoSkinControl Callbacks
	virtual void InitializeSubControls();

	//! This callback is called each time the skin changes
	virtual void SkinChangedCallback(char* newSkinPath);

	//! The FileProperties for theskin folder we are looking at
	FileProperties* m_skinFileProps;

public:
	FilePropertiesDlg();
	~FilePropertiesDlg();

	virtual void InitialiseWindow(void);
	virtual void DestroyWindow(void);
	virtual void DeferredDelete(void);

	/////////////////////////////////////////
	//! From DynamicListener
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	//! Use this to clear whatever is being seen and look at this new list
	void	EditTheseFiles(tList<FileProperties*>* p_fileList);

	//! Use this to programmatically cancel or commit changes to path, filename, and altName
	void CommitChanges();
	void CancelChanges();

	virtual void ReceiveDeferredMessage(unsigned ID1,unsigned ID2);

	//! Scripting Commands
	virtual bool ExecuteCommand(	char *FunctionName,						//! The string representation of the command to execute
									ScriptExecutionStack &InputParameters,	//! The set of input parameters to your function
									ScriptExecutionStack &OutputParameters,	//! You can use this to access variables as well
									char *&Error);							//! If you supported the command but failed for some reason
																			//! return a user string here

	//! FileButton_ViewMode_Parent
	virtual bool ShouldFileButtonExecuteOnDoubleClick(FileButton* fb)	{return false;}
	virtual bool ShouldFileButtonResizeOnParentChange(FileButton* fb)	{return false;}
	virtual bool ShouldFileButtonResizeOnViewModeChange(FileButton* fb)	{return false;}
	virtual bool DragNDrop_ShouldThisItemBeDragged(HWND Parent,HWND ItemInQuestion,HWND ItemDragStartedWith){return false;}
};

#endif //#ifndef __FilePropertiesDlgH_