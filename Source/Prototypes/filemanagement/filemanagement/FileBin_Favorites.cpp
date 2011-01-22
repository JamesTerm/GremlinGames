// FileBin_Favorites.h
// Defines classes for file favorites (model) and file favorites tab UI (view/controller)
// Mike Watkins
// NewTek
// 11-10-2003

//! Includes
#include "stdafx.h"
#include "shlobj.h"
//! Defines
#define Str_FileFavorites_RenameTab_MsgBoxTitle		TL_GetString("T3:FileFavorites:RenameTab:MsgBoxTitle", "Enter New Text")
#define Str_FileFavorites_RenameTab_MenuItem		TL_GetString("T3:FileFavorites:RenameTab:MenuItem", "Rename Tab...")	
#define Str_FileFavorites_RemoveTab_MenuItem		TL_GetString("T3:FileFavorites:RemoveTab:MenuItem", "Remove Tab")

//! Class factories for header classes
NewTek_RegisterPluginControl(FileBin_FavoritesList_TabView)
NewTek_RegisterPluginControl(FileBin_FavoritesList_TabPanel)

//! Class defns
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBin_DefaultFavoritesImpl
{
public:
	FileBin_DefaultFavoritesImpl()
	{
		m_szEffects[0] = m_szProjects[0] = m_szDDRProjects[0] = m_szClips[0] = m_szCGPages[0] =	m_szBCProjects[0] = m_szUtils[0] = m_szMyDocs[0] = m_szDesktop[0] = 0;
	}

	const char* GetDefaultFavoritePath(unsigned char id)
	{
		if (id == FileBin_DefaultFavorites::DVES)
		{
			DWORD dwSucceeded =	::GetModuleFileName(NULL, m_szDVEs, MAX_PATH);
				if (dwSucceeded)
				{
					::strcpy(::NewTek_GetLastSlash(m_szDVEs), "\\DVEs");
				}
			return m_szDVEs;
			// Changed for new DVE location ProgDir\DVEs
			//return Preferences::GetValue_String(FILEBIN_PREFS_DVEDIR, "\\DVEs\\");
		}
		else if (id == FileBin_DefaultFavorites::EFFECTS)
		{
			if (0 == m_szEffects[0])
			{
				DWORD dwSucceeded =	::GetModuleFileName(NULL, m_szEffects, MAX_PATH);
				if (dwSucceeded)
				{
					::strcpy(::NewTek_GetLastSlash(m_szEffects), "\\Effects");
				}
			}
			
			return m_szEffects;
		}
		else if (id == FileBin_DefaultFavorites::CGPAGES)
		{
			if (0 == m_szCGPages[0])
			{
				DWORD dwSucceeded =	::GetModuleFileName(NULL, m_szCGPages, MAX_PATH);
				if (dwSucceeded)
				{
					::strcpy(::NewTek_GetLastSlash(m_szCGPages), "\\Effects\\Titles");
				}
			}
			
			return m_szCGPages;
		}
		else if (id == FileBin_DefaultFavorites::UTILS)
		{
			if (0 == m_szUtils[0])
			{
				DWORD dwSucceeded =	::GetModuleFileName(NULL, m_szUtils, MAX_PATH);
				if (dwSucceeded)
				{
					::strcpy(::NewTek_GetLastSlash(m_szUtils), "\\Utilities");
				}
			}

			return m_szUtils;
		}
		else
		{
			char *szBuffer = NULL;
			char szPathEnd[MAX_PATH] = {0};
			//char *szUserAccessable = "";
			if (id == FileBin_DefaultFavorites::PROJECTS)
			{
				szBuffer = m_szProjects;
				::NewTek_GetUserDataFolder(szBuffer, eUserDataPurpose_Accessible);
				NewTek_FixFilename( szBuffer, szBuffer );
				::strcat(szBuffer,"\\My Projects");
				if (szBuffer && !::NewTek_DoesFolderExist(szBuffer))
				{
					::NewTek_CreateDirectoryPath(szBuffer);
				}
			}
			else if (id == FileBin_DefaultFavorites::CLIPS)
			{
				szBuffer = m_szClips;
				::strcpy(szPathEnd, "\\Clips");
			}
			else if (id == FileBin_DefaultFavorites::DDRPROJECTS)
			{
				szBuffer = m_szDDRProjects;
				::strcpy(szPathEnd, "\\DDR Projects");
			}
			else if (id == FileBin_DefaultFavorites::BCPROJECTS)
			{
				szBuffer = m_szBCProjects;
				::strcpy(szPathEnd, "\\Batch Projects");
			}
			/* 
			else if (id == FileBin_DefaultFavorites::CGPAGES)
			{
				szBuffer = m_szCGPages;
				::strcpy(szPathEnd, "\\CG Pages");
			}
			*/
			// John hacking away - God help us...
			else if (id == FileBin_DefaultFavorites::MYDOCS)
			{
				LPITEMIDLIST pidl;
				HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_MYDOCUMENTS, &pidl);
				if (hRes==NOERROR)
				{
					SHGetPathFromIDList( pidl, m_szMyDocs );
					szBuffer = m_szMyDocs;
				}
			}
			else if (id == FileBin_DefaultFavorites::DESKTOP)
			{
				LPITEMIDLIST pidl;
				HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);
				if (hRes==NOERROR)
				{
					SHGetPathFromIDList( pidl, m_szDesktop );
					szBuffer = m_szDesktop;
				}
			}
			else 

			if (szBuffer && 0==szBuffer[0])
			{
				// Get the paths to the app file folders created on the video drive
				if (::GetPrivateProfileString("VideoToaster", "VideoDrive", "NoVideoDrive", szBuffer, MAX_PATH, "WMSysPr85.prx"))
				{
					// Added for BlackBox.  Everybody has a C: drive on Windows
					if (0 == ::strcmp("NoVideoDrive", szBuffer))
						::strcpy(szBuffer, "C:\\");
					
                    ::strcat(szBuffer, DEFAULT_FILES_PATH );
					//::strcat(szBuffer, "NewTek Files\\");	// App independence, Nah...other apps should use scripting!!!)
					::strcat(szBuffer, ::NewTek_GetUserName());
					::strcat(szBuffer, szPathEnd);
				}
			}

			return szBuffer;
				
		}
	}

