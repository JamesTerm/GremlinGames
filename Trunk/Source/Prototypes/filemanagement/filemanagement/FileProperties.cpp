#include "stdafx.h"
#include <time.h>

//***************************************************************************************************************************************
//mwatkins

bool ArePathsOnSameVolume(const char *szSourcePath, const char *szDestPath)
{
	if (!szSourcePath || !szDestPath)
		return false;

	char szSourceDrive[_MAX_DRIVE] = {0};
	char szDestDrive[_MAX_DRIVE] = {0};

	_splitpath(szSourcePath, szSourceDrive, NULL, NULL, NULL);
	_splitpath(szDestPath, szDestDrive, NULL, NULL, NULL);

	return (0 == stricmp(szSourceDrive, szDestDrive));
}

//***************************************************************************************************************************************
static VariableBlock g_AssetHandleLock;

inline unsigned __int64 GetInt64(unsigned long high, unsigned long low)
{
	return ((unsigned __int64)(high) << 32) + (unsigned __int64)low;
}

inline void BreakInt64(unsigned __int64 value, unsigned long & high, unsigned long & low)
{
	low = (unsigned long)value;
	high = (unsigned long)(value >> 32);
}

//***************************************************************************************************************************************
// Static Variables
FileProperties*				FileProperties::s_MyComputer = NULL;
unsigned					FileProperties::s_numProperties = 0;
unsigned					FileProperties::s_maxProperties = 1024;
FileProperties*				FileProperties::s_deletionListHead = NULL;
FileProperties*				FileProperties::s_deletionListTail = NULL;
FileProperties_EditList*	FileProperties::s_editList = NULL;
char						FileProperties::s_messageString[1024];

//***************************************************************************************************************************************
void FileProperties_MoveSupportingFiles(const char* fromFile, const char* toFile, bool move)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Create the search string for the supporting files
		// Figure out the newtek info folder to use
			char supportingFilesFilter[MAX_PATH];
			FileProperties::GetInfoFromFileName(fromFile, supportingFilesFilter);
			char* lastFilterSlash = NewTek_GetLastSlash(supportingFilesFilter);
			if (lastFilterSlash) *lastFilterSlash = 0;

		// Append the name only portion of the original filename
			const char* origNameOnly = NewTek_GetLastSlash((char*)fromFile);
			if (origNameOnly)
				strcat(supportingFilesFilter, origNameOnly);

		// Append the .* on the end to complete the filter
			strcat(supportingFilesFilter, ".*");

	// Create a list of all of the existing support files
		tList<char*>	supportingFiles;
		FindFiles_Search(supportingFilesFilter, supportingFiles);

	// Grab the destination directory and name for where files will go, and create a string to help
		char destInfo[MAX_PATH];
		char* appendHere = NULL;
		if (toFile)
		{
			// Grab the info folder by itself
				if ( !FileProperties::GetInfoFromFileName(toFile, destInfo) )
				{	char NewFolder[MAX_PATH*2];
					strcpy(NewFolder,destInfo);
					char *LastSlash = NewTek_GetLastSlash(NewFolder);
					if (LastSlash) LastSlash[0]=0;
					CreateDirectory( NewFolder , NULL );

					if ( !(GetFileAttributes( NewFolder ) & FILE_ATTRIBUTE_HIDDEN ) )
						SetFileAttributes( NewFolder, FILE_ATTRIBUTE_HIDDEN);
				}
				char* lastNewInfoSlash = NewTek_GetLastSlash(destInfo);
				*lastNewInfoSlash = 0;

			// Be sure the new folder exists and is hidden
			// mwatkins - ALWAYS make the dir hidden
				CreateDirectory(destInfo, NULL);
				if ( !(GetFileAttributes( destInfo ) & FILE_ATTRIBUTE_HIDDEN ) )
					SetFileAttributes(destInfo, FILE_ATTRIBUTE_HIDDEN);

			// Grab the filename only of the toFile
				const char* newNameOnly = NewTek_GetLastSlash((char*)toFile);

			// Build the string to use with the name
				strcat(destInfo, newNameOnly);

			// Find a good pointer to always append to
				appendHere = destInfo + strlen(destInfo);
		}

	// Iterate through the list to move, copy, or delete them
		for (unsigned i = 0; i < supportingFiles.NoItems(); i++)
		{
			// Are we moving or copying?
			if (toFile)
			{
				// Grab the extension to append to destInfo
				const char* lastDot = NewTek_GetLastDot(supportingFiles[i]);
				strcpy(appendHere, lastDot);

				// Move or Copy
				if (move)
					::MoveFile(supportingFiles[i], destInfo);
				else
					::CopyFile(supportingFiles[i], destInfo, false);
			}
			else	// Delete the file
				::DeleteFile(supportingFiles[i]);
		}

	// Free the list that I just created
		FindFiles_SearchBelowApplication_Free(supportingFiles);
}

/*	FileProperties_EditList
Keeps track of the static edit list.  There is only one of these and it is private and static
*/
//***************************************************************************************************************************************
FileProperties_EditList::FileProperties_EditList(){m_amICutting = false;}
FileProperties_EditList::~FileProperties_EditList(){ClearList();}
void FileProperties_EditList::ClearList()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	for (unsigned i = 0; i < m_editList.NoItems(); i++)
	{
		m_editList[i]->SetCut(false);
		FileProperties::ReleaseHandle(m_editList[i]);
	}
	// Delete the list itself
	m_editList.DeleteAll();
	m_amICutting = false;
}

//***************************************************************************************************************************************
FileProperties* FileProperties_EditList::AddToList(FileProperties* fileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileName) return NULL;
	if (!m_editList.Exists(fileName))
	{
		fileName->GetHandle();
		m_editList.Add(fileName);
	}
	fileName->SetCut(m_amICutting);
	return fileName;
}

//***************************************************************************************************************************************
void FileProperties_EditList::SetCutting(bool amICutting)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	m_amICutting = amICutting;
	for (unsigned i = 0; i < m_editList.NoItems(); i++)
		m_editList[i]->SetCut(m_amICutting);
}

//***************************************************************************************************************************************
void FileProperties::OnProcessDetach()
{	
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Get rid of the edit list
	if (s_editList)
	{
		delete s_editList;
		s_editList=NULL;
	}

	long n = s_numProperties;
	char debugString[32];
	sprintf(debugString, "Deleting %ld FileProperties.\n", n);
	OutputDebugString(debugString);
	// This gets rid of the whole tree for us!

	if (s_MyComputer)
	{
		delete s_MyComputer;
		s_MyComputer=NULL;
	}

	// We can then get rid of anything else that is left in the deletion list
	while (s_deletionListHead)
		delete s_deletionListHead;

	assert(s_deletionListHead==NULL);

	if (s_numProperties)
		_throw ("FileProperties::OnProcessDetach, Not all File PRoperties Deleted");

	OutputDebugString("Finished Deleting FileProperties.\n");
}


//***************************************************************************************************************************************
void FileProperties::OnProcessAttach()
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Create the First Property for My Computer
	s_MyComputer = new FileProperties(FILEBIN_MYCOMPUTER, NULL, FileProperties_MyComputer);

	// Create the Edit List
	s_editList = new FileProperties_EditList();
}

//***************************************************************************************************************************************
const char*	FileProperties::GetNextAvailableFileName(const char* fileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	
	int FileNameingType=GetFileNamingPreference();
	// Be sure the original makes since
	if (!fileName||(!fileName[0])) return fileName;
	// See if the original is already ok
	if ((!NewTek_DoesFileExist(fileName))&& 
		(!NewTek_DoesFolderExist(fileName))) return fileName;
	// Store the filename
	static char Buffer[MAX_PATH];
	strcpy(Buffer,fileName);
	// Is there a file extension ?
	char *Remainder=NewTek_GetLastDot(Buffer);
	const char *Extension=NewTek_GetLastDot(fileName);
	if (!Extension) Extension="";
	// Now get any number
	if (!Remainder) Remainder=Buffer+strlen(Buffer);
	else			;//Remainder--;	
	// Now get any nunber
	while((Remainder[-1]>='0')&&(Remainder[-1]<='9'))
		Remainder--;
	// Get the number
	int currentNumber=FileNameingType>4?0:1;

	//See if a number already exists
	if ((Remainder[0]>='0')&&(Remainder[0]<='9')) {
		currentNumber=atoi(Remainder);
		//now for the next position (if not found this starts with default)
		currentNumber++;
		}
	// Get the naming string
	char *NamingString="%d%s";
	switch(FileNameingType)
	{	case 0:	NamingString="%d%s"; break;
		case 1:	NamingString="%03d%s"; break;
		case 2:	NamingString="%04d%s"; break;
		case 3:	NamingString="%05d%s"; break;
		case 4:	NamingString="%d%s"; break;
		case 5:	NamingString="%03d%s"; break;
		case 6:	NamingString="%04d%s"; break;
		case 7:	NamingString="%05d%s"; break;
	}
	// Now rebuild the path
	do {
		sprintf(Remainder,NamingString,currentNumber,Extension);
		if ((!NewTek_DoesFileExist(Buffer))&& 
			(!NewTek_DoesFolderExist(Buffer))) return Buffer;
		currentNumber++;
		} while(true);
	return NULL;
}


#define NumOfFileNamingFormats 8
const static char *FileNamingTable[NumOfFileNamingFormats] = {
	"%d%s",
	"%03d%s",
	"%04d%s",
	"%05d%s",
	"%d%s",
	"%03d%s",
	"%04d%s",
	"%05d%s"
	};

// These are the file naming options
const static ComboItem RecordPanel_ComboItems[]=
	{	{ "File1.avi File2.avi File3.avi ... File10.avi","File1.avi File2.avi File3.avi ... File10.avi",0 },
		{ "File001.avi File002.avi File003.avi ... File010.avi","File001.avi File002.avi File003.avi ... File010.avi",1 },
		{ "File0001.avi File0002.avi File0003.avi ... File0010.avi","File0001.avi File0002.avi File0003.avi ... File0010.avi",2 },
		{ "File00001.avi File00002.avi File00003.avi ... File00010.avi","File00001.avi File00002.avi File00003.avi ... File00010.avi",3 },
		{ "File0.avi File1.avi File2.avi ... File10.avi","File0.avi File1.avi File2.avi ... File10.avi",4 },
		{ "File000.avi File001.avi File002.avi ... File010.avi","File000.avi File001.avi File002.avi ... File010.avi",5 },
		{ "File0000.avi File0001.avi File0002.avi ... File0010.avi","File0000.avi File0001.avi File0002.avi ... File0010.avi",6 },
		{ "File00000.avi File00001.avi File00002.avi ... File00010.avi","File00000.avi File00001.avi File00002.avi ... File00010.avi",7 }
	};	

int FileProperties::GetFileNamingPreference()
{
	tList<ComboItem> l_ComboItem_Input;
	for(unsigned i=0;i<8;i++)
		l_ComboItem_Input.Add(RecordPanel_ComboItems[i]);

	return Preferences::GetValue_Int("File Naming",0,&l_ComboItem_Input);
}

unsigned FileProperties::GetNextAvailableFileName(const char *Source,char *DestBuffer,unsigned BufferSize,int FileNameingType,bool MayReturnOrignalName) 
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	
	FileNameingType=(FileNameingType==-1)?GetFileNamingPreference():FileNameingType;

	char SourceBuffer[MAX_PATH];
	SourceBuffer[0]=0;

	{
		// Be sure the original exists and not empty
		if (!Source) goto WrapItUp;
		if (Source[0]==0) goto WrapItUp;
		// See if the original is already ok
		if	((MayReturnOrignalName) &&
			 (!NewTek_DoesFileExist(Source)) && 
			 (!NewTek_DoesFolderExist(Source))
			) 
		{
			strcpy(SourceBuffer,Source);
			goto WrapItUp;
		}
		strcpy(SourceBuffer,Source);
		// Is there a file extension ?
		char *Remainder=NewTek_GetLastDot(SourceBuffer);
		const char *Extension=NewTek_GetLastDot(const_cast<char *>(Source));
		if (!Extension) Extension="";
		// Now get any number
		if (!Remainder) Remainder=SourceBuffer+strlen(SourceBuffer);
		else			;//Remainder--;	
		// Now get any nunber
		while((Remainder[-1]>='0')&&(Remainder[-1]<='9'))
			Remainder--;
		// Get the number
		int currentNumber=(FileNameingType>4)?0:1;
		//See if a number already exists
		if ((Remainder[0]>='0')&&(Remainder[0]<='9')) {
			currentNumber=atoi(Remainder);
			//now for the next position (if not found this starts with default)
			currentNumber++;
			}
		// Get the naming string
		const char *NamingString=FileNamingTable[FileNameingType];

		sprintf(Remainder,NamingString,currentNumber,Extension);

		do {	
			sprintf(Remainder,NamingString,currentNumber,Extension);
			if ((!NewTek_DoesFileExist(SourceBuffer))&& 
				(!NewTek_DoesFolderExist(SourceBuffer))) goto WrapItUp;
			currentNumber++;
			} while(true);

	} WrapItUp:;

	unsigned NumBytesCopied=strlen(SourceBuffer)+1;

	if ((DestBuffer)&&(NumBytesCopied)) {
		NumBytesCopied=min(BufferSize,NumBytesCopied);
		memcpy(DestBuffer,SourceBuffer,NumBytesCopied);
		}

	return NumBytesCopied;
	}

