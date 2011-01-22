class FileProperties;

#ifndef __FilePropertiesH_
#define __FilePropertiesH_

void FileManagementDLL FilePropsCloseHack();

/*	FileProperties
Is able to take a file, read in its file info. and read its info file. (Ha).
When a change is made, can move the file if needed, modify system parameters,
and write out the data.  Only one is created for each new property
*/

#define FileProperties_Changed_Cut					"FileProperties_Changed_Cut"
#define FileProperties_Changed_ReCreated			"FileProperties_Changed_ReCreated"
#define FileProperties_Changed_MovedFile			"FileProperties_Changed_MovedFile"
#define FileProperties_Changed_FileDeletedInfo		"FileProperties_Changed_FileDeletedInfo"
#define FileProperties_Changed_FileDeletedNotInfo	"FileProperties_Changed_FileDeletedNotInfo"
#define FileProperties_Changed_AltName				"FileProperties_Changed_AltName"
#define FileProperties_Changed_ColorTint			"FileProperties_Changed_ColorTint"
#define FileProperties_Changed_InProject			"FileProperties_Changed_InProject"

//! These are changed messages a parent directory sends when its children enter, leave, or are moved
#define FileProperties_Changed_FileLeftDir			"FileProperties_Changed_FileLeftDir"
#define FileProperties_Changed_FileEnteredDir		"FileProperties_Changed_FileEnteredDir"

#define ICONINFOFOLDER		"NewTek Info"
#define INFOEXTENSION		"NewTekInfo"
#define ICONEXTENSION		"jpg"
#define ANIMEXTENSION		"avi"
#define DEFAULTICON			"File Icons\\default.jpg"
#define NOFILEICON			"File Icons\\NoFile.jpg"
#define FOLDERICON			"File Icons\\folder.jpg"
#define DRIVENAMEHEADER		"File Icons\\DRIVE_"
#define MYCOMPUTERINFO		"C:\\NewTek Info\\My Computer.MyComputerInfo"
#define DRIVEINFO			"_Drive.DriveInfo"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//mwatkins
bool ArePathsOnSameVolume(const char *szSourcePath, const char *szDestPath);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum eFP_CleanUp
{
	CleanUp_MissingFiles = 0,
	CleanUp_AllFiles = 1,
	
	Delete_AssetManagement = 2,	// Kills the NewTek info file
	Delete_TempFiles = 4,		// Kills files that can be regenerated
	Delete_NonRegenerated = 8,	// Kills files that cannot be regenerated

	Delete_EditCache = 16,		// Always completely deletes, never to Recycle bin
};

class FileProperties_EditList
{
private:
	bool	m_amICutting;
public:
	tList<FileProperties*> m_editList;

	FileProperties_EditList();
	~FileProperties_EditList();

	void	ClearList();
	FileProperties*	AddToList(FileProperties* fileName);

	void	SetCutting(bool amICutting);
	bool	AmICutting(){return m_amICutting;};
};

enum FileProperties_Status
{
	FileProperties_NoFile,
	FileProperties_File,
	FileProperties_Drive,
	FileProperties_MyComputer,
	FileProperties_Directory,
};
class FileButton;
class AnimatedIcon_Interface;
class FileCopyMoveHandler;		//mwatkins
class FileManagementDLL FileProperties : public AssetList, public AssetFile_Interface
{
public:
	//! Process Management
		static void OnProcessDetach();
		static void OnProcessAttach();

	//! Just what kind of file is this anyway  FileProperties_Status?
		static long FindFileStatus(const char* fileName);
		long		GetFileStatus();

	//! Finding all of My helper files
		static bool				GetFileFromInfoName			(const char* infoFileName, char* fileName);	// returns true if an info file, false if some temp file
		static bool				GetInfoFromFileName			(const char* fileName, char* infoFileName);
		static void				GetIconFromFileName			(const char* fileName, char* iconFileName);
		static void				GetAnimatedIconFromFileName	(const char* fileName, char* iconFileName);

	//! static functions for file manipulation, acts on helper-files also
		static  bool			_MoveFile(const char* oldFileName, const char* newFileName);
		static  bool			_CopyFile(const char* oldFileName, const char* newFileName);
		static  bool			_DeleteFile(const char* oldFileName, bool deleteInfo = true, bool recycle = false);

	//! checks a path for attributes that indicate it's a newtek info folder
		static bool IsNewTekInfoFolder(const char* testPath);	//mwatkins
		
	////////////////////////////////////////////////
	//! Global Edit functions that act on a group of files
		static bool	EditCut(tList<FileProperties*>* fileNames);
		static bool	EditCopy(tList<FileProperties*>* fileNames);
		static bool	EditPaste(const char* destination);
		static bool	EditDelete(tList<FileProperties*>* fileNames);
		static void	EditProperties(tList<FileProperties*>* fileNames, BaseWindowClass* parent, long XPos = -1, long YPos = -1);