private:
	char m_szEffects[MAX_PATH];
	char m_szDVEs[MAX_PATH];
	char m_szProjects[MAX_PATH];
	char m_szDDRProjects[MAX_PATH];
	char m_szClips[MAX_PATH];
	char m_szCGPages[MAX_PATH];
	char m_szBCProjects[MAX_PATH];
	char m_szUtils[MAX_PATH];
	char m_szMyDocs[MAX_PATH];
	char m_szDesktop[MAX_PATH];
}
g_DefaultFileFavorites;

// Had to add this to handle case where user file favorites file still exists, the default folders are ref'ed in the file
// favorites, and the default folders have been deleted, and the build number has been incremented.  Otherwise when the
// new build is run the default folders will show up in the file bin tabs w/ their full path names.
void FileBin_DefaultFavorites::CreateDefaultFavoritePaths(void)
{
	const char *szPath = NULL;
	szPath = GetDefaultFavoritePath(FileBin_DefaultFavorites::PROJECTS);
	if (szPath && !::NewTek_IsDrive(szPath) && !::NewTek_DoesFolderExist(szPath))
	{
		::NewTek_CreateDirectoryPath(szPath);

		FileProperties* thisFile = FileProperties::GetHandle(szPath);
		if (thisFile)
		{
			if (FileProperties_Directory==thisFile->GetFileStatus())
			{
				thisFile->FileChangeAltName(Str_FileBin_Favorites_MyProjects);
				thisFile->SetTintColor(FavoriteFileList::FAV_MYPROJ_FOLDER_COLOR);
			}
			FileProperties::ReleaseHandle(thisFile);			
		}
	}

	szPath = GetDefaultFavoritePath(FileBin_DefaultFavorites::CLIPS);
	if (szPath && !::NewTek_IsDrive(szPath) && !::NewTek_DoesFolderExist(szPath))
	{
		::NewTek_CreateDirectoryPath(szPath);

		FileProperties* thisFile = FileProperties::GetHandle(szPath);
		if (thisFile)
		{
			if (FileProperties_Directory==thisFile->GetFileStatus())
			{
				thisFile->FileChangeAltName(Str_FileBin_Favorites_MyClips);
				thisFile->SetTintColor(FavoriteFileList::FAV_MYCLIPS_FOLDER_COLOR);
			}
			FileProperties::ReleaseHandle(thisFile);			
		}
	}

	szPath = GetDefaultFavoritePath(FileBin_DefaultFavorites::DDRPROJECTS);
	if (szPath && !::NewTek_IsDrive(szPath) && !::NewTek_DoesFolderExist(szPath))
	{
		::NewTek_CreateDirectoryPath(szPath);
	}

	szPath = GetDefaultFavoritePath(FileBin_DefaultFavorites::BCPROJECTS);
	if (szPath && !::NewTek_IsDrive(szPath) && !::NewTek_DoesFolderExist(szPath))
	{
		::NewTek_CreateDirectoryPath(szPath);

		FileProperties* thisFile = FileProperties::GetHandle(szPath);
		if (thisFile)
		{
			if (FileProperties_Directory==thisFile->GetFileStatus())
			{
				thisFile->FileChangeAltName(Str_FileBin_Favorites_MyBCProjects);
				thisFile->SetTintColor(FavoriteFileList::FAV_MYBCPROJ_FOLDER_COLOR);
			}
			FileProperties::ReleaseHandle(thisFile);			
		}
	}

	szPath = GetDefaultFavoritePath(FileBin_DefaultFavorites::CGPAGES);
	if (szPath && !::NewTek_IsDrive(szPath) && !::NewTek_DoesFolderExist(szPath))
	{
		::NewTek_CreateDirectoryPath(szPath);

		FileProperties* thisFile = FileProperties::GetHandle(szPath);
		if (thisFile)
		{
			if (FileProperties_Directory==thisFile->GetFileStatus())
			{
				thisFile->FileChangeAltName(Str_FileBin_Favorites_MyCGPages);
				thisFile->SetTintColor(FavoriteFileList::FAV_MYCGPAGES_FOLDER_COLOR);
			}
			FileProperties::ReleaseHandle(thisFile);			
		}
	}
}