unsigned FileProperties::GetNextAvailableStringName(const char *Source,char *DestBuffer,unsigned BufferSize) {
	int FileNameingType=GetFileNamingPreference();

	char SourceBuffer[MAX_PATH];
	SourceBuffer[0]=0;
	if (Source) if(Source[0]) {
		strcpy(SourceBuffer,Source);
		// Is there a file extension ?
		char *Remainder=NewTek_GetLastDot(SourceBuffer);
		const char *Extension=NewTek_GetLastDot(const_cast<char *>(Source));
		if (!Extension) Extension="";
		// Now get any number
		if (!Remainder) Remainder=SourceBuffer+strlen(SourceBuffer);
		else			;//Remainder--;	
		// Now get any nunber
		while((Remainder[-1]>='0')&&(Remainder[-1]<='9'))
			Remainder--;
		// Get the number
		int currentNumber=(FileNameingType>4)?0:1;
		//See if a number already exists
		if ((Remainder[0]>='0')&&(Remainder[0]<='9')) {
			currentNumber=atoi(Remainder);
			//now for the next position (if not found this starts with 1)
			currentNumber++;
			}
		// Get the naming string
		const char *NamingString=FileNamingTable[FileNameingType];

		sprintf(Remainder,NamingString,currentNumber,Extension);
		}

	unsigned NumBytesCopied=strlen(SourceBuffer)+1;

	if ((DestBuffer)&&(NumBytesCopied)) {
		NumBytesCopied=min(BufferSize,NumBytesCopied);
		memcpy(DestBuffer,SourceBuffer,NumBytesCopied);
		}

	return NumBytesCopied;
	}

unsigned FileProperties::GetStringNameWithAppendedIndex(const char *Source,char *DestBuffer,unsigned AppendedIndex,unsigned BufferSize) {
	int FileNameingType=GetFileNamingPreference();

	char SourceBuffer[MAX_PATH];
	SourceBuffer[0]=0;
	if (Source) if(Source[0]) {
		strcpy(SourceBuffer,Source);
		if (AppendedIndex!=-1) { //if -1 it will return a copy with no change
			// Is there a file extension ?
			char *Remainder=NewTek_GetLastDot(SourceBuffer);
			const char *Extension=NewTek_GetLastDot(const_cast<char *>(Source));
			if (!Extension) Extension="";
			// Now get any number
			if (!Remainder) Remainder=SourceBuffer+strlen(SourceBuffer);
			else			;//Remainder--;	
			// Now get any nunber
			while((Remainder[-1]>='0')&&(Remainder[-1]<='9'))
				Remainder--;

			int currentNumber=(FileNameingType>4)?0:1;
			currentNumber+=AppendedIndex;

			// Get the naming string
			const char *NamingString=FileNamingTable[FileNameingType];

			sprintf(Remainder,NamingString,currentNumber,Extension);
			}
		}

	unsigned NumBytesCopied=strlen(SourceBuffer)+1;

	if ((DestBuffer)&&(NumBytesCopied)) {
		NumBytesCopied=min(BufferSize,NumBytesCopied);
		memcpy(DestBuffer,SourceBuffer,NumBytesCopied);
		}

	return NumBytesCopied;
	}

//***************************************************************************************************************************************
long	FileProperties::GetFileStatus()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	return m_status;
}

//***************************************************************************************************************************************
long FileProperties::FindFileStatus(const char* fileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Check my parameter
	if (!fileName) return FileProperties_NoFile;

	// Is this a drive
	if (GetDriveType(fileName) != DRIVE_NO_ROOT_DIR)
		return FileProperties_Drive;

	// See if this is a folder or a file
	// Get the file information
	long ret = FileProperties_NoFile;
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(fileName, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ret = FileProperties_Directory;
		else ret = FileProperties_File;
	}
	FindClose(findHandle);

	// Is this FILEBIN_MYCOMPUTER
	if ((ret == FileProperties_NoFile) && (!stricmp(fileName, FILEBIN_MYCOMPUTER)))
		return FileProperties_MyComputer;

	if (ret == FileProperties_NoFile)
		ret = FileProperties_NoFile;

	return ret;
}

//***************************************************************************************************************************************
bool FileProperties::GetFileFromInfoName(const char* infoFileName, char* fileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!infoFileName || !fileName)
		return false;

	// Look to see if this info file represents My Computer
	const char* lastDot = NewTek_GetLastDot(infoFileName);
	if (!lastDot)
	{
		strcpy(fileName, "");
		return false;
	}
	if (!stricmp(lastDot+1, "MyComputerInfo"))
	{
		strcpy(fileName, FILEBIN_MYCOMPUTER);
		return true;
	}
	else if (!stricmp(lastDot+1, "DriveInfo"))
	{
		// The drive letter is 7 letters BEFORE the dot
		char driveName = *(lastDot-7);
		sprintf(fileName, "%c:", driveName);
		return true;
	}

	// Make a copy of the full path to play with
	DynamicString fullPath(infoFileName);
	
	// Getting just the file name and path
	char* nameOnly = NewTek_GetLastSlash( (char*)(const char*)fullPath );
	*nameOnly = 0; nameOnly++;

	// Stripping the extension to find the real file name
	char* dotpos = (char*)NewTek_GetLastDot(nameOnly);
	if (dotpos)
		*dotpos = 0;

	// Backing up one directory
	char* upOneDir = (char*)NewTek_GetLastSlash((char*)(const char*)fullPath);
	if (upOneDir)
		*upOneDir = 0;

	// Here is the new path
	sprintf(fileName, "%s\\%s", fullPath.Get(), nameOnly);

	return (!stricmp(lastDot+1, INFOEXTENSION));
}

//***************************************************************************************************************************************
bool FileProperties::GetInfoFromFileName(const char* fileName, char* infoFileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!infoFileName || !fileName)
		return false;

	// First aee if this is a drive
	long status = FileProperties_NoFile;

	UINT driveType = GetDriveType(fileName);
	if (driveType != DRIVE_NO_ROOT_DIR)
		status = FileProperties_Drive;
	else if (!stricmp(fileName, FILEBIN_MYCOMPUTER))
		status = FileProperties_MyComputer;

	char driveLetter = 0;
	if (status == FileProperties_MyComputer)
	{
		strcpy(infoFileName, MYCOMPUTERINFO);
		driveLetter = 'C';
	}
	else if (status == FileProperties_Drive)
	{
		driveLetter = fileName[0];
		sprintf(infoFileName, "C:\\%s\\%c%s", ICONINFOFOLDER, driveLetter, DRIVEINFO);
	}
	if (driveLetter)
	{
		char newFolder[32];
		sprintf(newFolder, "C:\\%s", ICONINFOFOLDER);
		if ( !(GetFileAttributes( newFolder ) & FILE_ATTRIBUTE_HIDDEN ) )
			SetFileAttributes(newFolder, FILE_ATTRIBUTE_HIDDEN);
		return NewTek_DoesFolderExist( newFolder );
	}

	// Make a copy of the full path to play with
	DynamicString fullPath(fileName);

	// Getting just the file name and path
	char* nameOnly = NewTek_GetLastSlash((char*)(const char*)fullPath);
	if (!nameOnly)
	{
		strcpy(infoFileName, "");
		return false;
	}
	*nameOnly = 0; nameOnly++;

	// Create the folder first
	sprintf(infoFileName, "%s\\%s", (const char*)fullPath, ICONINFOFOLDER);
	if ( !(GetFileAttributes( infoFileName ) & FILE_ATTRIBUTE_HIDDEN ) )
		SetFileAttributes(infoFileName, FILE_ATTRIBUTE_HIDDEN);

	sprintf(infoFileName, "%s\\%s\\%s.%s", (const char*)fullPath, ICONINFOFOLDER, nameOnly, INFOEXTENSION);

	return NewTek_DoesFolderExist( infoFileName );
}

//***************************************************************************************************************************************
void FileProperties::GetAnimatedIconFromFileName(const char* fileName, char* iconFileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!iconFileName || !fileName)
		return;

	// Find out what theinfo Path is
	char infoPath[MAX_PATH];
	GetInfoFromFileName(fileName, infoPath);
	char* lastDot = NewTek_GetLastDot(infoPath);
	if (!lastDot)
	{
		strcpy(iconFileName, "");
		return;
	}
	*lastDot = 0;

	sprintf(iconFileName, "%s.%s", infoPath, ANIMEXTENSION);
}

//***************************************************************************************************************************************
void FileProperties::GetIconFromFileName(const char* fileName, char* iconFileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!iconFileName || !fileName)
		return;

	// Find out what theinfo Path is
	char infoPath[MAX_PATH];
	GetInfoFromFileName(fileName, infoPath);
	char* lastDot = NewTek_GetLastDot(infoPath);
	if (!lastDot)
	{
		strcpy(iconFileName, "");
		return;
	}
	*lastDot = 0;

	sprintf(iconFileName, "%s.%s", infoPath, ICONEXTENSION);
}

//***************************************************************************************************************************************
bool FileProperties::_MoveFile(const char* oldFileName, const char* newFileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!oldFileName || !newFileName)
		return false;
	FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
	bool ret = oldFile->FileMoveFile(newFileName);
	FileProperties::ReleaseHandle(oldFile);
	return ret;
}

//***************************************************************************************************************************************
bool FileProperties::_CopyFile(const char* oldFileName, const char* newFileName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!oldFileName || !newFileName)
		return false;
	FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
	bool ret = oldFile->FileCopyFile(newFileName);
	FileProperties::ReleaseHandle(oldFile);
	return ret;
}

//***************************************************************************************************************************************
bool FileProperties::_DeleteFile(const char* oldFileName, bool deleteInfo, bool recycle)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!oldFileName || !oldFileName[0])
		return false;
	bool ret = false;
	FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
	if (oldFile)
	{
		ret = oldFile->FileDeleteFile(deleteInfo, recycle);
		FileProperties::ReleaseHandle(oldFile);
	}
	return ret;
}

//***************************************************************************************************************************************
//mwatkins shit
bool FileProperties::IsNewTekInfoFolder(const char* testPath)
{
	bool bRetVal = false;
	
	//test for hidden directory, w/ NewTek Info name
	FileProperties_Status status = 
		(FileProperties_Status) FileProperties::FindFileStatus(testPath);
	if (FileProperties_Directory == status)
	{
		DWORD dwAtts = ::GetFileAttributes(testPath);
		if (FILE_ATTRIBUTE_HIDDEN & dwAtts)
		{
			char fname[_MAX_FNAME] = {0};
			::_splitpath(testPath, NULL, NULL, fname, NULL);

			bRetVal = (0 == ::stricmp(fname, ICONINFOFOLDER));
		}
	}

	return bRetVal;
}

bool FileProperties::_AsyncEnsureCopyFile(char* oldFileName, char* newFileName,
										  bool bMultipleFiles, FileProperties_Status fileType,
										  bool &bReplaceAll)
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);

	// Bad pointer
	if (!newFileName || !newFileName) 
		return false;

	// Make sure we're not copying over the same path
	if (0 == stricmp(oldFileName, newFileName))
		return false;

	if (FileProperties::EnsureOverwrite(newFileName, bMultipleFiles, 
										bReplaceAll, fileType == FileProperties_File))
	{
		// If the file already exists, ask if we want to overwrite
		if (::NewTek_DoesFileExist(newFileName))
		{
			if (!FileProperties::_DeleteFile(newFileName))
			{
				sprintf(s_messageString, TL_GetString( "TR::" "Could not overwrite the file:\n\"%s\"", "Could not overwrite the file:\n\"%s\"" ), newFileName);
				NewTek_MessageBox(NewTek_GetGlobalParentHWND(),s_messageString,TL_GetString( "TR::" "Failed to Overwrite File!", "Failed to Overwrite File!" ), MB_OK, NULL);
				return false;
			}
		}

		return true;
	}
	else
		return false;
}

bool FileProperties::_AsyncEnsureMoveFile(char* oldFileName, char* newFileName,
											bool bMultipleFiles, FileProperties_Status fileType,
											bool &bReadOnlyMoveAll, bool &bReplaceAll)
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);

	bool bRetVal = false;
	
	if (oldFileName && newFileName)
	{
		FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
		bRetVal = oldFile->EnsureAsyncMoveFile(newFileName, bMultipleFiles, fileType,
												bReadOnlyMoveAll, bReplaceAll);
		FileProperties::ReleaseHandle(oldFile);
	}

	return bRetVal;
}

void FileProperties::_AsyncCopiedFile(char* oldFileName, char* newFileName)
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!oldFileName || !newFileName)
		return;
	FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
	oldFile->FileAsyncCopiedFile(newFileName);
	FileProperties::ReleaseHandle(oldFile);
}