	//mwatkins - added last param for async copying and moving for versions 2.5 and greater
	//           also removed some stupid copyToDiffVolume flag in the MoveToDirectory method that wasn't being used
		static bool CopyToDirectory(tList<FileProperties*>* fileNames, const char* destination, bool bDoAsync = false);
		static bool MoveToDirectory(tList<FileProperties*>* fileNames, const char* destination, bool bDoAsync = false);


	//! Working with the Handle of this file
		static FileProperties*	GetHandle(const char* _fileName, FileProperties* parent = NULL);	
		static void				ReleaseHandle(FileProperties* releaseMe);
		void					GetHandle(){m_numAccesses++;}
		void					ReleaseHandle(){ReleaseHandle(this);}
		FileProperties*			GetParentHandle();


	/* Call this little utility function to be sure that a newly created file shows up in any FileBin
		This function is no longer needed because filebins listen to the windwos hooks. 
		//static void CreateNewFile(char* fileName){ReleaseHandle(GetHandle(fileName));}*/

	//! Dealing with the filename of me and my daddy, (Move, Delete, Copy)
		const char*	GetFileName();
		const char*	GetParentDir();
		void	SetFileName(const char* filename)
			{	FileMoveFile(filename);
			}

		bool	FileMoveFile(const char* newFullPath);			
		bool	FileCopyFile(const char* newFullPath);

		bool	FileDeleteFile(bool deleteInfo, bool recycle);
		bool	FileChangePath(const char* newPath, bool incrementIfExists = false);			//! leaves the name the same, only changes path
		bool	FileChangeFilename(const char* newFilename, bool incrementIfExists = false);	//! leaves the path the same, only changes name

	//! Dealing with my alter ego
		void	FileChangeAltName(const char* newAltName);
		DynamicString*	GetAltFilenameDynamic();
		const char*		GetAltFilename();
		
	//! Get a string that represents this type of item
		char* FP_GetTypeString();

	//! Just what color am I anyways?
		WindowPixel	GetTintColor();
		void SetTintColor(unsigned newColor);

	//! Keeping to Cut information
		bool	AmICut();
		void	SetCut(bool cut = true);

	//! Check on the existence of the files
		bool	DoesFileExist();
		bool	DoesInfoExist();

	//! As a DynamicListener to all of my properties
		virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	//! The actual call that writes out the changes to the file
		void	WriteInfoFile();

	//! Use these to add and remove from file properties dialogs
		void			AddToDialog();
		void			RemoveFromDialog();
		void			GenerateFileProperties();
		unsigned __int64 GetDirectorySize( const char* dirName);
		void			GetDirectorySize( const char* dirName, unsigned long & highSize, unsigned long & lowSize);
		

	//! Doing the AssetList, Write to Structure thing
		virtual void	AssetList_WriteToStructure(ScriptStructure & scriptStructure) ;
		virtual void	AssetList_ReadFromStructure(ScriptStructure & scriptStructure);

	//! Publicly accessing the edit list
		static FileProperties_EditList* s_editList;

	/*! A nice function for finding the next empty but available file name
		BEWARE, uses a static buffer, be sure to cache the value if this will be called multiple times
	*/
		static const char* GetNextAvailableFileName(const char* fileName);

	//This will avoid you from having to create duplicate combo items when accessing the users preference
		static int GetFileNamingPreference();

	/*! A nice function for finding the next empty but available file name
		TODO, cleanup all functions using the old method
	*/
		static unsigned GetNextAvailableFileName(const char *Source,char *DestBuffer,unsigned BufferSize=MAX_PATH,
			int FileNameingType=-1,			//You can override the preference's naming convention. -1 will use the prefs
			bool MayReturnOrignalName=true  //If the file doesn't exist it may return the orginal name... otherwise it will always return a numbered index value
			);
	//! This similiar to GetNextAvailableFileName... but does not check the condition of a file's existance
		static unsigned GetNextAvailableStringName(const char *Source,char *DestBuffer,unsigned BufferSize=MAX_PATH);
	//! Call this directly if you know what index you wish to append. 
	//  Note: -1 will not append and simply copy the source into the DestBuffer
		static unsigned GetStringNameWithAppendedIndex(const char *Source,char *DestBuffer,unsigned AppendedIndex=-1,unsigned BufferSize=MAX_PATH);
		
	//! Dealing with FileButtons that are interested in me
		unsigned GetNumInterestedFileButtons()				{	return (unsigned)m_interestedButtons.NoItems();	}
		FileButton* GetInterestedFileButton(unsigned index)	{	return m_interestedButtons[index];	}
		void RefreshInterestedFileButtonIcons();

	/*! Cleaning up NewTek Info files and folders
		\return the number of files deleted, 
		\param path be sure path does not end with delimeter
		\param cleanUpFlags Use eFP_CleanUp flags 
	*/
		static unsigned CleanUp_NewTekInfo(const char* path, bool recursive, int cleanUpFlags, bool recycleBin);