const char* FileBin_DefaultFavorites::GetDefaultFavoritePath(unsigned short id)
{
	//We should probably have FileBin_DefaultFavoritesImpl use unsigned short for this parameter as well -James
	return g_DefaultFileFavorites.GetDefaultFavoritePath((unsigned char)id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static 
FavoriteFileList* FavoriteFileList::ms_pInst = NULL;

//static 
FavoriteFileList* FavoriteFileList::GetInstance(void)
{
	if (!ms_pInst)
	{
		ms_pInst = new FavoriteFileList;
		ms_pInst->FFL_ReadList();
	}
	return ms_pInst;
}
//static 
void FavoriteFileList::FreeInstance(void)
{
	delete ms_pInst;
	ms_pInst = NULL;
}

// Def colors for def HotList items
/*static */const unsigned FavoriteFileList::FAV_EFFECTS_FOLDER_COLOR	= 0xffFF7F7F;
/*static */const unsigned FavoriteFileList::FAV_MYPROJ_FOLDER_COLOR		= 0xff749EFF;
/*static */const unsigned FavoriteFileList::FAV_MYCLIPS_FOLDER_COLOR	= 0xffFAFFC3;
/*static */const unsigned FavoriteFileList::FAV_DVE_FOLDER_COLOR		= 0xff54F9FF;
/*static */const unsigned FavoriteFileList::FAV_MYCGPAGES_FOLDER_COLOR	= 0xff99FFBF;
/*static */const unsigned FavoriteFileList::FAV_MYBCPROJ_FOLDER_COLOR	= 0xffF499FF;
/*static */const unsigned FavoriteFileList::FAV_UTILS_FOLDER_COLOR		= 0xffFF0505;
/*static */const unsigned FavoriteFileList::FAV_MYDOCS_FOLDER_COLOR		= 0xffFF7F7F;
/*static */const unsigned FavoriteFileList::FAV_DESKTOP_FOLDER_COLOR	= 0xffFF7F7F;
////////////////////////////////////////////////////////////////////////////////////////////

FavoriteFileList::FavoriteFileList(void)
{	
}

////////////////////////////////////////////////////////////////////////////////////////////

FavoriteFileList::~FavoriteFileList(void)
{
	// Same as Clear list but don't want to send a changed msg
	for (unsigned i = 0; i < m_list.NoItems(); i++)
	{
		m_list[i]->DeleteDependant(this);
		FileProperties::ReleaseHandle(m_list[i]);
		m_list[i] = NULL;
	}
	m_list.DeleteAll();
}

////////////////////////////////////////////////////////////////////////////////////////////

int FavoriteFileList::FFL_FindPath(const char *szPath) const
{
	// Precondition
	if (!szPath) 
		return -1;

	int iRetVal = -1;

	FileProperties* thisFile = FileProperties::GetHandle(szPath);
	iRetVal = m_list.GetPosition(thisFile);
	FileProperties::ReleaseHandle(thisFile);

	return iRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////

const char* FavoriteFileList::operator[] (unsigned index) const
{
	// Preconditions
	assert(index < m_list.NoItems());
	if (index >= m_list.NoItems()) 
		return NULL;

	return (m_list[index]->GetFileName());
}

////////////////////////////////////////////////////////////////////////////////////////////

bool FavoriteFileList::Add(const char* filename)
{
	if (!filename || 0==filename[0]) 
		return false;

	FileProperties* thisFile = FileProperties::GetHandle(filename);
	if (!thisFile)
		return false;
	
	if (thisFile->GetFileStatus() != FileProperties_NoFile)
	{
		if (!m_list.Exists(thisFile))
		{
			thisFile->AddDependant(this, (long)thisFile);
			m_list.Add(thisFile);

			this->Changed();
			
			return true;
		}
	}

	FileProperties::ReleaseHandle(thisFile);
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::Delete(const char* filename)
{
	if (!filename) 
		return;

	for (unsigned i = 0; i < m_list.NoItems(); i++)
	{
		if (0 == ::strcmp(filename, m_list[i]->GetFileName()))
		{
			m_list[i]->DeleteDependant(this);
			FileProperties::ReleaseHandle(m_list[i]);
			m_list.DeleteEntryInOrder(i);

			this->Changed();
			
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::Delete(unsigned idx)
{
	// Preconditions
	assert(idx < m_list.NoItems());
	if (idx < m_list.NoItems())
	{
		m_list.DeleteEntryInOrder(idx);
		this->Changed();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::FFL_RearrageItem(unsigned itemToMove, unsigned newIdx)
{
	// Precondition
	assert(itemToMove<m_list.NoItems() && newIdx<m_list.NoItems());
	if (itemToMove<m_list.NoItems() && newIdx<m_list.NoItems())
	{
		if (itemToMove != newIdx)
		{
			FileProperties *pPropToMove = m_list[itemToMove];
			m_list.DeleteEntryInOrder(itemToMove);
			m_list.AddAtPosition(pPropToMove, newIdx);

			this->Changed();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::DynamicCallback(long ID, char *String, void *args, DynamicTalker *ItemChanged)
{
	// Watching for the file being deleted
	if ((0 == ::strcmp(String, FileProperties_Changed_FileDeletedInfo)) ||
		(0 == ::strcmp(String, FileProperties_Changed_FileDeletedNotInfo)))
	{
		for (unsigned i = 0; i < m_list.NoItems(); i++)
		{
			if (ID == (long)(m_list[i]))
			{
				m_list[i]->DeleteDependant(this);
				FileProperties::ReleaseHandle(m_list[i]);
				m_list.DeleteEntryInOrder(i);
				this->Changed();
				return;
			}
		}
	}
	// Watching for the file alias to change
	else if (0 == ::strcmp(String, FileProperties_Changed_AltName))
	{
		for (unsigned i = 0; i < m_list.NoItems(); i++)
		{
			if (ID == (long)(m_list[i]))
			{
				this->Changed();
				return;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

const char* FavoriteFileList::FFL_GetAliasAt(unsigned index) const
{
	// Precondition
	assert(index < m_list.NoItems());
	if (index >= m_list.NoItems())
		return NULL;

	const char *szRetVal = m_list[index]->GetAltFilename();
	if (!szRetVal || !szRetVal[0])
		szRetVal = m_list[index]->GetFileName();

	return szRetVal;
}

////////////////////////////////////////////////////////////////////////////////////////////

bool FavoriteFileList::FFL_SetAliasAt(unsigned index, const char *szNewAlias)
{
	// Precondition
	assert(index < m_list.NoItems());
	if (index >= m_list.NoItems())
		return false;

	m_list[index]->FileChangeAltName(szNewAlias);	// My dynamic callback will get called and I'll talk from there
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::GetStringList(tList<const char*>* stringList) const
{
	// Precondition
	if (!stringList) 
		return;

	for (unsigned i = 0; i < m_list.NoItems(); i++)
	{
		const char* altName = m_list[i]->GetAltFilename();
		if (altName && altName[0])
			stringList->Add(altName);
		else
			stringList->Add(m_list[i]->GetFileName());
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::GetTextItemList(tList<TextItem*>* stringList) const
{	
	// Precondition
	if (!stringList) 
		return;
	
	for (unsigned i = 0; i < m_list.NoItems(); i++)
	{	
		TextItem *TI=new TextItem;
		TI->SetColor(m_list[i]->GetTintColor());
//		TI->SetColor(DEFAULT_FG_COLOR);
		stringList->Add(TI);
		const char* altName = m_list[i]->GetAltFilename();
		if (altName && altName[0])
			TI->SetText(altName);
		else	
			TI->SetText(m_list[i]->GetFileName());
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

bool FavoriteFileList::FFL_ReadList(void)
{
	this->FFL_ClearList();
	
	// Read in the saved favorites from the file
	// Build the filename
	char favoritesFile[MAX_PATH*2] = { 0 };
	this->FFL_GetPersistPath(favoritesFile);
	
	// Block individaul add msgs
	this->MessagesOff();

	// Read each element from the File
	FILE* readFile = ::fopen(favoritesFile, "r");
	if (readFile)
	{
		// Use the old favoritesFile as our buffer
		char* buffer = favoritesFile;
		
		// Read in the number of elements
		int numElements = 0;
		if (::fgets(buffer, MAX_PATH*2, readFile))
			::sscanf(buffer, "%i", &numElements);
		
		// Loop through each element and add the names to the list
		bool eof = false;
		for (int i = 0; i < numElements && !eof; i++)
		{
			if (::fgets(buffer, MAX_PATH*2, readFile))
			{
				// Rip the extra characters off the end if they exist
				char* endString = ::strrchr(buffer, 10);
				if (endString) 
					*endString = 0;
				
				// Add the file to the favorites list
				this->Add(buffer);
			}
			else 
				eof = true;
		}
		
		// Close the File
		::fclose(readFile);
	}
	else if (ApplicationInstance_Default())	// Only for VT[4]
	{
		// This will be handled in scripts from now on!
		const char *szPath = NULL;

		// Add the hard-coded paths (Effects, Clips, Projects) to the favorites list
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::MYDOCS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_MyDocs, &FAV_MYDOCS_FOLDER_COLOR);
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::DESKTOP);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_Desktop, &FAV_DESKTOP_FOLDER_COLOR);
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::PROJECTS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_MyProjects, &FAV_MYPROJ_FOLDER_COLOR);
		
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::CLIPS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_MyClips, &FAV_MYCLIPS_FOLDER_COLOR);
		
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::CGPAGES);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_MyCGPages, &FAV_MYCGPAGES_FOLDER_COLOR);
		
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::BCPROJECTS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_MyBCProjects, &FAV_MYBCPROJ_FOLDER_COLOR);
		
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::EFFECTS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_Effects, &FAV_EFFECTS_FOLDER_COLOR);
		
		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::DVES);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_DVE, &FAV_DVE_FOLDER_COLOR);

		szPath = FileBin_DefaultFavorites::GetDefaultFavoritePath(FileBin_DefaultFavorites::UTILS);
		if (szPath)
			this->AddDefault(szPath, Str_FileBin_Favorites_Utils, &FAV_UTILS_FOLDER_COLOR);
		
	}

	// Unblock msgs
	this->MessagesOn();

	// Send changed msg
	this->Changed();
	
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::FFL_WriteList(void) const
{
	
	// write out the saved favorites to the file...
	
	// Build the filename
	char favoritesFile[MAX_PATH*2] = { 0 };
	this->FFL_GetPersistPath(favoritesFile);
	
	// write each element from the File
	FILE* writeFile = ::fopen(favoritesFile, "w");
	if (writeFile)
	{		
		// write in the number of elements to save
		::fprintf(writeFile, "%i\n", (int)this->FFL_GetNumElements());
		
		// Loop through each element and add the names to the list
		bool eof = false;
		for (unsigned i=0; i<this->FFL_GetNumElements() && !eof; i++)
		{
			::fprintf(writeFile, "%s\n", (*this)[i]);
		}
		
		// Close the File
		::fclose(writeFile);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::FFL_GetPersistPath(char favoritesFile[]) const
{
	// We want to use the Users Name
	char *UserName=::NewTek_GetUserName();
	if (!UserName) 
		UserName = FILEBIN_DEFAULT_USER_STR;

	//::sprintf(favoritesFile, "%s\\%s_%s", ::FindFiles_FindFile(FINDFILE_SKINS,FileBin_ScriptRoot), UserName, FileBin_FavoritesFile);
	::NewTek_GetUserDataFolder(favoritesFile, eUserDataPurpose_Default);
	if ( ! ApplicationInstance_LiveText() )		// Konno
		::strcat(favoritesFile, __BUILD_NAME_STRING__);		// mwatkins - THIS REALLY has to be done!!!  DO NOT REMOVE!!!
	::strcat(favoritesFile, FileBin_FavoritesFile);

	if (ApplicationInstance_TriCaster())
	{
		if (!::NewTek_DoesFileExist(favoritesFile))
		{
			if (500 < NewTek_GetVideoFormat().m_YRes) // Am I PAL boy???
			{
				::sprintf(favoritesFile, "%s\\(%s)%s_%s", ::FindFiles_FindFile(FINDFILE_SKINS,FileBin_ScriptRoot), "PAL", UserName, FileBin_FavoritesFile);
			}
			else
			{
				::sprintf(favoritesFile, "%s\\(%s)%s_%s", ::FindFiles_FindFile(FINDFILE_SKINS,FileBin_ScriptRoot), "NTSC", UserName, FileBin_FavoritesFile);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

void FavoriteFileList::FFL_ClearList(void)
{
	
	for (unsigned i = 0; i < m_list.NoItems(); i++)
	{
		m_list[i]->DeleteDependant(this);
		FileProperties::ReleaseHandle(m_list[i]);
		m_list[i] = NULL;
	}
	m_list.DeleteAll();

	this->Changed();
}

////////////////////////////////////////////////////////////////////////////////////////////

bool FavoriteFileList::AddDefault(const char* filename, const char* alias, const unsigned* tintColor, bool bForceProps)
{
	if (!filename || 0==filename[0]) 
		return false;

	FileProperties* thisFile = FileProperties::GetHandle(filename);
	if (!thisFile)
		return false;

	bool bRetVal = false;
	if (thisFile->GetFileStatus() != FileProperties_NoFile)
	{
		if (!m_list.Exists(thisFile))
		{
			if (alias)
			{
				// Add an alias to the file props if one doesn't exist (or if we're forcing things)
				const char* oldAlias = thisFile->GetAltFilename();
				if (!oldAlias || !oldAlias[0] || bForceProps)
				{
					thisFile->FileChangeAltName(alias);
					if (tintColor)
						thisFile->SetTintColor(*tintColor);
				}
			}

			thisFile->AddDependant(this, (long)thisFile);
			m_list.Add(thisFile);

			this->Changed();
			
			return true;
		}
		else if (bForceProps)
		{
			this->ReceiveMessagesOff();
			if (alias && *alias)
				thisFile->FileChangeAltName(alias);
			if (tintColor)
				thisFile->SetTintColor(*tintColor);
			this->ReceiveMessagesOn();

			this->Changed();

			bRetVal = true;

			// Already have one handle on the file props, so fall through to dec ref count
		}
	}

	FileProperties::ReleaseHandle(thisFile);
	return bRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FileBin_FavoriteTab : public TextButton
{
	typedef TextButton _superWndClass;
public:
	virtual void MouseRButtonRelease(long Flags, long x, long y);
};
NewTek_RegisterPluginControl(FileBin_FavoriteTab);

void FileBin_FavoriteTab::MouseRButtonRelease(long Flags, long x, long y)
{
	IFileBin_FavoriteTab_Parent *pParent = GetInterface<IFileBin_FavoriteTab_Parent>(this->GetParentBWC());
	if (pParent)
	{
		if (pParent->FavoritesTab_RightClicked(this, Flags, x, y))
			return;
	}

	_superWndClass::MouseRButtonRelease(Flags, x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBin_FavoritesList_TabView::FileBin_FavoritesList_TabView(void) : 
m_pFavFileListModel(NULL), m_pDirectoryLayout(NULL), m_iStartDragIndex(-1), m_iEndDragIndex(-1)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBin_FavoritesList_TabView::~FileBin_FavoritesList_TabView(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabView::InitializeFileBinData(FavoriteFileList *pFavFileListModel, DirectoryLayout *pDirectoryLayout)
{
	// Precondition: Assert we haven't been called before
	assert(!m_pFavFileListModel && !m_pDirectoryLayout);
	if (m_pFavFileListModel || m_pDirectoryLayout)
		return;

	// Set my data members
	m_pFavFileListModel = pFavFileListModel;
	m_pDirectoryLayout = pDirectoryLayout;

	// Listen to them
	pFavFileListModel->AddDependant(this);
	pDirectoryLayout->AddDependant(this);

	// Listen to myself for drag messages
	this->AddDependant(this);

	// Initialize my view from the list model
	this->HandleFileFavoritesListModelChange();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabView::DynamicCallback(long ID, char *String, void *args, DynamicTalker *ItemChanging)
{
	// Listen for deletion (prevent dangling ptrs)
	if (::IsDeletion(String))
	{
		if (ItemChanging == m_pFavFileListModel)
		{
			m_pFavFileListModel = NULL;
			return;
		}
		if (ItemChanging == m_pDirectoryLayout)
		{
			m_pDirectoryLayout = NULL;
			return;
		}
	}
	// Listen for my drag messages
	else if (ItemChanging == this)
	{
		if (0 == ::strcmp(DragNDropContext_DragStarted, String))
		{
			m_iStartDragIndex = m_iEndDragIndex = -1;
		}
		else if (0 == ::strcmp(DragNDropContext_DragFinished, String))
		{
			if (this->GetFavoriteFileList() && 
				(0 <= m_iStartDragIndex) && (0 <= m_iEndDragIndex) && (m_iStartDragIndex != m_iEndDragIndex))
			{
				this->GetFavoriteFileList()->FFL_RearrageItem((unsigned)m_iStartDragIndex, (unsigned)m_iEndDragIndex);
			}
		}

		return;
	}
	// Listen for favorite list model changes
	else if (ItemChanging == m_pFavFileListModel)
	{
		this->HandleFileFavoritesListModelChange();
		return;
	}
	// Update the selection if the current directory changes
	else if (ItemChanging == m_pDirectoryLayout)
	{
		if (0 == ::strcmp(DirectoryLayout_DirectoryChanged, String))
			this->UpdateTabSelection();

		return;
	}
	// Update the current directory if the selected tab changes
	else if ((ItemChanging == m_dynamicToUse) || (ItemChanging == &m_internalDynamic))
	{
		// My superclass is the original dependant
		_superWndClass::DynamicCallback(ID, String, args, ItemChanging);

		if (m_dynamicToUse && (0 <= m_dynamicToUse->Get()) && m_pDirectoryLayout)
		{
			unsigned newFavDirIdx = (unsigned)m_dynamicToUse->Get();
			this->ReceiveMessagesOff();
			m_pDirectoryLayout->SetDirectory((*m_pFavFileListModel)[newFavDirIdx]);
			this->ReceiveMessagesOn();
		}

		return;
	}

	_superWndClass::DynamicCallback(ID, String, args, ItemChanging);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FileBin_FavoritesList_TabView::DragNDrop_ShouldThisItemBeDragged(HWND Parent, HWND ItemInQuestion, HWND ItemDragStartedWith)
{
	// This item should be dragged if I'm the parent window (Sanity check)
	// and force only one drag item at a time
	if (Parent == this->GetWindowHandle() && ItemInQuestion == ItemDragStartedWith)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FileBin_FavoritesList_TabView::DragNDrop_CanItemBeDroppedHere(HWND hWnd, Control_DragNDrop_DropInfo *Dropped)
{
	return (NULL != ::GetWindowInterface<FileBin_FavoriteTab>(Dropped->hWnd));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
WindowLayout_Item* FileBin_FavoritesList_TabView::DragNDrop_DropItemsHere
								(	int Width, int Height,				//! Window size
									int MousePosnX, int MousePosnY,		//! Mouse posn
									WindowLayout_Item *ObjectsInWindow,	//! The objects already in the window
									WindowLayout_Item *ObjectsDropped,	//! The objects that have been dropped in the window
									bool Resizing, bool DroppedHere
								)
{
	// If the item hasn't been dropped
		// Call superclass implementation and return
	// Else
		// Use the button selection state to determine the original list index of the item dragged
		// Call the superclass implementation to layout
		// Find it's position in the layout item list

	if (!DroppedHere)
		return _superWndClass::DragNDrop_DropItemsHere(Width, Height, MousePosnX, MousePosnY, ObjectsInWindow, ObjectsDropped, Resizing, DroppedHere);
	
	m_iStartDragIndex = this->GetTabIndex(GetWindowInterface<TextButton>(ObjectsDropped->hWnd));
	assert(-1 != m_iStartDragIndex);	// Sanity check
	
	WindowLayout_Item *pRetVal = _superWndClass::DragNDrop_DropItemsHere(Width, Height, MousePosnX, MousePosnY, ObjectsInWindow, ObjectsDropped, Resizing, DroppedHere);

	int iScan = 0;
	WindowLayout_Item *pScan = pRetVal;
	m_iEndDragIndex = -1;
	while (pScan)
	{
		if (m_iStartDragIndex == this->GetTabIndex(GetWindowInterface<TextButton>(pScan->hWnd)))
		{
			m_iEndDragIndex = iScan;
			break;
		}
		else
		{
			pScan = pScan->Next;
			iScan++;
		}
	}
	assert(-1 != m_iEndDragIndex);
	
	return pRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FileBin_FavoritesList_TabView::FavoritesTab_RightClicked(TextButton *pTab, long Flags, long x, long y)
{
	// Preconditions
	if (!pTab)
		return false;
	if (!this->GetFavoriteFileList())
		return false;

	// Get the tab's index
	int tabIdx = this->GetTabIndex(pTab);

	// Create the menu
	tList<char*> menuItemStrs;
	menuItemStrs.Add(Str_FileFavorites_RemoveTab_MenuItem);
	menuItemStrs.Add(Str_FileFavorites_RenameTab_MenuItem);
	RECT menuRect = ::CreateMenu(&menuItemStrs);

	// Posn the menu
	POINT pt;
	::GetCursorPos(&pt);

	menuRect.left += pt.x;
	menuRect.right += pt.x;
	menuRect.top += pt.y;
	menuRect.bottom += pt.y;

	// Show the menu
	int menuRet = ::ShowMenu(menuRect);

	// Modify the favorites list based on the menu selection
	if (0 == menuRet)
	{
		this->GetFavoriteFileList()->Delete(tabIdx);
	}
	else if (1 == menuRet)
	{
		long reasonForExit;
		char* szNewName = ::NewTek_GetTextFromPopupWindow(Str_FileFavorites_RenameTab_MsgBoxTitle, pTab->GetText(), *this, reasonForExit);

		if (reasonForExit != Popup_Escape_EscapeKey)
		{
			if (szNewName && szNewName[0])
			{
				this->GetFavoriteFileList()->FFL_SetAliasAt(tabIdx, szNewName);
			}
		}
	}

	return true;	// I handled it
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HWND FileBin_FavoritesList_TabView::SBSL_CreateNewButton()
{
	// Base Class just makes a Text Button
	HWND ret = this->OpenChild("FileBin_FavoriteTab"/*"TextButton"*/, -200, -200);
	TextButton* tb = GetWindowInterface<TextButton>(ret);
	if (tb)
	{
		tb->TB_SetResize(true, false);
		tb->SetDisplayCaching(true);	// Turn on display caching since I added drag and drop and want to prevent flicker
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int FileBin_FavoritesList_TabView::GetTabIndex(TextButton *pTab)
{
	assert(pTab);
	if (!pTab)
		return -1;

	return pTab->Button_GetSelectedState()-1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabView::HandleFileFavoritesListModelChange(void)
{
	// Precondition
	if (!this->GetFavoriteFileList())
		return;

	// Get the favorite folder path list
	tList<const char*> favDirPathList;
	this->GetFavoriteFileList()->GetStringList(&favDirPathList);
			
	// mwatkins TODO: Hack this into a list that can be accepted for the tabs
	tList<char*> favTabStrList(favDirPathList.NoItems());
	for (unsigned i=0; i<favDirPathList.NoItems(); i++)
		favTabStrList[i] = const_cast<char*>(favDirPathList[i]);
			
	// Assigne the strings to the tabs
	this->SBS_AssignStrings(favTabStrList);

	// Update the tab selection
	this->UpdateTabSelection();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabView::UpdateTabSelection(void)
{
	// Precondition
	if (!this->GetFavoriteFileList() || !this->GetDirectoryLayout() || !m_dynamicToUse)
		return;

	// Get the path from the directory layout
	// Find the index for this path in the list model
	// Turn the corresponding tab button on
	this->ReceiveMessagesOff();
	m_dynamicToUse->Set(this->GetFavoriteFileList()->FFL_FindPath(this->GetDirectoryLayout()->GetDirectory()));
	this->ReceiveMessagesOn();
	
	_superWndClass::DynamicCallback(-1, "Changed", NULL, m_dynamicToUse);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBin_FavoritesList_TabPanel::FileBin_FavoritesList_TabPanel(void) : m_pSkin(NULL)
{
	// Set the skin root dir
	this->SetSkinRoot(::FindFiles_FindFile(FINDFILE_SKINS, "FileBin/TabbedArea"));
	// Set the Default params of the our new Text Item
	m_TabTextItem.SetColor(DEFAULT_FG_COLOR); /* use default color now */
	m_TabTextItem.SetAlignment(TextItem_CenterHAlign|TextItem_CenterVAlign);
	m_TabTextItem.SetFont(TextItem_DefaultFont);
	m_TabTextItem.SetItalic(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileBin_FavoritesList_TabPanel::~FileBin_FavoritesList_TabPanel(void)
{
	// Destroy my skin
	::NewTek_Delete(m_pSkin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabPanel::InitialiseWindow(void)
{
	// Set the Canvas resource/skin
	m_pSkin = ::GetInterface<Auto_StretchySkin>(::NewTek_New("Auto_StretchySkin"));
	if (!m_pSkin) 
		_throw("Cannot create the stretchy skin for the FileBin_FavFolderTabCtrl window!");

	this->Canvas_SetResource(m_pSkin);

	// Call superclass method
	_superWndClass::InitialiseWindow();

	// Create and initialize my controls
	FileBin_FavoritesList_TabView *pFavListTabView =
		::GetWindowInterface<FileBin_FavoritesList_TabView>(this->OpenChild(RGBA(0,63,207), "FileBin_FavoritesList_TabView"));
	if (pFavListTabView)
	{
		//This will set the color of the text applied to the tabs -James
		CopyPlugin(&m_TabTextItem,pFavListTabView->SBL_GetMasterTextItem());
		// Set the screen objects to be used for the tabs
		for (int i=0; i<(sizeof(m_FavoriteTabScreenObjs))/(sizeof(m_FavoriteTabScreenObjs[0])); i++)
		{	
			// Set each screen object up with the correct layer
			m_FavoriteTabScreenObjs[i].ChangeAllLayers(i);
			
			// Setup the screen object
			pFavListTabView->SBL_AssignScreenObjects(i , &m_FavoriteTabScreenObjs[i]);
		}
		
		// Create scroller/spinner for tabs
		JumpSpinner *pFavListTabViewScroller = ::GetWindowInterface<JumpSpinner>(this->OpenChild(RGBA(0,174,255), "JumpSpinner"));
		if (pFavListTabViewScroller) 
		{
			pFavListTabViewScroller->JS_SetLayout(pFavListTabView);
			
			// Set Init values for var,min,max,width
			pFavListTabViewScroller->Spinner_GetVariable()->Set(0.0f);
			pFavListTabViewScroller->Spinner_GetMinVariable()->Set(0.0f);
			pFavListTabViewScroller->Spinner_GetMaxVariable()->Set(1.0f);
			pFavListTabViewScroller->Spinner_GetSliderWidth()->Set(30.0f);
			
			pFavListTabView->BaseWindowLayoutManager_X_Slider_SetVariable(pFavListTabViewScroller->Spinner_GetVariable());
			pFavListTabView->BaseWindowLayoutManager_X_Slider_SetMinVariable(pFavListTabViewScroller->Spinner_GetMinVariable());
			pFavListTabView->BaseWindowLayoutManager_X_Slider_SetMaxVariable(pFavListTabViewScroller->Spinner_GetMaxVariable());
			pFavListTabView->BaseWindowLayoutManager_X_Slider_SetSliderWidth(pFavListTabViewScroller->Spinner_GetSliderWidth());
			
			pFavListTabViewScroller->SetAttr(UtilLib_Spinner_LeftRight);
		}

		StretchyFileBin *pParentFileBin = GetInterface<StretchyFileBin>(this->GetParentBWC());
		if (pParentFileBin)
			pFavListTabView->InitializeFileBinData(pParentFileBin->SFB_GetFavoriteFileList(), pParentFileBin->GetFileLayout());
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileBin_FavoritesList_TabPanel::DestroyWindow(void)
{
	// Call inherited method
	_superWndClass::DestroyWindow();

	// Destroy my skin
	::NewTek_Delete(m_pSkin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FileBin_FavoritesList_TabPanel::ExecuteCommand(char *FunctionName, ScriptExecutionStack &InputParameters,
													ScriptExecutionStack &OutputParameters,	char *&Error)
{
	if (0 == ::strcmp(FunctionName , "SetTabSkin"))
	{	
		// Ensure that the skin root is set up
		for (int i=0; i<(sizeof(m_FavoriteTabScreenObjs))/(sizeof(m_FavoriteTabScreenObjs[0])) ; i++)
		{	
			// Load the script
			Error = m_FavoriteTabScreenObjs[i].LoadFromFolder(InputParameters[0]);

			// If there was an error, stop loading
			if (Error) 
				return false;
		}

		return true;
	}

	if (!stricmp("SetTabTextItem", FunctionName))
	{	
		ScriptExecutionStack *NewStack = ScriptExecutionStack::GetNewStack(&OutputParameters);
		NewStack->SetContext(&m_TabTextItem);
		OutputParameters.Add() = NewStack;
		NewStack->Release();
		return true;
	}

	return _superWndClass::ExecuteCommand(FunctionName, InputParameters, OutputParameters, Error);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

tList<Scripting_FavoriteFileList::DefaultPathKey>* Scripting_FavoriteFileList::ms_pDefaultPathKeys = NULL;
tList<char*>* Scripting_FavoriteFileList::ms_pDefaultPaths = NULL;

bool Scripting_FavoriteFileList::ExecuteCommand(char *FunctionName,	
												ScriptExecutionStack &InputParameters,
												ScriptExecutionStack &OutputParameters,
												char *&Error)
{	if (!stricmp( FunctionName , "DoesFolderExist" ))
	{	OutputParameters.Add() = (bool) NewTek_DoesFolderExist( InputParameters[0] );
		return true;
	}
	else if (!stricmp(FunctionName, "AddFavoriteFolder"))
	{
		if (0 < InputParameters.GetSizeOfStack())
		{
            const char *szPath = (char*)InputParameters[0];
			const char *szAlias = NULL;
			if (1 < InputParameters.GetSizeOfStack())
				szAlias = (char*)InputParameters[1];

			const char *szExtKey = NULL;
			const char *szSkinRootKey = NULL;
			if (2 < InputParameters.GetSizeOfStack())
				szExtKey = (char*)InputParameters[2];
			if (3 < InputParameters.GetSizeOfStack())
				szSkinRootKey = (char*)InputParameters[3];
            
			this->AddFavFolderCmd(szPath, szAlias, szExtKey, szSkinRootKey);
			return true;
		}
		else
		{
			assert(false);
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scripting_FavoriteFileList::AddFavFolderCmd(const char *szPath, const char *szAlias, 
												 const char *szExt, const char *szSkinRootKey)
{
	assert(szPath);
	
	// Support relative paths as well which are ID by a beginning "<appdir>/" etc.
	if (0 == ::strncmp("<appdir>", szPath, 8))
	{
		static char s_szPath[MAX_PATH*2] = { 0 };
		DWORD dwSucceeded =	::GetModuleFileName(NULL, s_szPath, MAX_PATH);
		if (dwSucceeded)
		{
			::strcpy(::NewTek_GetLastSlash(s_szPath), const_cast<char*>(szPath+8));
			szPath = s_szPath;
		}
	}
	else if (0 == ::strncmp("<MyDocs>", szPath, 8))
	{
		static char s_szPath[MAX_PATH*2] = { 0 };

		LPITEMIDLIST pidl;
		HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_MYDOCUMENTS, &pidl);
		if (hRes==NOERROR)
		{
			SHGetPathFromIDList( pidl, s_szPath );
			::strcat(s_szPath, const_cast<char*>(szPath+8));
			szPath = s_szPath;
		}

	}
	else if (0 == ::strncmp("<Desktop>", szPath, 9))
	{
		static char s_szPath[MAX_PATH*2] = { 0 };

		LPITEMIDLIST pidl;
		HRESULT hRes = SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &pidl);
		if (hRes==NOERROR)
		{
			SHGetPathFromIDList( pidl, s_szPath );
			::strcat(s_szPath, const_cast<char*>(szPath+9));
			szPath = s_szPath;
		}
	}

	::NewTek_CreateDirectoryPath(szPath);
	if (szAlias && *szAlias)
        FavoriteFileList::GetInstance()->AddDefault(szPath, szAlias, NULL, true);	// Only add to hotlist if I got an alias
	
	if (szExt)
	{
		if (!ms_pDefaultPathKeys)
		{
			ms_pDefaultPathKeys = new tList<DefaultPathKey>;
			ms_pDefaultPaths = new tList<char*>;
		}

		DefaultPathKey newPath = {	::strupr(::NewTek_malloc(szExt)), 
									::NewTek_malloc(::NewTek_FixFilename(::FindFiles_FindFile(FINDFILE_SKINS, const_cast<char*>(szSkinRootKey)))) };
		ms_pDefaultPathKeys->Add(newPath);
		ms_pDefaultPaths->Add(::NewTek_malloc(szPath));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* Scripting_FavoriteFileList::GetDefaultPath(const tList<char*> *pExtList, const char *szSkinRoot)
{
	assert(pExtList || szSkinRoot);

	// If I have a skin root -> find all the keys w/ a matching skin root
	// If I have an extension -> find all the keys w/ a matching extension
	// Return a value based on the 2 iterations

	const char *szRetVal = "";
	if (ms_pDefaultPathKeys)
	{
        size_t firstSkinMatch = static_cast<size_t>(-1);
		size_t bestExtMatch = static_cast<size_t>(-1);
		size_t firstFileFilterMatch = ULONG_MAX;	// Doing a min below
		for (size_t i=0; (!szRetVal[0]) && (i<ms_pDefaultPathKeys->NoItems()); i++)
		{
			const char *szSkinRootIter = (*ms_pDefaultPathKeys)[i].m_szSkinRoot;
			bool bMatchedSkin = false;
			if (szSkinRoot && szSkinRootIter && (0==::stricmp(::NewTek_FixFilename(szSkinRoot), szSkinRootIter)))
			{
				bMatchedSkin = true;
				if (static_cast<size_t>(-1) == firstSkinMatch)
					firstSkinMatch = i;
			}

			const char *szExtIter = (*ms_pDefaultPathKeys)[i].m_szFileExt;
			if (pExtList && szExtIter && szExtIter[0])
			{
				char szMultiExtIter[MAX_PATH] = { 0 };
				::sprintf(szMultiExtIter, "%s%s", szExtIter, ";");
				for (size_t j=0; j<pExtList->NoItems(); j++)
				{
					char *szExtSearch = ::strupr(::NewTek_malloc((*pExtList)[j]));
					if (szExtSearch && (0==::strcmp(szExtSearch, szExtIter) || ::strstr(szExtSearch, szMultiExtIter)))
					{
						::NewTek_free(szExtSearch);

						if (j < firstFileFilterMatch)
						{
							bestExtMatch = i;
							firstFileFilterMatch = j;
						}

						if (bMatchedSkin)
						{
							szRetVal = (*ms_pDefaultPaths)[i];
							break;
						}
					}
					else
                        ::NewTek_free(szExtSearch);
				}
			}
			else if (bMatchedSkin && (!pExtList && (!szExtIter || !szExtIter[0])))	// Are both extensions empty strings?
			{
				szRetVal = (*ms_pDefaultPaths)[i];
			}
		}

		// If matched on both, already returned first match found
		// Else if a found matching extension
		// Else return a found matching skin root
		if (!szRetVal[0])	// If I didn't find an exact match
		{
			if (static_cast<size_t>(-1) != bestExtMatch)
				szRetVal = (*ms_pDefaultPaths)[bestExtMatch];
			else if (static_cast<size_t>(-1) != firstSkinMatch)
				szRetVal = (*ms_pDefaultPaths)[firstSkinMatch];
		}
	}

	return szRetVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Scripting_FavoriteFileList::OnProcessDetach(void)
{
	if (!ms_pDefaultPathKeys)
		return;

	for (size_t i=0; i<ms_pDefaultPathKeys->NoItems(); i++)
	{
		::NewTek_free((*ms_pDefaultPathKeys)[i].m_szSkinRoot);
		::NewTek_free((*ms_pDefaultPathKeys)[i].m_szFileExt);
		::NewTek_free((*ms_pDefaultPaths)[i]);
	}

	delete ms_pDefaultPathKeys;	
	ms_pDefaultPathKeys = NULL;
	delete ms_pDefaultPaths;	
	ms_pDefaultPaths = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

NewTek_RegisterPluginClass(Scripting_FavoriteFileList);
NewTek_RegisterPluginType(Scripting_FavoriteFileList, Script_Interface_GlobalCommand, Mike Watkins, (c)2005 NewTek, 1, /*Priority*/2000);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////