void FileProperties::_AsyncMovedFile(char* oldFileName, char* newFileName)
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!oldFileName || !newFileName)
		return;
	FileProperties* oldFile = FileProperties::GetHandle(oldFileName);
	oldFile->FileAsyncMovedFile(newFileName);
	FileProperties::ReleaseHandle(oldFile);
}

//end mwatkins shit
//***************************************************************************************************************************************

FileProperties*	FileProperties::GetHandle(const char* _fileName, FileProperties* parent)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);

	// Check for valid input
	if (!_fileName) return NULL;
	if (!_fileName[0]) return NULL;	
	
	// Are we wanting My Computer?
	if (!stricmp(_fileName, FILEBIN_MYCOMPUTER))
		return s_MyComputer;

	// Make my own internal copy of the filename and fix it in place
	char *fileName=NewTek_malloc(_fileName);
	NewTek_FixFilename(fileName, fileName);

	// Do a little check to make sure this was done right
	char* firstSlash = fileName;
	if (parent && (parent != s_MyComputer))
	{
		const char* parentDir = parent->GetFileName();
		int parentLen = strlen(parentDir);
		int newLen = strlen(fileName);
		if (newLen <= parentLen) parent = NULL;
		else
		{
			char thisChar = *(fileName+parentLen);
			*(fileName+parentLen) = 0;
			if (stricmp(fileName, parentDir)) parentDir = NULL;
			else firstSlash = fileName+parentLen;
			*(fileName+parentLen) = thisChar;
		}
	}
	if (!parent) parent = s_MyComputer;

	// Find the parents all the way down the chain
	firstSlash = NewTek_GetFirstSlash(firstSlash+1);
	while (firstSlash)
	{
		firstSlash[0] = 0;
		// Assert: fileName is my parent directory
		// Look through the existing parent to see if this one is already in the list
		FileProperties* newParent = NULL;
		for (unsigned i = 0; i < parent->m_children.NoItems() && !newParent; i++)
		{
			if (!stricmp(fileName, parent->m_children[i]->GetFileName()))
				newParent = parent->m_children[i];
		}

		// Look to see if no parent was found
		if (!newParent)
		{
			newParent = NewFileProperties(fileName, parent);
			long status = newParent->GetFileStatus();
			if ((status != FileProperties_Drive) && (status != FileProperties_Directory))
			{
				char debugString[512];
				sprintf(debugString, "FileProperties::WARNING %s listed as a parent, not Directory OR Drive\n", fileName);
				OutputDebugString(debugString);	
			}
		}
		parent = newParent;

		*firstSlash = '\\';
		firstSlash = NewTek_GetFirstSlash(firstSlash+1);
	}

	// Try to find me in my parent
	FileProperties* ret = NULL;
	for (unsigned i = 0; i < parent->m_children.NoItems() && !ret; i++)
	{
		if (!stricmp(fileName, parent->m_children[i]->GetFileName()))
			ret = parent->m_children[i];
	}
	if (!ret) ret = NewFileProperties(fileName, parent);
	else
	{
		// See if ret is non-existant
		if (ret->GetFileStatus() == FileProperties_NoFile)
		{
			ret->m_status = FindFileStatus(fileName);
			if (ret->GetFileStatus() != FileProperties_NoFile)
			{
				ret->Changed(FileProperties_Changed_ReCreated);
				parent->Changed(FileProperties_Changed_FileEnteredDir, fileName);
			}
		}
		ret->RemoveFromDeletionList();
	}

	/* delete some of my old ones if I need to
	while (s_deletionListHead && (s_numProperties > s_maxProperties))
		delete s_deletionListHead;
		*/

	ret->m_numAccesses++;
	NewTek_free(fileName);
	return ret;
}

FileProperties* FileProperties::NewFileProperties(char* fileName, FileProperties* parent, long status)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (status == -1)
		status = FindFileStatus(fileName);
	if (status == FileProperties_MyComputer)
		status = FileProperties_NoFile;

	return new FileProperties(fileName, parent, status);
}


void FileProperties::ReleaseHandle(FileProperties* releaseMe)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!releaseMe) return;
	if (releaseMe->m_numAccesses) releaseMe->m_numAccesses--;
	if (!releaseMe->m_numAccesses && (releaseMe != s_MyComputer))
		releaseMe->AddToDeletionList();
}

const char*	FileProperties::GetFileName()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	return m_fullPath.Get();
}

const char*	FileProperties::GetParentDir()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (this->m_parentDir)
		return m_parentDir->GetFileName();
	else
		return NULL;
}

bool	FileProperties::FileMoveFile(const char* newFullPath)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Bad pointer
	if (!newFullPath) return false;

	// Check to see if we are moving to the same place
	if (!stricmp(newFullPath, (const char*)m_fullPath))
		return false;

	// If the file is being locked
	if (FP_IsLockedInToasterApp())
	{
		LockedInAppMessageBox("move");
		return false;
	}

	// If the file already exists, we need to delete it so we can place the new ones
	if (NewTek_DoesFileExist(newFullPath))
	{
		FileProperties* oldFP = GetHandle(newFullPath);
		tList<FileProperties*> fpList;
		fpList.Add(oldFP);
		if (!FileProperties::EditDelete(&fpList))
		{
			sprintf(s_messageString, TL_GetString( "TR::" "Could not overwrite the file:\n\"%s\"", "Could not overwrite the file:\n\"%s\"" ), newFullPath);
			NewTek_MessageBox(NewTek_GetGlobalParentHWND(),s_messageString,TL_GetString( "TR::" "Failed to Overwrite File!", "Failed to Overwrite File!" ), MB_OK, NULL);
			return false;
		}
	}

	// Write the file
	WriteInfoFile();

	//mwatkins - fixes 2.0 bug.  SDK doc says MoveFile can't be used to move folders between volumes.
	//				Caused many UI problems when the user tried.
	
	// See if the file moved properly
	bool movedOk = false;
	long status = GetFileStatus();
	if (status == FileProperties_Directory)
		movedOk = FileMoveFolder(newFullPath);
	else if (status == FileProperties_File)
	{
		movedOk = ::MoveFile((const char*)m_fullPath, newFullPath); 
		if (!movedOk)
		{
			DWORD error = GetLastError();
		}
	}

	if (movedOk)
	{
		// Do not send changes
		m_amIchanging = true;

		// We are no longer cut
		m_cut = false;

		// Move the supporting files
		FileProperties_MoveSupportingFiles((const char*)m_fullPath, newFullPath, true);
		
		char newInfo[MAX_PATH];
		// Finally Change my own filename
		strcpy(newInfo, (const char*)m_fullPath);
		// ASSERT: newInfo is now our old name
		m_fullPath.Set(newFullPath);

		// Reset all of my data if a dialog is listening to me
		if (this->m_numPropDLG)
			ResetFileProperties();

		// Send a changed message to let everyone know
		this->Changed(FileProperties_Changed_MovedFile, newInfo, (const char*)m_fullPath);

		// Switch parent directories around if necessary
		char newParentDir[MAX_PATH];
		strcpy(newParentDir, newFullPath);
		char* lastSlash = NewTek_GetLastSlash(newParentDir);
		if (lastSlash) *lastSlash = 0;
		if (stricmp(newParentDir, this->GetParentDir()))
		{
			FileProperties* newParent = FileProperties::GetHandle(newParentDir);	

			m_parentDir->Changed(FileProperties_Changed_FileLeftDir, newInfo);
			this->ChangeParent(newParent);
			m_parentDir->Changed(FileProperties_Changed_FileEnteredDir, newFullPath);
			newParent->ReleaseHandle();
		}

		m_amIchanging = false;
		return true;
	}
	return false;
}

//***************************************************************************************************************************************
//mwatkins
bool	FileProperties::EnsureAsyncMoveFile(char* newFullPath, bool bMultipleFiles, 
											FileProperties_Status fileType, bool &bReadOnlyMoveAll,
											bool &bReplaceAll)
{
	FunctionBlock ThreadSafe(g_AssetHandleLock);

	// Bad pointer
	if (!newFullPath) 
		return false;

	// Be sure we are not just going to the same place
	if (0 == ::stricmp(newFullPath, (const char*)m_fullPath))
		return false;

	// mwatkins - Don't ask if user wants to move read only files if in the middle of moving multiple files
	// and the user has already selected that he/she wants to move all read only files.
	bool bMoveReadOnly = true;
	if (!(bMultipleFiles && bReadOnlyMoveAll))
		bMoveReadOnly = this->EnsureMoveReadOnly(bMultipleFiles, bReadOnlyMoveAll);

	if (!bMoveReadOnly)	// Cancel
		return false;

	bool bReplace = FileProperties::EnsureOverwrite(newFullPath, bMultipleFiles, 
													bReplaceAll, fileType == FileProperties_File);

	if (!bReplace)
		return false;

	// If the file is being locked
	if (this->FP_IsLockedInToasterApp())
	{
		this->LockedInAppMessageBox("move");
		return false;
	}

	// If the file already exists, we need to delete it so we can place the new ones
	if (::NewTek_DoesFileExist(newFullPath))
	{
		FileProperties* oldFP = FileProperties::GetHandle(newFullPath);
		tList<FileProperties*> fpList;
		fpList.Add(oldFP);
		if (!FileProperties::EditDelete(&fpList))
		{
			::sprintf(s_messageString, TL_GetString( "TR::" "Could not overwrite the file:\n\"%s\"", "Could not overwrite the file:\n\"%s\"" ), newFullPath);
			::NewTek_MessageBox(::NewTek_GetGlobalParentHWND(),s_messageString,TL_GetString( "TR::" "Failed to Overwrite File!", "Failed to Overwrite File!" ), MB_OK, NULL);
			return false;
		}
	}

	return true;
}

//***************************************************************************************************************************************
//mwatkins
void	FileProperties::FileAsyncMovedFile(char* newFullPath)
{
	//! Do not send changes
	m_amIchanging = true;
	
	//! We are no longer cut
	m_cut = false;
	
	//! Move the supporting files
	::FileProperties_MoveSupportingFiles((const char*)m_fullPath, newFullPath, true);

	// Broadcast the move (used by path layout control on top of file bin)
	this->Changed(FileProperties_Changed_MovedFile, (const char*)m_fullPath, newFullPath);

	//! Extract the path to the parent
	char szDestParentDir[MAX_PATH] = {0};
	::strcpy(szDestParentDir, newFullPath);
	char* lastSlash = ::NewTek_GetLastSlash(szDestParentDir);
	if (lastSlash) 
		*lastSlash = 0;

	//! Get a handle to the new parent's file properties
	FileProperties* destParentFP = FileProperties::GetHandle(szDestParentDir);

	//! try and find the child file props for the new destination path in the parent
	/*! this will be found if the polling of the destination parent directory already detected
		the new file while it was in the process of being written */
	FileProperties* destFP = NULL;
	for (unsigned i = 0; i < destParentFP->m_children.NoItems(); i++)
	{
		if (0 == ::stricmp(newFullPath, destParentFP->m_children[i]->GetFileName()))
		{
			destFP = destParentFP->m_children[i];
			break;
		}
	}

	char szOldFullPath[MAX_PATH] = {0};
	::strcpy(szOldFullPath, (const char*)m_fullPath);

	//! avoid duplicate file properties in the destination's parent
	//! if duplicates, just try and clean the old one up as best we can
	bool bDoAvoidDuplicateFP = destFP && (this != destFP);
	if (bDoAvoidDuplicateFP)
	{
		m_status = FileProperties_NoFile;

		if (m_numPropDLG)
			this->ResetFileProperties();
	}
	else
	{
		//! Finally Change my own filename
		m_fullPath.Set(newFullPath);
	}

	//! inform the old parents that we left
	m_parentDir->Changed(FileProperties_Changed_FileLeftDir, szOldFullPath);

	if (bDoAvoidDuplicateFP)
		this->ChangeParent(NULL);	//! do some more clean up
	else
		this->ChangeParent(destParentFP);

	//! inform the new parent that we arrived
	destParentFP->Changed(FileProperties_Changed_FileEnteredDir, newFullPath);

	//! if a child destination file properties already existed, it must be forced refreshed
	if (destFP)
	{
		destFP->ResetFileProperties(true);
		destFP->RefreshInterestedFileButtonIcons();
	}
	
	FileProperties::ReleaseHandle(destParentFP);
	destParentFP = NULL;
	
	m_amIchanging = false;
}

//***************************************************************************************************************************************
bool	FileProperties::DoesFileExist()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	return (m_status != FileProperties_NoFile);
}

//***************************************************************************************************************************************
bool	FileProperties::DoesInfoExist()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	return m_infoExists;
}

//***************************************************************************************************************************************
void FileProperties::RemoveFromDeletionList()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Remove this FileProps from the deletion list if it is there
	if (this->m_deletionListNext)
		this->m_deletionListNext->m_deletionListPrev = this->m_deletionListPrev;
	if (this->m_deletionListPrev)
		this->m_deletionListPrev->m_deletionListNext = this->m_deletionListNext;
	if (this == s_deletionListHead)
		s_deletionListHead = this->m_deletionListNext;
	if (this == s_deletionListTail)
		s_deletionListTail = this->m_deletionListPrev;
	this->m_deletionListNext = this->m_deletionListPrev = NULL;
}