	//! Is this thing being edited in a Toaster App
		void FP_LockInToasterApp();
		bool FP_FreeFromToasterApp();
		bool FP_IsLockedInToasterApp(){return (m_toasterAppLocks != 0);}

//! Don't Touch!
protected:
	bool EnsureMoveReadOnly(bool multiple, bool & yesAll);
	static bool EnsureOverwrite(const char* overThisFile, bool multiple, bool &yesAll, bool fileOverwrites);
	static bool EnsureDirectory(const char* directoryName);

//mwatkins - I added all this bullshit below
//! stuff for async copying and moving of files and folders (implemented in FileCopyMove.h & .cpp)
friend FileCopyMoveHandler;
	static bool _AsyncEnsureCopyFile(char* oldFileName, char* newFileName,
									bool bMultipleFiles, FileProperties_Status fileType,
									bool &bReplaceAll);
	static bool _AsyncEnsureMoveFile(char* oldFileName, char* newFileName,
									bool bMultipleFiles, FileProperties_Status fileType,
									bool &bReadOnlyMoveAll, bool &bReplaceAll);
	static void _AsyncCopiedFile(char* oldFileName, char* newFileName);
	static void _AsyncMovedFile(char* oldFileName, char* newFileName);
	void FileAsyncCopiedFile(char* newFullPath);
	void FileAsyncMovedFile(char* newFullPath);
	bool EnsureAsyncMoveFile(char* newFullPath, bool bMultipleFiles, 
							FileProperties_Status fileType, bool &bReadOnlyMoveAll,
							bool &bReplaceAll);

//! The Friends who REALLY know what I am like on the inside!
friend FileButton;
	//! Creating the Proper FileProperties
		static FileProperties* NewFileProperties(char* fileName, FileProperties* parent, long status = -1);

	//! Maintain a deletion list for cleaning up assets when no longer needed
		static unsigned			s_numProperties;
		static unsigned			s_maxProperties;
		static FileProperties*	s_deletionListHead;
		static FileProperties*	s_deletionListTail;
		FileProperties*			m_deletionListPrev;
		FileProperties*			m_deletionListNext;
		void RemoveFromDeletionList();
		void AddToDeletionList();

	//! Copying or moving a folder is a special process
		bool	FileCopyFolder(const char* newFullPath);
		bool	FileMoveFolder(const char* newFullPath);	//mwatkins - fixes 2.0 bug.  SDK doc says MoveFile can't be used to move folders between volumes.

	//! How many people are interested in me
		long	m_numAccesses;

	//! My Computer is always kept track of
		static FileProperties*	s_MyComputer;

	//! I know about my parent and all of my children
		tList<FileProperties*>	m_children;
		FileProperties*	m_parentDir;
		void			ChangeParent(FileProperties* newParent);

	//! Dealing with file specific info
		void	ResetFileProperties(bool bForceReset = false);	//mwatkins - added param to force delete all assets and force read info from disk
		void	ResetFileSize();

	//! Am I being looked at in a properties dialog?
		unsigned	m_numPropDLG;
		bool		m_propsGenerated;

	//! Creating and setting the common file properties
		virtual void GetCommonAssets(WIN32_FIND_DATA findData);
		virtual void GenerateAssetList(){if (!m_propsGenerated) GenerateFileProperties();}

	//! Finding the AltFilename & Color Tint
		AssetElement*	m_colorAsset;
		WindowPixel		m_colorTint;
		DynamicString	m_altName;
		bool			m_altNameRead;
		bool			m_altNameChanged;

	//! Constructors & Destructors
		FileProperties(char* fileName, FileProperties* parent, long type);
		~FileProperties();

	//! Other Members
		DynamicString	m_fullPath;
		bool			m_cut;
		bool			m_infoExists;
		long			m_status;
		bool			m_amIchanging;
		bool			m_propsChanged;

	//! Help with message boxes
		static char s_messageString[1024];

	//! Helper for the Cleanup
		static unsigned CleanUp_NewTekInfoFolder(const char* newTekInfoFolder, int cleanUpFlags, bool recycleBin);
		static unsigned CleanUp_EditorCacheFiles(const char* path);

	//! Does not delete, returns true if the file should be deleted
		static bool CleanUp_TemporaryFile(const char* newTekInfoFolder, WIN32_FIND_DATA & findData, int cleanUpFlags);
		static bool IsFileNonRegenerated(WIN32_FIND_DATA & findData, char* originalFile);

	//! Keeping track of a list of all existing file buttons that are talking to me
		tList<FileButton*>		m_interestedButtons;

	//! Keep track of Being edited in some Toaster App
		unsigned				m_toasterAppLocks;
		void					LockedInAppMessageBox(char* operationTitle);
};

#endif //! #ifndef __FilePropertiesH_