// FileBin_Favorites.h
// Defines classes for file favorites (model) and file favorites tab UI (view/controller)
// Mike Watkins
// NewTek
// 11-10-2003

#ifndef __FILEBIN_FAVORITES_H__
#define __FILEBIN_FAVORITES_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Defines

#define FileBin_ScriptRoot		"FileBin\\Main"
#define FileBin_FavoritesFile	"Favorites.txt"

#define FILEBIN_PREFS_DVEDIR		"DVE Root Path"

#define FILEBIN_MYCOMPUTER			TL_GetString("FileBin::MyComputer","My Computer")

#define FILEBIN_DEFAULT_USER_STR	"Default"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! My global routines
namespace FileBin_DefaultFavorites
{
	FileManagementDLL void CreateDefaultFavoritePaths(void);
	FileManagementDLL const char* GetDefaultFavoritePath(unsigned short id);

	const unsigned char EFFECTS = 1;
	const unsigned char PROJECTS = 2;
	const unsigned char DDRPROJECTS = 3;
	const unsigned char CLIPS = 4;
	const unsigned char DVES = 5;
	const unsigned char CGPAGES = 6;
	const unsigned char BCPROJECTS = 7;
	const unsigned char UTILS = 8;
	const unsigned char MYDOCS = 9;
	const unsigned char DESKTOP = 10;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Class defns

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileManagementDLL FavoriteFileList : public DynamicListener,	//! I listen for folder deletions and alias changes
											public DynamicTalker	//! I talk about chages to my list
{
	//! Default favorites tints
public:
	static const unsigned FAV_EFFECTS_FOLDER_COLOR;
	static const unsigned FAV_MYPROJ_FOLDER_COLOR;
	static const unsigned FAV_MYCLIPS_FOLDER_COLOR;
	static const unsigned FAV_DVE_FOLDER_COLOR;
	static const unsigned FAV_MYCGPAGES_FOLDER_COLOR;
	static const unsigned FAV_MYBCPROJ_FOLDER_COLOR;
	static const unsigned FAV_UTILS_FOLDER_COLOR;
	static const unsigned FAV_MYDOCS_FOLDER_COLOR;
	static const unsigned FAV_DESKTOP_FOLDER_COLOR;
	//! Interface...
public:

	static FavoriteFileList* GetInstance(void);
	static void FreeInstance(void);
	
	//! C'tors/D'tor
	FavoriteFileList(void);
	virtual ~FavoriteFileList(void);

	//! Get the number of favorites
	unsigned FFL_GetNumElements(void) const { return (unsigned)m_list.NoItems(); }

	//! Find the index for a favorite item by path
	int FFL_FindPath(const char *szPath) const;		//! Returns -1 if not found

	//! Get the favorites path at a given index
	const char* operator[] (unsigned index) const;	//! Returns the path

	//! Add and Delete a favorite by path
	bool Add(const char* filename);
	void Delete(const char* filename);

	//! Delete a favorite by index
	void Delete(unsigned idx);

	//! Rearrange the order of two favorites in the list
	void FFL_RearrageItem(unsigned itemToMove, unsigned newIdx);

	//! Watch my favorites folders for alias change or deletion
	void DynamicCallback(long ID, char *String, void *args, DynamicTalker *ItemChanging);

	//! Get and set a favorite's alias by index
	const char* FFL_GetAliasAt(unsigned index) const;
	bool FFL_SetAliasAt(unsigned index, const char *szNewAlias);

	//! Populate a list of all the favorites
	void GetStringList(tList<const char*>* stringList) const;
	void GetTextItemList(tList<TextItem*>* stringList) const;

	//! Serialization of the favorites list
	bool FFL_ReadList(void);
	void FFL_WriteList(void) const;

	//! Add a default favorite item with specified alias and tinting
	bool AddDefault(const char* filename, const char* alias=NULL, const unsigned* tintColor=NULL, bool bForceProps=false);

	//! Implementation
protected:

	//! Clear the favorites list
	void FFL_ClearList(void);

	//! Get the path to the favorites file
	void FFL_GetPersistPath(char favoritesFile[]) const;

	//! Members
protected:

	//! The list of favorites
	tList<FileProperties*> m_list;

	//! The singelton
	static FavoriteFileList *ms_pInst;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct IFileBin_FavoriteTab_Parent
{
	virtual bool FavoritesTab_RightClicked(TextButton *pTab, long Flags, long x, long y) = 0;
};

class FileManagementDLL FileBin_FavoritesList_TabView : public StringButtonSelection_Layout,
														public IFileBin_FavoriteTab_Parent
{
	//! Types
	typedef StringButtonSelection_Layout _superWndClass;

	//! Interface
public:
	//! C'tors/D'tor
	FileBin_FavoritesList_TabView(void);
	virtual ~FileBin_FavoritesList_TabView(void);

	//! Set and get the instance data members for the favorite file list model, and the directory layout
	void InitializeFileBinData(FavoriteFileList *pFavFileListModel, DirectoryLayout *pDirectoryLayout);
	FavoriteFileList* GetFavoriteFileList(void) const { return m_pFavFileListModel; }
	DirectoryLayout* GetDirectoryLayout(void) const { return m_pDirectoryLayout; }

	//! DynamicListener
	virtual void DynamicCallback(long ID, char *String, void *args, DynamicTalker *ItemChanging);

	//! Control_DragNDrop
	/*! If you return anything other than NULL for this value, then the window returned is the only possible
		drag target of items dragged out of this window. A good use for this is to ensure that DnD is allowed
		inside your own window, but the items cannot be dragged to others */
	virtual HWND DragNDrop_GetTargetDnDWindowForChildren(void) { return this->GetWindowHandle(); }
	virtual bool DragNDrop_ShouldThisItemBeDragged(HWND Parent, HWND ItemInQuestion, HWND ItemDragStartedWith);
	virtual bool DragNDrop_CanItemBeDroppedHere(HWND hWnd, Control_DragNDrop_DropInfo *Dropped);
	
	//! Drop the items here. 
	//! You must merge the list of existing children with the list of items being dropped.
	//! and return a linked list as your result. You must insert all items from dropped into
	//! your returned list. The "dropped" list is only items that pass your overloaded
	//! DragNDrop_CanItemBeDroppedHere test.
	virtual WindowLayout_Item *DragNDrop_DropItemsHere
								(	int Width, int Height,				//! Window size
									int MousePosnX, int MousePosnY,		//! Mouse posn
									WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
									WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
									bool Resizing, bool DroppedHere
								);

	//! IFileBin_FavoriteTab_Parent interface
	virtual bool FavoritesTab_RightClicked(TextButton *pTab, long Flags, long x, long y);

	//! Facilitator functions
protected:
	//! StringButtonSelection_Layout
	virtual HWND SBSL_CreateNewButton();

	//! Get the order index of the tab button
	virtual int GetTabIndex(TextButton *pTab);
	//! Update the view of the file favorites list
	virtual void HandleFileFavoritesListModelChange(void);
	//! Update the selected tab based on the current directory retrieved from the directory layout
	virtual void UpdateTabSelection(void);

	//! Instance data members
protected:
	FavoriteFileList *m_pFavFileListModel;	//! The file favorites model object
	DirectoryLayout *m_pDirectoryLayout;	//! The directory layout object

	int m_iStartDragIndex;
	int m_iEndDragIndex;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward decls
class StretchyFileBin;

class FileManagementDLL FileBin_FavoritesList_TabPanel : public AutoSkinControl,			// !I'm an autoskincontrol
															public BaseLoadSave_UiConfig	//! I don't want any persistance
{
	//! Types
	typedef AutoSkinControl _superWndClass;

	//! Interface
public:
	FileBin_FavoritesList_TabPanel(void);
	virtual ~FileBin_FavoritesList_TabPanel(void);

	//! BaseLoadSave_UiConfig
	virtual bool BaseLoadSave_UiConfig_ShouldISaveOrLoad(void) { return false; }
	virtual bool Persitance_ShouldBeUsed_Load(void) { return false; }
	virtual bool Persitance_ShouldBeUsed_Save(void) { return false; }

	//! AutoSkinControl
	virtual void InitialiseWindow(void);
	virtual void DestroyWindow(void);

	//! Scripting
	virtual bool ExecuteCommand(char *FunctionName, ScriptExecutionStack &InputParameters,
								ScriptExecutionStack &OutputParameters,	char *&Error);

	//! Implementation
protected:

	//! Instance data members
	Auto_StretchySkin *m_pSkin;	//! The skin
	ScreenObject_FromScript m_FavoriteTabScreenObjs[4];	//! The screen objects for the tabs
	//The tabs text color loaded from script
	TextItem m_TabTextItem;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//! Commands : FavoriteFileList

class FileManagementDLL Scripting_FavoriteFileList : public Script_Interface_GlobalCommand
{	
public:			
	virtual bool ExecuteCommand(char *FunctionName,	
								ScriptExecutionStack &InputParameters,
								ScriptExecutionStack &OutputParameters,
								char *&Error);

protected:
	void AddFavFolderCmd(const char *szPath, const char *szAlias, const char *szExt, const char *szSkinRootKey);

public:
	static const char* GetDefaultPath(const tList<char*> *pExtList, const char *szSkinRoot=NULL);
	
	static void OnProcessDetach(void);

protected:
	struct DefaultPathKey
	{
		char* m_szFileExt;
		char* m_szSkinRoot;
	};
	static tList<DefaultPathKey> *ms_pDefaultPathKeys;
	static tList<char*> *ms_pDefaultPaths;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif __FILEBIN_FAVORITES_H__