//***************************************************************************************************************************************
void FileProperties::AddToDeletionList()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_deletionListPrev || m_deletionListNext || (s_deletionListTail == this)) return;	// Already in the list!

	// Add me to the deletion list
	if (!s_deletionListTail)
		s_deletionListTail = s_deletionListHead = this;
	else
	{
		this->m_deletionListPrev = s_deletionListTail;
		s_deletionListTail->m_deletionListNext = this;
		s_deletionListTail = this;
	}
}

//***************************************************************************************************************************************
FileProperties::FileProperties(char* fileName, FileProperties* parent, long status)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// I am not cut to start
	m_cut = false;

	// No one has accessed me
	m_numAccesses = 0;

	// I am not locked in a TOaster App
	m_toasterAppLocks = 0;

	// I am not in any properties dialogs
	m_numPropDLG = 0;

	// My properties are not yet generated
	m_propsGenerated = false;

	// I do want to write out info on close
	m_propsChanged = false;

	// This is the full path
	m_fullPath.Set(fileName);

	// I do want to listen to myself
	AddDependant(this, (long)this);

	// I am not changing anything yet
	m_amIchanging = false;

	// My color is clear
	m_colorAsset = NULL;
	m_colorTint.bgra = RGBA(255,255,255,0);

	// We are not in the deletion list
	m_deletionListPrev = m_deletionListNext = NULL;
	s_numProperties++;

	// I don't have an alt name yet
	m_altName.Set("");
	m_altNameRead = false;
	m_altNameChanged = false;
	m_altName.AddDependant(this, (long)(&m_altName));

	// Here is my parent
	m_parentDir = NULL;
	this->ChangeParent(parent);
	
	// We already know our status
	m_status = status;

	// Notify my parent of a new one added, If we are valid
	if (parent && (m_status != FileProperties_NoFile))
		parent->Changed(FileProperties_Changed_FileEnteredDir, fileName);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
FileProperties* FileProperties::GetParentHandle()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_parentDir)
		m_parentDir->GetHandle();
	return m_parentDir;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::GenerateFileProperties()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!m_propsGenerated)
	{
		// We have done all we can to write the file out
		m_propsChanged = m_altNameChanged = false;
		m_propsGenerated = true;
		m_amIchanging = true;

		// First try to read in the info file
		char infoFile[MAX_PATH];
		GetInfoFromFileName((const char*)m_fullPath, infoFile);
		ScriptStructure* script = ScriptStructure::NewScriptStructure();
		char* readError = script->ReadFromFile(infoFile);
		if (readError)
			m_infoExists = false;
		else
		{
			this->AssetList_ReadFromStructure(*script);
			m_infoExists = true;
			m_altNameRead = true;
		}
		ScriptStructure::DeleteScriptStructure(script);
		m_amIchanging = false;

		// Create the Color
		if (m_infoExists && !m_colorAsset)
		{
			bool newAsset = AddAsset(m_colorAsset, "Color Tint", "Color", true, false, false);
			DynamicGamut* myGamut = GetInterface<DynamicGamut>(m_colorAsset->GetProperty());
			if (myGamut)
			{
				if (newAsset)
					myGamut->SetRGBA(m_colorTint.r, m_colorTint.g, m_colorTint.b, m_colorTint.a);
				else
				{
					WindowPixel wp = myGamut->GetWindowPixel();
					if (wp.bgra != m_colorTint.bgra)
					{
						m_colorTint.bgra = wp.bgra;
						Changed(FileProperties_Changed_ColorTint);
					}
				}
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
DynamicString* FileProperties::GetAltFilenameDynamic( void )
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!m_altNameRead) GenerateFileProperties();
	return &m_altName;
}

const char* FileProperties::GetAltFilename( void )
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!m_altNameRead) GenerateFileProperties();
	return m_altName.Get();
}

WindowPixel	FileProperties::GetTintColor( void )
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!m_altNameRead) GenerateFileProperties();
	return m_colorTint;
}

void FileProperties::SetTintColor(unsigned newColor)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_colorTint.bgra != newColor)
	{
		m_colorTint.bgra = newColor;
		this->m_amIchanging = true;
		if (!m_colorAsset) m_colorAsset = AddAsset("Color Tint", "Color", true, false, false);
		DynamicGamut* myGamut = GetInterface<DynamicGamut>(m_colorAsset->GetProperty());
		this->m_amIchanging = false;
		if (myGamut)
			myGamut->SetRGBA(m_colorTint.r, m_colorTint.g, m_colorTint.b, m_colorTint.a);
		else
			Changed(FileProperties_Changed_ColorTint);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::GetDirectorySize( const char* dirName, unsigned long & highSize, unsigned long & lowSize)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	BreakInt64(GetDirectorySize(dirName), highSize, lowSize);
}

unsigned __int64 FileProperties::GetDirectorySize( const char* dirName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	char searchPath[MAX_PATH];
	sprintf(searchPath, "%s\\*", dirName);
	unsigned __int64 size = 0;

	/* TODO: Put Directory Sizing on a thread
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	bool foundNext = true;
	while ((findHandle != INVALID_HANDLE_VALUE) && foundNext)
	{
		if (findData.cFileName[0] != '.')
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				char thisDir[MAX_PATH];
				sprintf(thisDir, "%s\\%s", dirName, findData.cFileName);
				size += GetDirectorySize(thisDir);
			}
			else
				size += GetInt64(findData.nFileSizeHigh, findData.nFileSizeLow);
		}
		foundNext = FindNextFile(findHandle, &findData);
	}
	FindClose(findHandle);
	*/
	return size;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::AddToDialog()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	m_numPropDLG++;
	if (m_numPropDLG == 1)
		ResetFileProperties();	
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::RemoveFromDialog()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_numPropDLG > 0)
		m_numPropDLG--;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::GetCommonAssets(WIN32_FIND_DATA findData)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Create the Color
	if (!m_colorAsset)
	{
		bool newAsset = AddAsset(m_colorAsset, "Color Tint", "Color", true, false, false);
		DynamicGamut* myGamut = GetInterface<DynamicGamut>(m_colorAsset->GetProperty());
		if (myGamut)
		{
			if (newAsset)
				myGamut->SetRGBA(m_colorTint.r, m_colorTint.g, m_colorTint.b, m_colorTint.a);
			else
			{
				WindowPixel wp = myGamut->GetWindowPixel();
				if (wp.bgra != m_colorTint.bgra)
				{
					m_colorTint.bgra = wp.bgra;
					Changed(FileProperties_Changed_ColorTint);
				}
			}
		}
	}
	
	if ((m_status == FileProperties_Directory) || (m_status == FileProperties_File))
	{
		if (m_status == FileProperties_Directory)
			GetDirectorySize((const char*)m_fullPath, findData.nFileSizeHigh, findData.nFileSizeLow);

		// Created
		AssetElement* created = AddAsset("Created", "Date/Time String", true, true, false);
		AssetDateTimeString* dateCreated = GetInterface<AssetDateTimeString>(created->GetProperty());
		if (dateCreated)
			dateCreated->SetDateTimeString(&(findData.ftCreationTime));

		// Modified
		AssetElement* modified = AddAsset("Modified", "Date/Time String", true, true, false);
		AssetDateTimeString* dateModified = GetInterface<AssetDateTimeString>(modified->GetProperty());
		if (dateModified)
			dateModified->SetDateTimeString(&(findData.ftLastWriteTime));

		// Last Access
		AssetElement* accessed = AddAsset("Last Accessed", "Date/Time String", true, true, false);
		AssetDateTimeString* dateAccessed = GetInterface<AssetDateTimeString>(accessed->GetProperty());
		if (dateAccessed)
			dateAccessed->SetDateTimeString(&(findData.ftLastAccessTime));

		// File Size
		AssetElement* filesize = AddAsset("Size", "File Size String", true, true, false);
		AssetFileSizeString* sizeAsset = GetInterface<AssetFileSizeString>(filesize->GetProperty());
		if (sizeAsset)
			sizeAsset->SetFileSizeString(GetInt64(findData.nFileSizeHigh, findData.nFileSizeLow));

		// Dos 8.3
		AssetElement* dos = AddAsset("DOS 8.3 filename", "String", true, true, false);
		AssetString* dosName = GetInterface<AssetString>(dos->GetProperty());
		if (dosName)
			dosName->SetText(findData.cAlternateFileName);
		
		// Hidden
		AssetElement* hidden = AddAsset("Hidden", "Boolean", true, false, false);
		AssetBool* hiddenAsset = GetInterface<AssetBool>(hidden->GetProperty());
		if (hiddenAsset)
			hiddenAsset->Set(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN);
		
		// Read Only
		AssetElement* readonly = AddAsset("Read Only", "Boolean", true, false, false);
		AssetBool* readOnlyAsset = GetInterface<AssetBool>(readonly->GetProperty());
		if (readOnlyAsset)
			readOnlyAsset->Set(findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::ResetFileProperties(bool bForceReset)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// This function just Refreshes the Common Assets
	// If properties have not been generated, go ahead and do so.

	//mwatkins - added to really, really reset the file properties
	if (bForceReset)
	{
		this->DeleteAllAssets(false);
		m_colorAsset = NULL;
		m_colorTint.bgra = RGBA(255,255,255,0);
		m_propsGenerated = false;
	}

	if (!m_propsGenerated)
		GenerateFileProperties();

	m_amIchanging = true;

	// Get the file information
	WIN32_FIND_DATA findData;
	const char* fileNameToUse = (const char*)m_fullPath;
	if ((m_status == FileProperties_MyComputer) || (m_status == FileProperties_Drive))
		fileNameToUse = "";

	HANDLE findHandle = FindFirstFile(fileNameToUse, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		long oldStatus = m_status;
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			m_status = FileProperties_Directory;
		else m_status = FileProperties_File;
		if (!oldStatus && m_parentDir)
			m_parentDir->Changed(FileProperties_Changed_FileEnteredDir, fileNameToUse);
	}
	else if ((m_status == FileProperties_Directory) || (m_status == FileProperties_File))
		m_status = FileProperties_NoFile;

	// Generate and store the common file assets
	GetCommonAssets(findData);

	FindClose(findHandle);
	m_amIchanging = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
FileProperties::~FileProperties()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	WriteInfoFile();

	// I no longer need my parent
	ChangeParent(NULL);

	// I am no longer dependant on myself or anything else
	RemoveAllDependantsToThis();

	// Delete all of my children (I make a copy because the children will be removed from my list)
	tList<FileProperties*> children;
	children.CopyFrom(&this->m_children);
	for (unsigned i = 0; i < children.NoItems(); i++)
		delete children[i];

	// Do the Deletion list stuff
	s_numProperties--;
	RemoveFromDeletionList();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::ChangeParent(FileProperties* newParent)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// PEDANTIC Check
	if (m_parentDir == newParent) return;

	if (m_parentDir)
	{
		if (m_parentDir->GetFileStatus() == FileProperties_Directory)
			m_parentDir->DeleteDependant(this);
		m_parentDir->m_children.Delete(this);
		if (!m_parentDir->m_children.NoItems() && !m_parentDir->m_numAccesses && (m_parentDir != s_MyComputer))
			m_parentDir->AddToDeletionList();	
	}
	m_parentDir = newParent;
	if (m_parentDir)
	{
		// I should be in my parent's list
		m_parentDir->m_children.Add(this);

		// Add a dependency to this parent if it is OK
		long parentStatus = m_parentDir->GetFileStatus();
		if (parentStatus == FileProperties_Directory)
			m_parentDir->AddDependant(this);
	}
	else if (m_parentDir != s_MyComputer)
		AddToDeletionList();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileProperties::WriteInfoFile()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if(!m_propsChanged && !m_altNameChanged) return;

	// Write out the Info File
	char infoFile[MAX_PATH];
	if ( !GetInfoFromFileName((const char*)m_fullPath, infoFile) )
	{	char NewFolder[MAX_PATH*2];
		strcpy(NewFolder,infoFile);
		char *LastSlash = NewTek_GetLastSlash(NewFolder);
		if (LastSlash) LastSlash[0]=0;
		CreateDirectory( NewFolder , NULL );
	}

	ScriptStructure* ss = ScriptStructure::NewScriptStructure();
	AssetList_WriteToStructure(*ss);
	ss->WriteToFile(infoFile);
	m_propsChanged = m_altNameChanged = false;
	ScriptStructure::DeleteScriptStructure(ss);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileProperties::AmICut()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	return (DoesFileExist() && m_cut);
}
void FileProperties::SetCut(bool cut)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (cut != m_cut)
	{
		m_cut = cut;
		Changed(FileProperties_Changed_Cut, m_cut);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

bool	FileProperties::FileChangePath(const char* newPath, bool incrementIfExists)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!newPath) return false;
	const char* nameOnly = NewTek_GetFileNameFromPath((char*)(const char*)m_fullPath);
	if (!nameOnly || (nameOnly == (const char*)m_fullPath)) return false;
	char newFilename[MAX_PATH];
	sprintf(newFilename, "%s\\%s", newPath, nameOnly);
	
	// Has the Filename not changed?
	if (!stricmp(newFilename, (const char*)m_fullPath))
		return true;

	return FileMoveFile(incrementIfExists ? GetNextAvailableFileName(newFilename) : newFilename);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::FileChangeFilename(const char* newFilename, bool incrementIfExists)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!newFilename) return false;
	// Has the filename not changed
	char *CurrentName=NewTek_GetLastSlash((char*)GetFileName());
	if (CurrentName)
	{	CurrentName++;
		if (!strcmp(newFilename,CurrentName)) 
			return true;
	}
	char pathOnly[MAX_PATH]; strcpy(pathOnly, (const char*)m_fullPath);
	char* lastSlash = NewTek_GetLastSlash(pathOnly);
	if (!lastSlash) return false;
	*lastSlash = 0;
	char newFullPath[MAX_PATH];
	sprintf(newFullPath, "%s\\%s", pathOnly, newFilename);
	*lastSlash = '\\';

	// If the user renamed the file without extension, then we really should keep
	// the extension of the file as it was.
	if (!NewTek_GetLastDot(newFilename))
	{	char *pPrevExt = NewTek_GetLastDot(CurrentName);
		if (pPrevExt)
			strcat( newFullPath , pPrevExt );
	}

	return FileMoveFile(incrementIfExists ? GetNextAvailableFileName(newFullPath) : newFullPath);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileProperties::FileChangeAltName(const char* newAltName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!newAltName) newAltName = "";
	this->m_altName.Set(newAltName);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::FileCopyFolder(const char* newFullPath)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Try creating a folder in this new path
	CreateDirectory(newFullPath, NULL);

	// Now that we have a directory, iterate through each element in my own directory and move it
	char searchPath[MAX_PATH];
	char thisFile[MAX_PATH];
	const char* p_directory = GetFileName();
	sprintf(searchPath, "%s\\*", p_directory);

	// Create the list of items that need to be copied
	tList<FileProperties*> fileList;
	tList<char*> copyToList;
	
	// Here is where the search iteration is done
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		bool foundValid = true;
		do
		{
			while( foundValid && findData.cFileName[0] == '.')
			{
				if (!FindNextFile(findHandle, &findData ))
					foundValid = false;
			}
			if (foundValid)
			{
				sprintf(thisFile, "%s\\%s", p_directory, findData.cFileName);
				FileProperties* fp = FileProperties::GetHandle(thisFile, this);
				fileList.Add(fp);
				char* copyToFile = new char[strlen(newFullPath) + strlen(findData.cFileName) + 2];
				sprintf(copyToFile, "%s\\%s", newFullPath, findData.cFileName);
				copyToList.Add(copyToFile);
			}
		}while (foundValid && FindNextFile(findHandle, &findData));
	}
	FindClose(findHandle);

	// Now we have a list of FileProperties and a place to copy them
	bool bCopySuccessful = true;
	for (unsigned i = 0; i < fileList.NoItems(); i++)
	{
		// Do the copy
		if (bCopySuccessful)
			bCopySuccessful = fileList[i]->FileCopyFile(copyToList[i]);

		// Release the handle on the file
		fileList[i]->ReleaseHandle();
		fileList[i] = NULL;

		// Delete the dynamically allocated string
		delete[] copyToList[i];
		copyToList[i] = NULL;
	}

	// We were succesful
	return bCopySuccessful;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//mwatkins - fixes 2.0 bug.  SDK doc says MoveFile can't be used to move folders between volumes.
bool	FileProperties::FileMoveFolder(const char* newFullPath)
{	
	FunctionBlock ThreadSafe(g_AssetHandleLock);

	if (::ArePathsOnSameVolume(GetFileName(), newFullPath))
	{
		BOOL bOSMoveSuccess = ::MoveFile(GetFileName(), newFullPath);
		return (FALSE != bOSMoveSuccess);
	}
	
	// Try creating a folder in this new path
	CreateDirectory(newFullPath, NULL);

	// Now that we have a directory, iterate through each element in my own directory and move it
	char searchPath[MAX_PATH];
	char thisFile[MAX_PATH];
	const char* p_directory = GetFileName();
	sprintf(searchPath, "%s\\*", p_directory);

	// Create the list of items that need to be copied
	tList<FileProperties*> fileList;
	tList<char*> copyToList;
	
	// Here is where the search iteration is done
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		bool foundValid = true;
		do
		{
			while( foundValid && findData.cFileName[0] == '.')
			{
				if (!FindNextFile(findHandle, &findData ))
					foundValid = false;
			}
			if (foundValid)
			{
				sprintf(thisFile, "%s\\%s", p_directory, findData.cFileName);
				FileProperties* fp = FileProperties::GetHandle(thisFile, this);
				fileList.Add(fp);
				char* copyToFile = new char[strlen(newFullPath) + strlen(findData.cFileName) + 2];
				sprintf(copyToFile, "%s\\%s", newFullPath, findData.cFileName);
				copyToList.Add(copyToFile);
			}
		}while (foundValid && FindNextFile(findHandle, &findData));
	}
	FindClose(findHandle);

	// Now we have a list of FileProperties and a place to copy them
	bool bMoveSuccessful = true;
	for (unsigned i = 0; i < fileList.NoItems(); i++)
	{
		// Do the copy
		if (bMoveSuccessful)
			bMoveSuccessful = fileList[i]->FileMoveFile(copyToList[i]);

		// Release the handle on the file
		fileList[i]->ReleaseHandle();
		fileList[i] = NULL;

		// Delete the dynamically allocated string
		delete[] copyToList[i];
		copyToList[i] = NULL;
	}

	if (bMoveSuccessful)
		bMoveSuccessful = ::NewTek_DeleteFile(GetFileName(), false);//don't recycle

	return bMoveSuccessful;

	/*
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	
	bool bMoveSuccessful = false;

	if (FileCopyFolder(newFullPath))
		bMoveSuccessful = ::NewTek_DeleteFile(GetFileName(), false);//don't recycle
	
	return bMoveSuccessful;
	*/
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::FileCopyFile(const char* newFullPath)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Bad pointer
	if (!newFullPath) return false;

	// Write the file
	WriteInfoFile();

	// Find a new path that does not already exist if the names are the same
	const char* thisFile = GetFileName();
	if (!strcmp(newFullPath, thisFile))
		newFullPath = GetNextAvailableFileName(newFullPath);

	// If the file already exists, ask if we want to overwrite
	if (NewTek_DoesFileExist(newFullPath))
	{
		if (!FileProperties::_DeleteFile(newFullPath))
		{
			sprintf(s_messageString, TL_GetString( "TR::" "Could not overwrite the file:\n\"%s\"", "Could not overwrite the file:\n\"%s\"" ), newFullPath);
			NewTek_MessageBox(NewTek_GetGlobalParentHWND(),s_messageString,TL_GetString( "TR::" "Failed to Overwrite File!", "Failed to Overwrite File!" ), MB_OK, NULL);
			return false;
		}
	}

	// See if the file copies properly
	bool copiedOk = false;
	long status = GetFileStatus();
	if (status == FileProperties_Directory)
		copiedOk = FileCopyFolder(newFullPath);
	else if (status == FileProperties_File)
	{
		copiedOk = ::CopyFile(thisFile, newFullPath, false); 
		if (!copiedOk)
		{
			DWORD error = GetLastError();
		}
	}

	if (copiedOk)
	{
		// Do not send changes
		m_amIchanging = true;

		// We are no longer cut
		m_cut = false;

		// Copy the supporting files
		FileProperties_MoveSupportingFiles((const char*)m_fullPath, newFullPath, false);

		// Notify the new directory that we are coming
		char buffer[MAX_PATH]; strcpy(buffer, newFullPath); newFullPath = buffer;
		char* lastSlash = NewTek_GetLastSlash((char*)newFullPath);
		char* newNameOnly = NULL;
		if (lastSlash)
		{
			*lastSlash = 0;
			newNameOnly = lastSlash+1;
		}
		FileProperties* newDir = FileProperties::GetHandle(newFullPath);
		if (lastSlash) *lastSlash = '\\';
		newDir->Changed(FileProperties_Changed_FileEnteredDir, newFullPath);
		FileProperties::ReleaseHandle(newDir);
		newDir = NULL;

		m_amIchanging = false;
		return true;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//mwatkins
void	FileProperties::FileAsyncCopiedFile(char* newFullPath)
{
	// Do not send changes
	m_amIchanging = true;
	
	// We are no longer cut
	m_cut = false;
	
	// Copy the supporting files
	::FileProperties_MoveSupportingFiles((const char*)m_fullPath, newFullPath, false);
	
	// Notify the new directory that we are coming
	char parentDirPath[MAX_PATH] = {0};
	::strcpy(parentDirPath, newFullPath);
	char* lastSlash = ::NewTek_GetLastSlash(parentDirPath);
	char* newNameOnly = NULL;
	if (lastSlash)
	{
		*lastSlash = 0;
		newNameOnly = lastSlash+1;
	}
	FileProperties* newDir = FileProperties::GetHandle(parentDirPath);
	newDir->Changed(FileProperties_Changed_FileEnteredDir, newFullPath);
	FileProperties::ReleaseHandle(newDir);
	newDir = NULL;

	// Notify the new file button to force update
	FileProperties* newFP = FileProperties::GetHandle(newFullPath);
	newFP->ResetFileProperties(true);
	newFP->RefreshInterestedFileButtonIcons();
	FileProperties::ReleaseHandle(newFP);
	newFP = NULL;
	
	m_amIchanging = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::FileDeleteFile(bool deleteInfo, bool recycle)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	bool ret = false;
	m_amIchanging = true;

	// See if the file exists at all
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile((const char*)m_fullPath, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		// Close the handle so delete is OK
		FindClose(findHandle);

		// If the file is being locked
		if (FP_IsLockedInToasterApp())
		{
			LockedInAppMessageBox("delete");
			ret = false;
		}
		else //Delete the file permanently or recycle
			ret = NewTek_DeleteFile((const char*)m_fullPath, recycle);
	}
	
	if (ret)
	{
		m_status = FileProperties_NoFile;

		// Reset all of my data if a dialog is listening to me
		if (this->m_numPropDLG)
			ResetFileProperties();

		// Notify the previous directory that we have left
		if (this->m_parentDir)
		{
			m_parentDir->Changed(FileProperties_Changed_FileLeftDir, (const char*)m_fullPath);
			ChangeParent(NULL);
		}

		// delete the info and icons
		if (deleteInfo)
		{
			// Delete the supporting files
			FileProperties_MoveSupportingFiles((const char*)m_fullPath, NULL, true);
			this->Changed(FileProperties_Changed_FileDeletedInfo);
		}
		else 
			Changed(FileProperties_Changed_FileDeletedNotInfo);
	}

	m_amIchanging = false;
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void RecursiveSetFileAttributes(const char* fileName, bool readOnly, bool hidden)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	DWORD attributes = GetFileAttributes(fileName);
	if (attributes < 0xffffffff)
	{
		// If this is a folder, recursively scan it
		if (attributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// TODO: Do the Recursive scan
		}

		DWORD newAttr = attributes;
		if (!hidden)
			newAttr &= ~FILE_ATTRIBUTE_HIDDEN;
		else newAttr |= FILE_ATTRIBUTE_HIDDEN;
		if (!readOnly)
			newAttr &= ~FILE_ATTRIBUTE_READONLY;
		else newAttr |= FILE_ATTRIBUTE_READONLY;
		if (newAttr != attributes)
			SetFileAttributes(fileName, newAttr);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_amIchanging) return;
	if (ID == (long)(&m_altName))
	{
		m_altNameChanged = true;
		Changed(FileProperties_Changed_AltName, (const char*)m_altName);
	}
	else if (ItemChanging == m_parentDir)
	{
		if (!stricmp(FileProperties_Changed_MovedFile, String))
		{
			// I have been moved, I need to change my filename
			// Getting just the file name and path
			char oldPath[MAX_PATH];
			strcpy(oldPath, GetFileName());
			char* nameOnly = NewTek_GetFileNameFromPath(oldPath);
			if (!nameOnly) return;
			char newFullPath[MAX_PATH];
			sprintf(newFullPath, "%s\\%s", m_parentDir->GetFileName(), nameOnly);
			this->m_fullPath.Set(newFullPath);
			Changed(FileProperties_Changed_MovedFile, oldPath, newFullPath);
		}
		if (!stricmp(String, FileProperties_Changed_ColorTint))
		{
			// My Parents' color tint changed, if I am clear, pass the change along
			WindowPixel colorTint = GetTintColor();
			if (colorTint.a == 0)
				Changed(FileProperties_Changed_ColorTint);
		}
	}
	else if (ID == (long)this)
	{
		// Listen for things entering or leaving so I can re-calc my size
		if ((!stricmp(FileProperties_Changed_FileLeftDir, String)) ||
			(!stricmp(FileProperties_Changed_FileEnteredDir, String)))
		{
			// I need to recalculate my file size
			this->ResetFileSize();
		}
		else if (!stricmp(String, AssetList_Changed_ElementAdded))
		{
			// We have recorded a new element, we may need to write it out
			void* Args = args;
			AssetElement* ae=NewTek_GetArguement<AssetElement*>(Args);
			if (ae)
			{
				// Only set the changed flag if ae will be written to the file
				m_propsChanged |= (ae->m_writeToFile);
			}
		}
	}
	else
	{
		AssetElement* ae = (AssetElement*)ID;
		if (this->m_assetTable.Exists(ae))
		{
			if (IsDeletion(String))
			{
				if (ae == m_colorAsset)
					m_colorAsset = NULL;
				m_assetTable.DeleteInOrder(ae);
			}
			else if (m_propsGenerated)
			{
				AssetElement* hiddenElement = GetAsset("Hidden");
				AssetElement* readOnlyElement = GetAsset("Read Only");
				if ((hiddenElement && (ae == hiddenElement)) ||
					(readOnlyElement && (ae == readOnlyElement)))
				{
					// Do the internal file modification stuff
					bool hidden = false;
					bool readOnly = false;
					if (hiddenElement)
					{
						AssetBool* asset = GetInterface<AssetBool>(hiddenElement->GetProperty());
						if (asset)
							hidden = asset->Get();
						else hidden = false;
					}
					
					if (readOnlyElement)
					{
						AssetBool* asset = GetInterface<AssetBool>(readOnlyElement->GetProperty());
						if (asset)
							readOnly = asset->Get();
						else readOnly = false;
					}
					if (hiddenElement || readOnlyElement)
						RecursiveSetFileAttributes( GetFileName(), readOnly, hidden);
				}
			}
		
			// Watch for a change in my color tint
			if (ae == this->m_colorAsset)
			{
				DynamicGamut* myGamut = GetInterface<DynamicGamut>(m_colorAsset->GetProperty());
				if (myGamut)
				{
					ae->m_writeToFile = true;
					m_colorTint = myGamut->GetWindowPixel();
					Changed(FileProperties_Changed_ColorTint);
				}
			}

			// Only set the changed flag if ae will be written to the file
			m_propsChanged |= (ae->m_writeToFile);

			// Pass along the changed message
			Changed(AssetList_Changed_Properties, (AssetElement*)ID);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::ResetFileSize()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (m_status == FileProperties_Directory)
	{
		AssetElement* filesize = GetAsset("Size");
		if (filesize)
		{
			AssetFileSizeString* sizeAsset = GetInterface<AssetFileSizeString>(filesize->GetProperty());
			if (sizeAsset)
				sizeAsset->SetFileSizeString(GetDirectorySize((const char*)m_fullPath));
		}
	}

	// My parent has changed size and it might need to re-calc;
	if (this->m_parentDir)
		this->m_parentDir->ResetFileSize();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileProperties::AssetList_WriteToStructure(ScriptStructure & scriptStructure) 
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	scriptStructure.SetBlockMarker(0);
	// Create the Alt name
	ScriptStructure* altNameStruct = scriptStructure.CreateChildStructure();
	altNameStruct->SetBlockMarker('[');
	altNameStruct->m_writeEndlAfter = true;
	altNameStruct->CreateChildStructure()->SetString( "Alternate Name" );
	altNameStruct->CreateChildStructure()->SetString( "=" );
	altNameStruct->CreateChildStructure()->SetString( (char*)m_altName.Get() );

	// If we are writing anyway, go ahead and write my color
	if (m_colorAsset)
	{
		DynamicGamut* myGamut = GetInterface<DynamicGamut>(m_colorAsset->GetProperty());
		if (myGamut) m_colorAsset->m_writeToFile = true;
	}

	ScriptStructure* elements = scriptStructure.CreateChildStructure();
	elements->SetBlockMarker('{');
	AssetList::AssetList_WriteToStructure(*elements);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileProperties::AssetList_ReadFromStructure(ScriptStructure & scriptStructure)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Get the Asset name from the third element of the first element
	char* newAltName = (*scriptStructure[0])[2]->GetString();
	this->m_altName.Set(newAltName);
	AssetList::AssetList_ReadFromStructure(*scriptStructure[1]);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::EditCut(tList<FileProperties*>* fileNames)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames || !fileNames->NoItems()) return false;

	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to Cut.", "No files selected to Cut." ), TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
		return false;
	}

	// Clear the list
	s_editList->ClearList();
	
	// Set the state to cut
	s_editList->SetCutting(true);

	// Add the selected children names to the list
	for (unsigned i = 0; i < fileNames->NoItems(); i++)
		s_editList->AddToList((*fileNames)[i]);

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::EditCopy(tList<FileProperties*>* fileNames)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames || !fileNames->NoItems()) return false;

	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to Copy.", "No files selected to Copy." ), 
			TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
		return false;
	}

	// Clear the list
	s_editList->ClearList();
	
	// Set the state to cut
	s_editList->SetCutting(false);

	// Add the selected children names to the list
	for (unsigned i = 0; i < fileNames->NoItems(); i++)
		s_editList->AddToList((*fileNames)[i]);

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::EditPaste( const char* destination )
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	bool ret;
	NewTek_UIThreadIsBusy_Inst Thinking;

	//mwatkins - do the async dance for 2.5
	if (s_editList->AmICutting())	// If I am cutting, I want to move, if not, I want to copy
		ret = MoveToDirectory(&s_editList->m_editList, destination, true/*async it*/);
	else
		ret = CopyToDirectory(&s_editList->m_editList, destination, true/*do the async boogie*/);

	s_editList->SetCutting(false);
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool	FileProperties::EditDelete(tList<FileProperties*>* fileNames)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames || !fileNames->NoItems()) return false;
	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to Delete.", "No files selected to Delete." ), 
			TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
		return false;
	}
	else
	{
		// Check to see if we want to recycle(0), delete(1), or confirm(2)
		// Set up the combo box for PReferences
			static ComboItem CONFIRMDELETE_ComboItems[]=
			{	{ "Always Confirm Deleting Files","Ask", 2},
				{ "Automatically Recycle Files", "Recycle", 0},
				{ "Automatically Delete Files", "Delete", 1},
			};
			tList<ComboItem> l_ComboItem_Input;
			unsigned i=0;
			for(i=0;i<3;i++)
				l_ComboItem_Input.Add(CONFIRMDELETE_ComboItems[i]);
		
		// Find out what preferences has to say
			AssetList* al = Preferences::AssetList_GetDefault();
			int howToDelete = al->GetValue_Int("File Management, Confirm Deletion of Files",2,&l_ComboItem_Input);

		// Put up a Message box
		if (howToDelete == 2)
		{
			char* title = "Confirm Deletion";
			if (fileNames->NoItems() == 1)
			{
				FileProperties* fp = (*fileNames)[0];
				// Read Only
				AssetElement* readonlyElement = fp->GetAsset("Read Only");
				AssetBool* readOnlyAsset = readonlyElement ? GetInterface<AssetBool>(readonlyElement->GetProperty()) : NULL;
				bool readOnly = readOnlyAsset ? readOnlyAsset->Get() : false;
				char* readOnlyString = readOnly ? "read only " : "";

				long status = fp->GetFileStatus();
				const char* fn = fp->GetFileName();
				if (status == FileProperties_File)
					sprintf(s_messageString, TL_GetString( "Are you sure you want to delete the %sfile \"%s\"?" ), readOnlyString, fn);
				else if (status == FileProperties_Directory)
					sprintf(s_messageString, TL_GetString( "Are you sure you want to delete the %sfolder \"%s\"?" ), readOnlyString, fn);
				else
				{
					sprintf(s_messageString, TL_GetString( TL_GetString( "TR::" "It is impossible to delete \"%s\"!", "It is impossible to delete \"%s\"!" ) ), fn);
					NewTek_MessageBox(NULL,
						s_messageString, TL_GetString( TL_GetString( "TR::" "Cannot Delete", "Cannot Delete" ) ), MB_OK, NULL);
					return false;
				}
			}
			else
			{
				sprintf(s_messageString, TL_GetString( TL_GetString( "TR::" "Are you sure you want to delete these %i items?", "Are you sure you want to delete these %i items?" ) ), fileNames->NoItems());
			}

			// Show the Message box
			tList<char*> buttonText;
			Dynamic<int> NoShowCheck;
			if (ApplicationInstance_Default())
			{
				buttonText.Add(TL_GetString( "Recycle" ));
				buttonText.Add(TL_GetString( "Delete" ));
				buttonText.Add(TL_GetString( "Cancel" ));
				howToDelete = NewTek_StartMessageBox(NULL, "", title, s_messageString, buttonText, 2, 0, &NoShowCheck);
				if ((howToDelete < 0) || (howToDelete > 2)) howToDelete = 2;
			}
			else
			{
				buttonText.Add(TL_GetString( "Delete" ));
				buttonText.Add(TL_GetString( "Cancel" ));
				howToDelete = NewTek_StartMessageBox(NULL, "", title, s_messageString, buttonText, 1, 0, &NoShowCheck);
				if ((howToDelete < 0) || (howToDelete > 1)) howToDelete = 1;

				howToDelete++;	// Map back to VT expected value
			}

			// Check for Do not show again and Cancel
			if (NoShowCheck.Get() && (howToDelete != 2))
				al->SetValue_Int("File Management, Confirm Deletion of Files", howToDelete);
			else if (howToDelete == 2)
				return false;
		}

		bool notRet = false;
		for (i = 0; i < fileNames->NoItems(); i++)
		{	NewTek_UIThreadIsBusy_Inst Thinking;
			bool deleted = (*fileNames)[i]->FileDeleteFile(true, howToDelete == 0);
			const char* fn = (*fileNames)[i]->GetFileName();
			if (!deleted && (howToDelete == 0))
			{
				// We tried to recycle, but it didn't work, Do We want to do a real delete?
				sprintf(s_messageString, TL_GetString( "TR::" "Failed to recycle \"%s\"\nDo you want to delete permanently?", "Failed to recycle \"%s\"\nDo you want to delete permanently?" ), fn);
				int tryAgain = NewTek_MessageBox
							(NULL, s_messageString,
							"Delete Permanently?", MB_YESNOCANCEL, NULL);
				if (tryAgain == IDCANCEL) return false;
				else if (tryAgain == IDYES)
					deleted = (*fileNames)[i]->FileDeleteFile(true, false);
				else
					deleted = notRet = true;
			}
			if (!deleted)
			{
				if (i == (fileNames->NoItems() - 1))
				{
					sprintf(s_messageString, TL_GetString( "TR::" "There was a problem deleting \"%s\"", "There was a problem deleting \"%s\"" ), fn);
					NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Failed to Delete File", "Failed to Delete File" ), MB_OK, NULL);
				}
				else
				{
					sprintf(s_messageString, TL_GetString( "TR::" "There was a problem deleting \"%s\"\n Do you want to continue?", "There was a problem deleting \"%s\"\n Do you want to continue?" ), fn);
					if (NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Failed to Delete File", "Failed to Delete File" ), MB_YESNO, NULL) == IDNO)
						return false;
				}
				notRet = true;
			}
		}
		return !notRet;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::EditProperties(tList<FileProperties*>* fileNames, BaseWindowClass* parent, long XPos, long YPos)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames) return;
	if (!parent) return;
	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to see properties.", "No files selected to see properties." ), TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
	}
	else
	{
		POINT pt;
		GetCursorPos(&pt);
		if (XPos < 0) XPos = pt.x;
		if (YPos < 0) YPos = pt.y;

		const char* title = "Multi-File";
		if (fileNames->NoItems() == 1)
		{
			const char* fileName = (*fileNames)[0]->GetFileName();
			title = NewTek_GetFileNameFromPath((char*)fileName);
			if (!title) title = fileName;
		}
		FilePropertiesDlg* dlg = GetInterface<FilePropertiesDlg>
			(NewTek_New("FilePropertiesDlg",parent->GetWindowHandle(),0,0,0,0,NULL,WS_POPUP|WS_VISIBLE|DS_CENTERMOUSE));
		if (dlg)
		{
			int Xres, Yres;
			dlg->GetDefaultSize(Xres, Yres);
			dlg->SetWindowSize(Xres, Yres);
			dlg->EditTheseFiles(fileNames);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
//mwatkins - added last param for async copy in 2.5
bool FileProperties::CopyToDirectory(tList<FileProperties*>* fileNames, const char* destination, bool bDoAsync)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames || !destination)
		return false;

	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to Copy.", "No files selected to Copy." ), 
			TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
		return false;
	}

	if (!EnsureDirectory(destination))
		return false;	// A problem with the destination

	// mwatkins - check the async flag for 2.5
	if (bDoAsync)
	{
		FileCopyMoveHandler::StartAsyncCopy(*fileNames, destination, false/*not a move*/);
		return true;	//although we don't know if everything was successful b/c it's being done on another thread
	}

	// The folder we are copying to
	char copyToPath[MAX_PATH];
	strcpy(copyToPath, destination);
	NewTek_StrCatSlashIfNeeded(copyToPath);
	char* appendHere = &(copyToPath[strlen(copyToPath)]);

	// Loop through the list of files
	bool notRet = false;
	bool yesALLReplace = false;
	for (unsigned i = 0; i < fileNames->NoItems(); i++)
	{
		// The file to copy
		FileProperties* copyMe = (*fileNames)[i];
		const char* fn = copyMe->GetFileName();
		const char* copyFile = NewTek_GetFileNameFromPath( (char*)fn );
		if (!copyFile || (copyFile == fn)) notRet = true;
		else
		{
			// Create the new full path
			sprintf(appendHere, copyFile);

			// BE sure this thing is a file or directory
			long fileStatus = copyMe->GetFileStatus();
			if ((fileStatus == FileProperties_File) || (fileStatus == FileProperties_Directory))
			{
				// Be sure we are not just going to the same place
				if (stricmp(copyToPath, copyMe->GetFileName()))
				{
					// Check to be sure we are not overwriting the path
					bool okToMove = EnsureOverwrite(copyToPath, (fileNames->NoItems() - i) > 1, 
						yesALLReplace, fileStatus == FileProperties_File);

					if (okToMove)
					{
						// Now try to do the Copy
						if (!copyMe->FileCopyFile(copyToPath))
						{
							// There was a problem with this file
							sprintf(s_messageString, TL_GetString( "TR::" "There was a problem moving the file \"%s\"", "There was a problem moving the file \"%s\"" ), copyMe->GetFileName());
							NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Cannot Move or Copy", "Cannot Move or Copy" ), MB_OK, NULL);
							notRet = true;	// We failed
						}
					}
					else if (yesALLReplace)	// Cancel
						return false;
				}
			}
			else
			{
				// There was a problem with this file
				sprintf(s_messageString, TL_GetString( "TR::" "You cannot move or copy \"%s\"", "You cannot move or copy \"%s\"" ), copyMe->GetFileName());
				NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Cannot Move or Copy", "Cannot Move or Copy" ), MB_OK, NULL);
				notRet = true;	// We failed
			}
		}
	}

	// Only returns true if every copy was succesful
	return (!notRet);
}
/////////////////////////////////////////////////////////////////////////////////////////
//mwatkins - added last param for async moving in 2.5
bool FileProperties::MoveToDirectory(tList<FileProperties*>* fileNames, const char* destination, bool bDoAsync)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	if (!fileNames || !destination)
		return false;

	if (!fileNames->NoItems())
	{
		// Show a MessageBox and leave
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "No files selected to Move.", "No files selected to Move." ), TL_GetString( "TR::" "No Files Selected", "No Files Selected" ), MB_OK, NULL);
		return false;
	}

	if (!EnsureDirectory(destination))
		return false;	// A problem with the destination

// mwatkins - check the async flag for 2.5
	if (bDoAsync)
	{
		FileCopyMoveHandler::StartAsyncCopy(*fileNames, destination, true/*move, baby, move!!!*/);
		return true;	//although we don't know if everything was successful b/c it's being done on another thread
	}

	// The folder we are copying to
	char copyToPath[MAX_PATH];
	strcpy(copyToPath, destination);
	NewTek_StrCatSlashIfNeeded(copyToPath);
	char* appendHere = &(copyToPath[strlen(copyToPath)]);

	// Loop through the list of files
	bool notRet = false;
	bool yesALLReplace = false;
	bool yesALLReadOnly = false;
	for (unsigned i = 0; i < fileNames->NoItems(); i++)
	{
		// The file to copy
		FileProperties* copyMe = (*fileNames)[i];
		const char* fn = copyMe->GetFileName();
		const char* copyFile = NewTek_GetFileNameFromPath((char*)fn);
		if (!copyFile || (copyFile == fn)) notRet = true;
		else
		{
			// Create the new full path
			sprintf(appendHere, copyFile);

			// BE sure this thing is a file or directory
			long fileStatus = copyMe->GetFileStatus();
			if ((fileStatus == FileProperties_File) || (fileStatus == FileProperties_Directory))
			{
				// Be sure we are not just going to the same place
				if (stricmp(copyToPath, copyMe->GetFileName()))
				{
					bool multiple = (fileNames->NoItems() - i) > 1;

					bool okReadOnly = copyMe->EnsureMoveReadOnly(multiple, yesALLReadOnly);

					if (!okReadOnly && yesALLReadOnly)	// Cancel
						return false;
					else if (okReadOnly)
					{
						// Check to be sure we are not overwriting the path
						okReadOnly = EnsureOverwrite(copyToPath, multiple, 
							yesALLReplace, fileStatus == FileProperties_File);

						if (!okReadOnly && yesALLReplace)	// Cancel
							return false;
					}

					if (okReadOnly)
					{
						// Now try to do the Copy
						if (!copyMe->FileMoveFile(copyToPath))
						{
							// There was a problem with this file
							sprintf(s_messageString, TL_GetString( "TR::" "There was a problem moving the file \"%s\"", "There was a problem moving the file \"%s\"" ), copyMe->GetFileName());
							NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Cannot Move or Copy", "Cannot Move or Copy" ), MB_OK, NULL);
							notRet = true;	// We failed
						}
					}
					else notRet = true;	// User said NO
				}
			}
			else
			{
				// There was a problem with this file
				sprintf(s_messageString, TL_GetString( "TR::" "You cannot move or copy \"%s\"", "You cannot move or copy \"%s\"" ), copyMe->GetFileName());
				NewTek_MessageBox(NULL, s_messageString, TL_GetString( "TR::" "Cannot Move or Copy", "Cannot Move or Copy" ), MB_OK, NULL);
				notRet = true;	// We failed
			}
		}
	}

	// Only returns true if every copy was succesful
	return (!notRet);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool FileProperties::EnsureDirectory(const char* directoryName)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Grab the status of this file
	long status = FindFileStatus(directoryName);

	// Check to see if an existing drive or folder and return true
	if ((status == FileProperties_Drive) || (status == FileProperties_Directory))
		return true;
	else if (status == FileProperties_NoFile)
	{
		// We don't have to keep asking recursively
		static int recursiveEnsureCalls = 0;
		int createDirectory = recursiveEnsureCalls ? IDYES : IDCANCEL;

		if (createDirectory==IDCANCEL)
		{
			// Set up the combo box for PReferences
				static ComboItem CREATEDIRECTORY_ComboItems[]=
				{	{ "Automatically Create Folders", "Yes", IDYES },
					{ "Always Confirm Folder Creation","Ask", IDCANCEL },
				};
				tList<ComboItem> l_ComboItem_Input;
				for(unsigned i=0;i<2;i++)
					l_ComboItem_Input.Add(CREATEDIRECTORY_ComboItems[i]);
			
			// Find out what preferences has to say
				AssetList* al = Preferences::AssetList_GetDefault();
				createDirectory = al->GetValue_Int("File Management, Automatically Create Folders",IDCANCEL,&l_ComboItem_Input);
			
			// If preferences says we should ask, then ASK
				if (createDirectory == IDCANCEL)
				{
					// Create the Message for this folder
					sprintf(s_messageString, TL_GetString( "TR::" "The folder: \"%s\"\nDoes not exist. Would you like to create it?", "The folder: \"%s\"\nDoes not exist. Would you like to create it?" ), directoryName);

					// Ask the user
						Dynamic<int> showAgain;
						createDirectory = NewTek_MessageBox
							(NULL, s_messageString,
							TL_GetString( "TR::" "Create Folder?", "Create Folder?" ), MB_YESNOCANCEL, &showAgain);

					// Does the User not want to see this again?
						if (showAgain.Get() && (createDirectory != IDCANCEL))
							al->SetValue_Int("File Management, Automatically Create Folders", IDYES);
				}
		}
		
		if (createDirectory==IDYES)
		{
			// We need to create a directory, but first be sure the parent exists, all the way to a drive
			char* thisDir = NewTek_GetFileNameFromPath( (char*)directoryName );
			if (thisDir && (thisDir != directoryName))
			{
				char thisChar = thisDir[0];
				*thisDir = 0;
				recursiveEnsureCalls++;
				bool goodParent = EnsureDirectory(directoryName);
				recursiveEnsureCalls--;
				*thisDir = thisChar;

				if (goodParent)
				{
					if (CreateDirectory(directoryName, NULL))
						return true;
					else
					{
						// Show a MessageBox and leave
						sprintf (s_messageString, TL_GetString( "TR::" "Could not create the directory:\n\"%s\"", "Could not create the directory:\n\"%s\"" ), directoryName);
						NewTek_MessageBox(NULL,
							s_messageString, TL_GetString( "TR::" "Error Creating Directory", "Error Creating Directory" ), MB_OK, NULL);
						return false;
					}
				}
			}
			else return true;
		}
		// The user decided not to do it
		else return false;
	}
	else
	{
		// Show a MessageBox and leave
		sprintf (s_messageString, TL_GetString( "TR::" "Invalid Directory or Drive:\n\"%s\"", "Invalid Directory or Drive:\n\"%s\"" ), directoryName);
		NewTek_MessageBox(NULL,
			s_messageString, TL_GetString( "TR::" "Invalid Directory", "Invalid Directory" ), MB_OK, NULL);
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool FileProperties::EnsureOverwrite(const char* overThisFile, bool multiple, bool & yesAll, bool fileOverwrites)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Check for a bad param
	if (!overThisFile)
	{
		yesAll = false;
		return false;
	}

	// Check for the existance of this file 
	long status = FindFileStatus(overThisFile);

	if (status == FileProperties_NoFile)
		return true;
	else if ((status == FileProperties_Drive) || (status == FileProperties_MyComputer))
	{
		// Show a MessageBox and leave
		sprintf (s_messageString, TL_GetString( "TR::" "Cannot overwrite:\"%s\"", "Cannot overwrite:\"%s\"" ), overThisFile);
		NewTek_MessageBox(NULL,
			s_messageString, TL_GetString( "TR::" "Error Overwriting", "Error Overwriting" ), MB_OK, NULL);
		yesAll = false;
		return false;
	}
	else if ((status == FileProperties_Directory) && fileOverwrites)
	{
		sprintf (s_messageString, TL_GetString( "TR::" "Are you sure you want to replace the directory:\n\"%s\"\nwith a file?", "Are you sure you want to replace the directory:\n\"%s\"\nwith a file?" ),overThisFile);
		int overFolder = NewTek_MessageBox(NULL, s_messageString,
			TL_GetString( "TR::" "Overwrite Folder?", "Overwrite Folder?" ), MB_YESNOCANCEL, NULL);
		if (overFolder == IDCANCEL)
		{
			yesAll = true;
			return false;
		}
		else if (overFolder == IDNO)
		{
			yesAll = false;
			return false;
		}
		else status = FileProperties_File;	// We will later try to delete just like a file
	}
	// We may need to overwrite this, should we ask?
	else if (!yesAll)
	{
		// Check the Preferences to see if we should even ask
		// Set up the combo box for PReferences
			static ComboItem CONFIRMREPLACE_ComboItems[]=
			{	{ "Always Confirm Replacement","Ask", IDYES },
				{ "Automatically Replace When Moving", "Yes", IDNO },
			};
			tList<ComboItem> l_ComboItem_Input;
			for(unsigned i=0;i<2;i++)
				l_ComboItem_Input.Add(CONFIRMREPLACE_ComboItems[i]);
		
		// Find out what preferences has to say
			AssetList* al = Preferences::AssetList_GetDefault();
			bool confirmReplace = (al->GetValue_Int("File Management, Confirm Replacement While Moving",IDYES,&l_ComboItem_Input) == IDYES);

		if (confirmReplace)
		{
			char* titleBar;
			// What string do we place here?
			if (status == FileProperties_Directory)
			{
				titleBar = TL_GetString( "TR::" "Confirm Folder Replace", "Confirm Folder Replace" );
				// We are replacing a directory with a directory
				sprintf(s_messageString, TL_GetString( "TR::" "The folder already contains a folder named \"%s\"\n\n\
					If the files in the existing folder have the same name as files in the\n\
					folder you are moving or copying, they will be replaced.  Do you still\n\
					want to move or copy the folder?", "The folder already contains a folder named \"%s\"\n\n\
					If the files in the existing folder have the same name as files in the\n\
					folder you are moving or copying, they will be replaced.  Do you still\n\
					want to move or copy the folder?" ), overThisFile);
			}
			else	// A file
			{
				titleBar = TL_GetString( "TR::" "Confirm File Replace", "Confirm File Replace" );
				// We are replacing a directory with a directory
				sprintf(s_messageString, TL_GetString( "TR::" "The folder already contains a file named \"%s\"\nDo you want to replace it?", "The folder already contains a file named \"%s\"\nDo you want to replace it?" ),
					overThisFile);
			}

			// What set of buttons will we use?
			UINT buttonSet = multiple ? MB_ALLYESNOCANCEL : MB_YESNOCANCEL;

			Dynamic<int> showAgain;
			int askUser = NewTek_MessageBox(NULL, s_messageString, titleBar, buttonSet, &showAgain);

			// Does the User not want to see this again?
			if (showAgain.Get() && (askUser != IDCANCEL))
				al->SetValue_Int("File Management, Confirm Replacement While Moving", IDNO);

			if (askUser == IDCANCEL)
			{
				yesAll = true;
				return false;
			}
			else if (askUser == IDALL)
				yesAll = true;
			else if (askUser == IDNO)
			{
				yesAll = false;
				return false;
			}
		}
	}

	// If we have made it this far, we need to delete if we have a file
	if (status == FileProperties_File)
		return _DeleteFile(overThisFile);
	else return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool FileProperties::EnsureMoveReadOnly(bool multiple, bool & yesAll)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Read Only
	AssetElement* readonly = GetAsset("Read Only");
	AssetBool* readOnlyAsset = readonly ? GetInterface<AssetBool>(readonly->GetProperty()) : NULL;
	if (readOnlyAsset && readOnlyAsset->Get())
	{
		// Check the Preferences to see if we should even ask
		// Set up the combo box for PReferences
			static ComboItem CONFIRMREADONLY_ComboItems[]=
			{	{ "Always Confirm Moving Read Only Files","Ask", IDYES },
				{ "Automatically Move Read Only Files", "Yes", IDNO },
			};
			tList<ComboItem> l_ComboItem_Input;
			for(unsigned i=0;i<2;i++)
				l_ComboItem_Input.Add(CONFIRMREADONLY_ComboItems[i]);
		
		// Find out what preferences has to say
			AssetList* al = Preferences::AssetList_GetDefault();
			bool confirmReplace = (al->GetValue_Int("File Management, Confirm Moving Read Only Files",IDYES,&l_ComboItem_Input) == IDYES);

		if (confirmReplace)
		{
			// The title 
			char* titleBar = "Confirm Moving Read Only File";

			// We are replacing a directory with a directory
			sprintf(s_messageString, "Are you sure you want to move the read only file \"%s\"?",
				this->GetFileName());

			// What set of buttons will we use?
			UINT buttonSet = multiple ? MB_ALLYESNOCANCEL : MB_YESNOCANCEL;

			Dynamic<int> showAgain;
			int askUser = NewTek_MessageBox(NULL, s_messageString, titleBar, buttonSet, &showAgain);

			// Does the User not want to see this again?
			if (showAgain.Get() && (askUser != IDCANCEL))
				al->SetValue_Int("File Management, Confirm Moving Read Only Files", IDNO);

			if (askUser == IDCANCEL)
			{
				yesAll = true;
				return false;
			}
			else if (askUser == IDALL)
			{
				yesAll = true;
				return true;
			}
			else if (askUser == IDNO)
			{
				yesAll = false;
				return false;
			}
		}
	}
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::RefreshInterestedFileButtonIcons()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	for (unsigned i = 0; i < m_interestedButtons.NoItems(); i++)
		m_interestedButtons[i]->FB_RefreshIcon();
}
/////////////////////////////////////////////////////////////////////////////////////////

unsigned FileProperties::CleanUp_NewTekInfo(const char* path, bool recursive, int cleanUpFlags, bool recycleBin)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Simple param check
	if (!path || !path[0] || !cleanUpFlags)
		return 0;

	// How many files will we delete
	unsigned numDeleted = 0;

	// Make progress visible
	NewTek_UIThreadIsBusy(true);

	// Check for My Computer
	if (!stricmp(path, FILEBIN_MYCOMPUTER))
	{	// Do not allow clean-up from my computer

		/*// Only useful in a recursive check
		if (!recursive) return false;

		// Loop through each of the drives on the system (Not the Floppies)
		char thisDrive[3];
		strcpy(thisDrive, "A:");
		DWORD drives = GetLogicalDrives();
		for (char i = 0; i < (sizeof(DWORD)*8); i++)
		{
			DWORD mask = (DWORD)1 << i;
			if (drives & mask)
			{
				thisDrive[0] = 'A' + i;
				if (GetDriveType(thisDrive) == DRIVE_FIXED)
					numDeleted += CleanUp_NewTekInfo(thisDrive, recursive, cleanUpFlags, recycleBin);
			}
		}*/
	}
	else
	{
		// Check out the NewTek info folder
		char searchPath[MAX_PATH];
		sprintf(searchPath, "%s\\%s", path, ICONINFOFOLDER);
		numDeleted += CleanUp_NewTekInfoFolder(searchPath, cleanUpFlags, recycleBin);

		// Check out the Edit Cache
		if (cleanUpFlags & Delete_EditCache)
			numDeleted += CleanUp_EditorCacheFiles(path);	

		// Loop through all of the folders
		if (recursive)
		{
			// Look for each folder to recursively call this function
			sprintf(searchPath, "%s\\*", path);
			WIN32_FIND_DATA findData;
			HANDLE findHandle = FindFirstFile(searchPath, &findData);
			if(findHandle != INVALID_HANDLE_VALUE)
			{
				bool foundValid = true;
				do
				{
					while( foundValid && findData.cFileName[0] == '.')
					{	if (!FindNextFile(findHandle, &findData ))
							foundValid = false;
					}
					if (foundValid)
					{
						char thisFolder[MAX_PATH*8];
						sprintf(thisFolder, "%s\\%s", path, findData.cFileName);
						if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							numDeleted += CleanUp_NewTekInfo(thisFolder, recursive, cleanUpFlags, recycleBin);
					}

				}while (foundValid && FindNextFile(findHandle, &findData));
			}
			FindClose(findHandle);
		}
	}
	return numDeleted;
}
/////////////////////////////////////////////////////////////////////////////////////////

unsigned FileProperties::CleanUp_EditorCacheFiles(const char* path)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	unsigned numDeleted = 0;

	tList<char*> filesToDelete;

	// Look through each file in this folder
	char searchPath[MAX_PATH];
	sprintf(searchPath, "%s\\*.ToasterEdit Cache File", path);
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		bool foundValid = true;
		do
		{
			while( foundValid && findData.cFileName[0] == '.')
			{
				if (!FindNextFile(findHandle, &findData ))
					foundValid = false;
			}
			if (foundValid)
			{
				char* deleteMe = new char[strlen(path) + strlen(findData.cFileName) + 2];
				sprintf(deleteMe, "%s\\%s", path, findData.cFileName);
				filesToDelete.Add(deleteMe);
			}

		}while (foundValid && FindNextFile(findHandle, &findData));
	}
	FindClose(findHandle);

	// Delete the files and the dynamically allocated arrays
	for (unsigned i = 0; i < filesToDelete.NoItems(); i++)
	{
		if (NewTek_DeleteFile(filesToDelete[i], false))
			numDeleted++;
		delete[] filesToDelete[i];
	}

	return numDeleted;
}
/////////////////////////////////////////////////////////////////////////////////////////

unsigned FileProperties::CleanUp_NewTekInfoFolder(const char* newTekInfoFolder, int cleanUpFlags, bool recycleBin)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	unsigned numDeleted = 0;

	tList<char*> filesToDelete;

	// Look through each file in this folder
	char searchPath[MAX_PATH];
	sprintf(searchPath, "%s\\*", newTekInfoFolder);
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(searchPath, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		bool foundValid = true;
		do
		{	NewTek_UIThreadIsBusy(true);
			while( foundValid && findData.cFileName[0] == '.')
			{
				if (!FindNextFile(findHandle, &findData ))
					foundValid = false;
			}
			if (foundValid)
				if (CleanUp_TemporaryFile(newTekInfoFolder, findData, cleanUpFlags))
				{
					char* deleteMe = new char[strlen(newTekInfoFolder) + strlen(findData.cFileName) + 2];
					sprintf(deleteMe, "%s\\%s", newTekInfoFolder, findData.cFileName);
					filesToDelete.Add(deleteMe);
				}

		}while (foundValid && FindNextFile(findHandle, &findData));
	}
	FindClose(findHandle);

	// Delete the files and the dynamically allocated arrays
	for (unsigned i = 0; i < filesToDelete.NoItems(); i++)
	{	NewTek_UIThreadIsBusy(true);
		if (NewTek_DeleteFile(filesToDelete[i], recycleBin))
			numDeleted++;
		delete[] filesToDelete[i];
	}

	// Remove the directory if it is empty
	RemoveDirectory(newTekInfoFolder);

	return numDeleted;
}
/////////////////////////////////////////////////////////////////////////////////////////

bool FileProperties::CleanUp_TemporaryFile(const char* newTekInfoFolder, WIN32_FIND_DATA & findData, int cleanUpFlags)
{	
	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Get the corresponding file for this info file and see if it is THE info file or some other temp file
	char originalFile[MAX_PATH];
	char tempFile[MAX_PATH];
	sprintf(tempFile, "%s\\%s", newTekInfoFolder, findData.cFileName);
	bool trueInfoFile = GetFileFromInfoName(tempFile, originalFile);

	// Is the file a DVE or something else that will not be regenerated
	bool isNonRegen = IsFileNonRegenerated(findData, originalFile);

	// Is the file locked?
	FileProperties* origFP = GetHandle(originalFile);

	// Does the file exist that is associated with this 
	bool fileExists = origFP && (origFP->GetFileStatus() != FileProperties_NoFile);
	bool locked = origFP && origFP->FP_IsLockedInToasterApp();

	if (!locked && (!fileExists || (cleanUpFlags & CleanUp_AllFiles)))
	{
		if (trueInfoFile)
			return (cleanUpFlags & Delete_AssetManagement);
		else if (isNonRegen)
			return (cleanUpFlags & Delete_NonRegenerated);
		else
			return (cleanUpFlags & Delete_TempFiles);
	}

	// We should keep this file
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////

bool FileProperties::IsFileNonRegenerated(WIN32_FIND_DATA & findData, char* originalFile)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Files marked read only are also marked regenerated
	bool ret = (findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY);
	char* lastDot = NewTek_GetLastDot(originalFile);
	if (lastDot && !ret)
	{
		// If the original file has these extensions, they are not regenerated
		ret = ((!stricmp(lastDot+1, "dve")) ||
				(!stricmp(lastDot+1, "ColorTint")) ||
				(!stricmp(lastDot+1, "filter")) || 
				(!stricmp(lastDot+1, "transition")) ||
				(!stricmp(lastDot+1, "SolidColor")));
	}

	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////

void FileProperties::FP_LockInToasterApp()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// Increase my handle so I am not deleted
	GetHandle();
	bool wasLocked = FP_IsLockedInToasterApp();
	m_toasterAppLocks++;
	if (m_parentDir) m_parentDir->FP_LockInToasterApp();
	if (!wasLocked)
		Changed(FileProperties_Changed_InProject);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool FileProperties::FP_FreeFromToasterApp()
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	// PEDANTIC
	bool stillLocked = false;
	if (m_toasterAppLocks) 
	{
		m_toasterAppLocks--;
		stillLocked = FP_IsLockedInToasterApp();
		if (!stillLocked)
			Changed(FileProperties_Changed_InProject);
		if (m_parentDir)
			m_parentDir->FP_FreeFromToasterApp();
		ReleaseHandle();
	}
	return stillLocked;
}
/////////////////////////////////////////////////////////////////////////////////////////
void FileProperties::LockedInAppMessageBox(char* operationTitle)
{	FunctionBlock ThreadSafe(g_AssetHandleLock);
	char* spacer = " ";
	if (!operationTitle)
	{
		operationTitle = "";
		spacer = "";
	}
	if (GetFileStatus() == FileProperties_Directory)
		sprintf(s_messageString, 
		TL_GetString( "TR::" "The operation %s%scould not be performed because the directory:\n\"%s\"\n or one of its contents is being used by an applicaton.", "The operation %s%scould not be performed because the directory:\n\"%s\"\n or one of its contents is being used by an applicaton." ), 
			operationTitle, spacer, GetFileName());
	else
		sprintf(s_messageString, 
		TL_GetString( "TR::" "The operation %s%scould not be performed because the file:\n\"%s\"\n is being used by an application.", "The operation %s%scould not be performed because the file:\n\"%s\"\n is being used by an application." ), 
			operationTitle, spacer, GetFileName());
	NewTek_MessageBox(NewTek_GetGlobalParentHWND(),s_messageString,TL_GetString( "TR::" "File Locked", "File Locked" ), MB_OK, NULL);
}
/////////////////////////////////////////////////////////////////////////////////////////

char* FileProperties::FP_GetTypeString()
{
	long type = this->GetFileStatus();

	//This has the effect of making volumes and folders visually appears as though they don't have
	//the file type property.
	if (type == FileProperties_File)
	{
		const char* retFileName = GetFileName();
		char* lastDot = NewTek_FindExtension(retFileName);
		if (lastDot)
			return lastDot;
	}

	return "";
}
/////////////////////////////////////////////////////////////////////////////////////////