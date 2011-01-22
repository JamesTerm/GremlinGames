/*!	FileCopyMove.cpp
	Handles copying/moving files on another thread.  Also displays a dialog box w/ a progress bar.
	Mike Watkins
	NewTek
	10/30/2002
*/

#include "stdafx.h"
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////////////////////

using namespace FileCopyMove_Globals;

////////////////////////////////////////////////////////////////////////////////////////////////

#define FILECOPYMOVEHANDLER_SCRIPTROOT "FileCopyMove/Copy"
#define FILEMOVEHANDLER_SCRIPTROOT "FileCopyMove/Move"

#define COPY_FILE_DONE_ID 723

#define PROGRESS_TIMERID 47

////////////////////////////////////////////////////////////////////////////////////////////////
// Class that differentiates a move dialog UI from a copy dialog

class FileMoveHandler : public FileCopyMoveHandler
{
public:
	FileMoveHandler() 
	{
		// This is the skin root
		this->SetSkinRoot(::FindFiles_FindFile(FINDFILE_SKINS, FILEMOVEHANDLER_SCRIPTROOT));
	}
	virtual ~FileMoveHandler() { }
};

////////////////////////////////////////////////////////////////////////////////////////////////

// Register the plugin controls
NewTek_RegisterPluginControl(FileCopyMoveHandler);
NewTek_RegisterPluginControl(FileMoveHandler);

////////////////////////////////////////////////////////////////////////////////////////////////

bool FileCopyMove_Globals::IsInFolder(const char* szFolderPath, const char* szTestPath, char* szRelativePath)
{
	bool bRetVal = false;
	if (szFolderPath && szTestPath)
	{
		size_t cbFolderPath = ::strlen(szFolderPath);
		size_t cbTestPath = ::strlen(szTestPath);
		if ((0<cbFolderPath) && (cbFolderPath<cbTestPath))
		{
			bRetVal = (0 == ::strnicmp(szFolderPath, szTestPath, cbFolderPath));
			if (bRetVal && szRelativePath)
				::strcpy(szRelativePath, &(szTestPath[cbFolderPath]));
		}

	}

	return bRetVal;
}

//here b/c it handles multibyte paths correctly
void FileCopyMove_Globals::GetParentDirectoryName(const char* szPath, char szDirName[])
{
	char path_buffer[MAX_PATH] = {0};
	char drive[_MAX_DRIVE] = {0};
	char dir[_MAX_DIR] = {0};
	
   ::_splitpath(szPath, drive, dir, NULL, NULL);

   size_t cbDir = ::strlen(dir);
   if (1 >= cbDir)
	   ::strcpy(szDirName, drive);
   else
   {
	   if (('\\' == dir[cbDir-1]))
		   dir[cbDir-1] = '\0';

	   ::sprintf(path_buffer, "%s%s", drive, dir);
	   ::_splitpath(path_buffer, NULL, NULL, szDirName, NULL);
   }
}

bool FileCopyMove_Globals::IsFolderEmpty(const char* szPath, bool bIgnoreEmptyNewTekInfoFolder)
{
	bool bFoundFile = false;

	char szSearchPath[MAX_PATH] = { 0 };
	::sprintf(szSearchPath, "%s\\%s", szPath, "*.*");

	WIN32_FIND_DATA findData;
	HANDLE findHandle = ::FindFirstFile(szSearchPath, &findData);
	if (INVALID_HANDLE_VALUE != findHandle)
	{		
		do 
		{
			// If it's not a control path AND
				// I'm not ignoring the NewTek Info folder OR
				// it is the NewTek Info folder
			if ('.' != findData.cFileName[0])
			{
				if (!bIgnoreEmptyNewTekInfoFolder)	// I'm not ignoring anything, so I'm not empty
				{
					bFoundFile = true;
					break;
				}
				else if (FILE_ATTRIBUTE_DIRECTORY&findData.dwFileAttributes)	// If it's a directory
				{
					char szSubFolder[MAX_PATH] = { 0 };
					::sprintf(szSubFolder, "%s\\%s", szPath, findData.cFileName);
					if (!FileProperties::IsNewTekInfoFolder(szSubFolder))		// If it's not the NewTek Info folder, I'm not empty
					{
						bFoundFile = true;
						break;
					}
					else if (!IsFolderEmpty(szSubFolder, true))	// If the NewTek Info folder isn't empty, I'm not empty
					{
						bFoundFile = true;
						break;
					}
				}
				else	// It's not a directory, so I'm not empty
				{
					bFoundFile = true;
					break;
				}
			}
		}
		while (::FindNextFile(findHandle, &findData));
	}

	::FindClose(findHandle);

	return !bFoundFile;
}

////////////////////////////////////////////////////////////////////////////////////////////////

void FileCopyMoveHandler::StartAsyncCopy(const tList<FileProperties*> &sourceFilesList, const char* szDestPath, bool bMove)
{
	//BaseWindowClass *pDesktop = ::BaseWindowClass_GetNextWindow("Desktop2");
	BaseWindowClass *pDesktop = ::GetWindowInterface<BaseWindowClass>(::NewTek_GetGlobalParentHWND());
	
	//don't want it to be visible initially
	HWND myHwnd = NULL;
	if (pDesktop)
	{
        if (bMove)
			myHwnd = pDesktop->OpenChild("FileMoveHandler", 0, 0, 0, 0, WS_CHILD | WS_POPUP);
		else
			myHwnd = pDesktop->OpenChild("FileCopyMoveHandler", 0, 0, 0, 0, WS_CHILD | WS_POPUP);
	}
	else
	{
		if (bMove)
			myHwnd = ::Desktop_LaunchPlugin("FileMoveHandler", "Moving file...", 0, 0, 0, 0, true);
		else
			myHwnd = ::Desktop_LaunchPlugin("FileCopyMoveHandler", "Copying file...", 0, 0, 0, 0, true);
	}

	BaseWindowClass *pBWC = ::GetWindowInterface<BaseWindowClass>(myHwnd);
	FileCopyMoveHandler *pFCM = ::GetInterface<FileCopyMoveHandler>(pBWC);
	if (pFCM)
		pFCM->InitCopy(sourceFilesList, szDestPath, bMove);
}

DWORD CALLBACK FileCopyMoveHandler::CopyProgressRoutine(
														LARGE_INTEGER TotalFileSize,          // file size
														LARGE_INTEGER TotalBytesTransferred,  // bytes transferred
														LARGE_INTEGER StreamSize,             // bytes in stream
														LARGE_INTEGER StreamBytesTransferred, // bytes transferred for stream
														DWORD dwStreamNumber,                 // current stream
														DWORD dwCallbackReason,               // callback reason
														HANDLE hSourceFile,                   // handle to source file
														HANDLE hDestinationFile,              // handle to destination file
														LPVOID lpData                         // from CopyFileEx
														)
{
	// Get the content handle
	FileCopyMoveHandler	*me = reinterpret_cast<FileCopyMoveHandler*>(lpData);
	if (!me) 
		return PROGRESS_CONTINUE;

	me->m_PercentageProcessed = 
		double(TotalBytesTransferred.QuadPart)/double(TotalFileSize.QuadPart);

	if (me->GetCancelCopyFlagVal())
		return PROGRESS_CANCEL;
	else
		return PROGRESS_CONTINUE;
}

FileCopyMoveHandler::FileCopyMoveHandler() : 
	m_ScreenObject_StretchySkin(NULL), m_bMove(false), m_bReplaceAll(false), m_bReadOnlyMoveAll(false),
	m_bCancelCopyFlag(FALSE), m_NextCopyReadyLock(1), m_cAtomicOps(0), m_UtilLib_GasGauge(NULL), 
	m_UtilLib_FileNameTextItem(NULL), m_UtilLib_SourceDirTextItem(NULL),
	m_UtilLib_DestDirTextItem(NULL), m_currAtomicOpNum(0), m_CopyErrorCode(0)
{
	m_CurrFoldersAndFilesListIdx = 0;
	m_PercentageProcessed = 0.0;
	
	// This is the skin root
	this->SetSkinRoot(::FindFiles_FindFile(FINDFILE_SKINS, FILECOPYMOVEHANDLER_SCRIPTROOT));
}

FileCopyMoveHandler::~FileCopyMoveHandler()
{
	for (int i=0; i<m_SourceFoldersAndFilesList.NoItems(); i++)
	{
		delete [] (m_SourceFoldersAndFilesList[i]);
		m_SourceFoldersAndFilesList[i] = NULL;
	}

	for (int j=0; j<m_DestFoldersAndFilesList.NoItems(); j++)
	{
		delete [] (m_DestFoldersAndFilesList[j]);
		m_DestFoldersAndFilesList[j] = NULL;
	}

	while (m_SourceFoldersBeingCopied.size())
	{
		delete [] (m_SourceFoldersBeingCopied.top());
		m_SourceFoldersBeingCopied.pop();
	}
	
	while (m_DestFoldersBeingCopied.size())
	{
		delete [] (m_DestFoldersBeingCopied.top());
		m_DestFoldersBeingCopied.pop();
	}
}

void FileCopyMoveHandler::InitCopy(const tList<FileProperties*> &sourceFilesList,
									const char* szDestPath, bool bMove)
{
	if (!sourceFilesList.NoItems() || !szDestPath)
		return;

	m_bMove = bMove;

	for (int i=0; i<sourceFilesList.NoItems(); i++)
		this->EnumerateFoldersAndFiles(sourceFilesList[i], szDestPath);

	bool bCloseWindowNow = true;

	if (m_SourceFoldersAndFilesList.NoItems() &&
		(m_SourceFoldersAndFilesList.NoItems() == m_DestFoldersAndFilesList.NoItems()))
	{
		if (this->PrepareNextCopy())
		{
			this->SignalNextCopyReady();
			this->StartThread();
			this->StartTimer(PROGRESS_TIMERID, 1000/*1 second*/);
			bCloseWindowNow = false;
		}
	}

	if (bCloseWindowNow)
		this->FinishUp();
}

void FileCopyMoveHandler::EnumerateFoldersAndFiles(FileProperties* sourceFileProp, const char* szDestPath)
{
	if (!sourceFileProp || !szDestPath)
		return;

	char szSourcePath[MAX_PATH] = {0};
	::strcpy(szSourcePath, sourceFileProp->GetFileName());

	char szDestPathToUse[MAX_PATH] = {0};
	::strcpy(szDestPathToUse, szDestPath);
	::NewTek_StrCatSlashIfNeeded(szDestPathToUse);
	::strcat(szDestPathToUse, ::NewTek_GetFileNameFromPath(szSourcePath));

	if (m_bMove)
	{
		//don't need to f*?k around w/ this sh$t!!!
		if (!::stricmp(szSourcePath, szDestPathToUse))
		{
			return;
		}
	}
	else
	{
		if (!::stricmp(szSourcePath, szDestPathToUse))
			::strcpy(szDestPathToUse, sourceFileProp->GetNextAvailableFileName(szDestPathToUse));
	}

	//update the newtek info stuff
	sourceFileProp->WriteInfoFile();

	long status = sourceFileProp->GetFileStatus();
	if ((FileProperties_Directory==status) || (FileProperties_File==status))
	{
		// it's a file or folder, so add it to our lists
		char *pSourceFileBuf = new char[MAX_PATH];
		char *pDestFileBuf = new char[MAX_PATH];
		::strcpy(pSourceFileBuf, szSourcePath);
		::strcpy(pDestFileBuf, szDestPathToUse);
		
		m_SourceFoldersAndFilesList.Add(pSourceFileBuf);
		m_DestFoldersAndFilesList.Add(pDestFileBuf);
	}

	if (FileProperties_Directory == status)
	{
		if (!m_bMove || !::ArePathsOnSameVolume(szSourcePath, szDestPathToUse))	//! copying or moving between volumes
		{
			char szSearchPath[MAX_PATH] = {0};
			::sprintf(szSearchPath, "%s\\*", szSourcePath);
		
			WIN32_FIND_DATA findData;
			HANDLE findHandle2 = ::FindFirstFile(szSearchPath, &findData);
			if (INVALID_HANDLE_VALUE != findHandle2)
			{		
				bool bFoundValid = true;
				do
				{
					//skip the "down dir" and "up dir"
					while (bFoundValid && ('.' == findData.cFileName[0]))
					{
						if (!::FindNextFile(findHandle2, &findData))
							bFoundValid = false;
					}
				
					if (bFoundValid)
					{
						char szNextSourcePath[MAX_PATH] = {0};
						::sprintf(szNextSourcePath, "%s\\%s", szSourcePath, findData.cFileName);

						//don't bother w/ the newtek info shiat here
						if (!FileProperties::IsNewTekInfoFolder(szNextSourcePath))
						{
							FileProperties* nextFileProp = 
								FileProperties::GetHandle(szNextSourcePath, sourceFileProp);
					
							this->EnumerateFoldersAndFiles(nextFileProp, szDestPathToUse);

							FileProperties::ReleaseHandle(nextFileProp);
						}
					}
				
				} while (bFoundValid && ::FindNextFile(findHandle2, &findData));
			}
		
			::FindClose(findHandle2);
		}
		else	//! we're moving a directory on the same volume
		{
			m_cAtomicOps++;	//! treated like an atomic progress operation
		}
	}
	else if (FileProperties_File == status)
		m_cAtomicOps++;

}

//Create all parent directories for the next file to be copied
//Prompt user to overwrite files etc.
bool FileCopyMoveHandler::PrepareNextCopy()
{
	bool bRetVal = false;

	FileProperties_Status eFileType = FileProperties_NoFile;
	bool bAtomicOp = false;

	unsigned curFileIdx = m_CurrFoldersAndFilesListIdx;
		
	while (curFileIdx < m_SourceFoldersAndFilesList.NoItems())
	{
		//the source and destination path cursors
		char* szSourceCur = m_SourceFoldersAndFilesList[curFileIdx];
		char* szDestCur = m_DestFoldersAndFilesList[curFileIdx];

		//get the file type
		eFileType = (FileProperties_Status)	FileProperties::FindFileStatus(szSourceCur);

		bAtomicOp = (FileProperties_File==eFileType) || 
						((FileProperties_Directory==eFileType) &&
						::ArePathsOnSameVolume(szSourceCur, szDestCur));

		//if the current source path is a file, we don't need to be in this loop
		if (bAtomicOp)
			break;
		else if (FileProperties_Directory == eFileType)
		{
			//if the current source path is a directory, then it's a parent directory of the 
			//next file to be copied.  therefore, try and create it

			//ensure the copy (e.g. overwrite?)
			if (this->VerifyCopyFile(szSourceCur, szDestCur, eFileType))
			{
				//the directory might already exist, so this call might fail!
				::CreateDirectoryEx(szSourceCur, szDestCur, NULL);

				//cache the directory that we just created
				char *szSourceFolderPath = new char[MAX_PATH];
				::strcpy(szSourceFolderPath, szSourceCur);
				m_SourceFoldersBeingCopied.push(szSourceFolderPath);

				//cache it's corresponding destination
				char *szDestFolderPath = new char[MAX_PATH];
				::strcpy(szDestFolderPath, szDestCur);
				m_DestFoldersBeingCopied.push(szDestFolderPath);

				this->CheckFinishedFolderCopy(curFileIdx);
			}
			else
			{
				//if the copy wasn't ensured, it's not an error.  It just means the user chose 
				//not to overwrite a directory w/ the same name for example.  Therefore,
				//we must skip copying all paths in this folder
				unsigned cSkippedFiles = 0;
				curFileIdx = this->SkipAllSourceSubPaths(szSourceCur, curFileIdx+1, cSkippedFiles);
				m_currAtomicOpNum += cSkippedFiles;

				//curFileIdx is the index of the first path not in the folder.  We will decrement
				//it to point to last path in the folder, b/c the index always gets incremented @
				//the end of the while loop
				curFileIdx--;
			}
		}	
		
		curFileIdx++;

	}	//	while (curFileIdx < m_SourceFoldersAndFilesList.NoItems())

	m_CurrFoldersAndFilesListIdx = curFileIdx;

	if (bAtomicOp)
	{
		m_currAtomicOpNum += 1;
		m_PercentageProcessed = 0.0;

		if (this->VerifyCopyFile(m_SourceFoldersAndFilesList[curFileIdx],
								m_DestFoldersAndFilesList[curFileIdx], eFileType))
		{
			if (m_UtilLib_FileNameTextItem)
			{
				char fname[MAX_PATH] = {0};
				char ext[_MAX_EXT] = {0};
				::_splitpath(m_SourceFoldersAndFilesList[curFileIdx], NULL, NULL, fname, ext);
				::strcat(fname, ext);

				m_UtilLib_FileNameTextItem->SetText(fname);

				if (m_UtilLib_SourceDirTextItem && m_UtilLib_DestDirTextItem)
				{
					char dirname[_MAX_FNAME] = {0};
					
					GetParentDirectoryName(m_SourceFoldersAndFilesList[curFileIdx], dirname);
					m_UtilLib_SourceDirTextItem->SetText(dirname);

					dirname[0] = '\0';
					GetParentDirectoryName(m_DestFoldersAndFilesList[curFileIdx], dirname);
					m_UtilLib_DestDirTextItem->SetText(dirname);
				}
			}
			bRetVal = true;
		}
		else
		{
			//recursive call
			m_CurrFoldersAndFilesListIdx += 1;
			bRetVal = this->PrepareNextCopy();
		}
	}

	return bRetVal;
	
}

unsigned FileCopyMoveHandler::SkipAllSourceSubPaths(const char* szFolderPath, unsigned listIdx,
													unsigned &numFilesSkipped)
{
	numFilesSkipped = 0;

	if (szFolderPath)
	{
		while (listIdx < m_SourceFoldersAndFilesList.NoItems())
		{
			char* szSourceCur = m_SourceFoldersAndFilesList[listIdx];

			bool bInFolder = IsInFolder(szFolderPath, szSourceCur);
			if (!bInFolder)
				break;

			if (FileProperties_File ==
				(FileProperties_Status) FileProperties::FindFileStatus(szSourceCur))
			{
				numFilesSkipped++;
			}

			listIdx++;
		}
	}

	return listIdx;
}

void FileCopyMoveHandler::ReceiveDeferredMessage(unsigned ID1, unsigned ID2)
{
	if (ID1 == COPY_FILE_DONE_ID)
	{
		//the worker thread has finished a copy so...

		bool bDoneCopying = false;

		if (ID2)
		{
			//post process last copy
			this->FinishedCopyFile(m_CurrFoldersAndFilesListIdx);

			//preprocces next file to be copied
			m_CurrFoldersAndFilesListIdx += 1;
			if (this->PrepareNextCopy())
				this->SignalNextCopyReady();
			else if (this->GetRunning())
				bDoneCopying = true;
		}
		else
		{
			bDoneCopying  = true;
		}

		if (bDoneCopying)
		{
			this->FinishUp();

			if (!ID2)	//there was an error copying
			{
				DWORD lCopyErrorCode = m_CopyErrorCode.Get();
				
				LPVOID lpMsgBuf = NULL;
				::FormatMessage( 
					FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM | 
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					lCopyErrorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL 
					);
				
				// Process any inserts in lpMsgBuf.
				// ...
				
				// Display the string.
				::NewTek_MessageBox(NULL, (LPCTSTR)lpMsgBuf, TL_GetString( "TR::" "Error", "Error" ), MB_OK | MB_ICONINFORMATION, NULL);
				
				// Free the buffer.
				::LocalFree(lpMsgBuf);
			}
		}
	}

	//call superclass method
	AutoSkinControl::ReceiveDeferredMessage(ID1, ID2);
}

void FileCopyMoveHandler::ThreadProcessor()
{
	//while waitfornextcopy
	char szSourcePath[MAX_PATH] = {0}, szDestPath[MAX_PATH] = {0};
	if (this->WaitNextCopyReady(szSourcePath, szDestPath))
	{
		//perform copy
		BOOL bCopySuccess = FALSE;
		m_CopyErrorCode = 0;

		if (m_bMove)
		{
			bCopySuccess = ::MoveFileWithProgress(szSourcePath, szDestPath, (LPPROGRESS_ROUTINE)FileCopyMoveHandler::CopyProgressRoutine,
													(LPVOID)this,
													MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		}
		else
		{
			bCopySuccess = ::CopyFileEx(szSourcePath, szDestPath, (LPPROGRESS_ROUTINE)FileCopyMoveHandler::CopyProgressRoutine,
										(LPVOID)this, &m_bCancelCopyFlag, 0);
		}
		
		//signal copy finished
		if (!bCopySuccess)
			m_CopyErrorCode = ::GetLastError();

		this->SignalCopyFinished(bCopySuccess);
	}
	else
		this->SetThreadMustExit(true);	//so we don't get called anymore
}

void FileCopyMoveHandler::InitialiseWindow()
{
	// Setup the skin
	m_ScreenObject_StretchySkin = ::GetInterface<Auto_StretchySkin>(::NewTek_New("Auto_StretchySkin"));
	if (!m_ScreenObject_StretchySkin) 
		_throw("Cannot create the stretchy skin for the file copy move dialog!");

	this->Canvas_SetResource(m_ScreenObject_StretchySkin);
	
	// Call the superclass method
	AutoSkinControl::InitialiseWindow();

	// Setup the gass gauge
	HWND gasHwnd = this->OpenChild(RGBA(7,235,40),"UtilLib_GasGauge");
	m_UtilLib_GasGauge = ::GetWindowInterface<UtilLib_GasGauge>(gasHwnd);
	if (!m_UtilLib_GasGauge) 
		_throw("Could not create UtilLib_GasGauge for the file copy move dialog!");

	HWND staticTextHwnd = this->OpenChild(RGBA(10,240,226), "UtilLib_TextItem");
	m_UtilLib_FileNameTextItem = ::GetWindowInterface<UtilLib_TextItem>(staticTextHwnd);
	if (!m_UtilLib_FileNameTextItem)
		_throw("Could not create text item for filename in the file copy move dialog!");
	
	m_UtilLib_FileNameTextItem->SetAlignment(TextItem_LeftAlign | TextItem_CenterVAlign);
	m_UtilLib_FileNameTextItem->SetFont(TextItem_DefaultFont);
	m_UtilLib_FileNameTextItem->SetFontSize(TextItem_DefaultFontSize);
	m_UtilLib_FileNameTextItem->SetBold(TextItem_DefaultBoldState);
	m_UtilLib_FileNameTextItem->SetColor(0,0,0);
	m_UtilLib_FileNameTextItem->SetText("");

	staticTextHwnd = this->OpenChild(RGBA(221,240,8), "UtilLib_TextItem");
	m_UtilLib_SourceDirTextItem = ::GetWindowInterface<UtilLib_TextItem>(staticTextHwnd);
	if (!m_UtilLib_SourceDirTextItem)
		_throw("Could not create text item for source dir in file copy move dialog!");
	
	m_UtilLib_SourceDirTextItem->SetAlignment(TextItem_LeftAlign | TextItem_CenterVAlign);
	m_UtilLib_SourceDirTextItem->SetFont(TextItem_DefaultFont);
	m_UtilLib_SourceDirTextItem->SetFontSize(TextItem_DefaultFontSize);
	m_UtilLib_SourceDirTextItem->SetBold(TextItem_DefaultBoldState);
	m_UtilLib_SourceDirTextItem->SetColor(0,0,0);
	m_UtilLib_SourceDirTextItem->SetText("");

	staticTextHwnd = this->OpenChild(RGBA(157,68,154), "UtilLib_TextItem");
	m_UtilLib_DestDirTextItem = ::GetWindowInterface<UtilLib_TextItem>(staticTextHwnd);
	if (!m_UtilLib_DestDirTextItem)
		_throw("Could not create text item for dest dir in file copy move dialog!");
	
	m_UtilLib_DestDirTextItem->SetAlignment(TextItem_LeftAlign | TextItem_CenterVAlign);
	m_UtilLib_DestDirTextItem->SetFont(TextItem_DefaultFont);
	m_UtilLib_DestDirTextItem->SetFontSize(TextItem_DefaultFontSize);
	m_UtilLib_DestDirTextItem->SetBold(TextItem_DefaultBoldState);
	m_UtilLib_DestDirTextItem->SetColor(0,0,0);
	m_UtilLib_DestDirTextItem->SetText("");
}

void FileCopyMoveHandler::DestroyWindow()
{
	// Stop the copy thread
	this->StopThread();

	// Stop our timer
	this->StopTimer(PROGRESS_TIMERID);
	
	// Call the superclass method
	AutoSkinControl::DestroyWindow();
	
	// Delete the skin
	::NewTek_Delete(m_ScreenObject_StretchySkin);
	m_ScreenObject_StretchySkin = NULL;
}

void FileCopyMoveHandler::OnTimer(unsigned long TimerID)
{
	//if we're not displayed and enough time has passed, display dialog
	//update the progressbar

	// Set the %'age
	if (0 >= m_currAtomicOpNum)
		return;

	if (!this->IsWindowVisible())
		this->ShowWindow(SW_SHOW);

	m_UtilLib_GasGauge->Slider_GetMinVariable()->Set(0.0);
	m_UtilLib_GasGauge->Slider_GetMaxVariable()->Set((double)m_cAtomicOps);
	m_UtilLib_GasGauge->Slider_GetVariable()->Set(((double)(m_currAtomicOpNum-1)) + m_PercentageProcessed);
}

bool FileCopyMoveHandler::VerifyCopyFile(char* szSourcePath, char* szDestPath, FileProperties_Status fileType)
{
	return ( m_bMove ? 
			FileProperties::_AsyncEnsureMoveFile(szSourcePath, szDestPath, m_cAtomicOps > 1, fileType, m_bReadOnlyMoveAll, m_bReplaceAll) : 
			FileProperties::_AsyncEnsureCopyFile(szSourcePath, szDestPath, m_cAtomicOps > 1, fileType, m_bReplaceAll) );
}

void FileCopyMoveHandler::FinishedCopyFile(unsigned listIdx)
{
	m_bMove ? 
		FileProperties::_AsyncMovedFile(m_SourceFoldersAndFilesList[listIdx],
										m_DestFoldersAndFilesList[listIdx]) : 
		FileProperties::_AsyncCopiedFile(m_SourceFoldersAndFilesList[listIdx],
										m_DestFoldersAndFilesList[listIdx]);

	this->CheckFinishedFolderCopy(listIdx);
}

void FileCopyMoveHandler::CheckFinishedFolderCopy(unsigned listIdx)
{
	assert(m_SourceFoldersBeingCopied.size() == m_DestFoldersBeingCopied.size());
	
	if (!m_SourceFoldersBeingCopied.size())
		return;

	char *szCurrentSourceFolder = m_SourceFoldersBeingCopied.top();
	char *szCurrentDestFolder = m_DestFoldersBeingCopied.top();

	//! done if this is the last item being copied or the next file isn't in the current folder
	bool bDoneCopyingCurrentFolder = 
			(listIdx==m_SourceFoldersAndFilesList.NoItems()-1) ||
			!IsInFolder(szCurrentSourceFolder, m_SourceFoldersAndFilesList[listIdx+1]);

	if (bDoneCopyingCurrentFolder)
	{
		//! copy the newtek file info for the directory

		if (m_bMove && IsFolderEmpty(szCurrentSourceFolder, true/*Ignore NewTek Info folder*/))
		{
			// This moves the NewTek Info folder
			FileProperties::_AsyncMovedFile(szCurrentSourceFolder, szCurrentDestFolder);
			
			::NewTek_DeleteFile(szCurrentSourceFolder, false/*don't recycle*/);
		}
		else
		{
			FileProperties::_AsyncCopiedFile(szCurrentSourceFolder, szCurrentDestFolder);
		}
		
		m_SourceFoldersBeingCopied.pop();
		delete [] szCurrentSourceFolder;
		
		m_DestFoldersBeingCopied.pop();
		delete [] szCurrentDestFolder;
		
	}
}

void FileCopyMoveHandler::ClosingWindow()
{
	//set stop thread flag
	this->SetThreadMustExit(true);

	//make sure to stop the current operation in progress
	this->SetCancelCopyFlagVal(TRUE);

	//finish copying folders (an error might have occurred)
	assert(m_SourceFoldersBeingCopied.size() == m_DestFoldersBeingCopied.size());
	while (m_SourceFoldersBeingCopied.size())
	{
		char *szSourceFolder = m_SourceFoldersBeingCopied.top();
		char *szDestFolder = m_DestFoldersBeingCopied.top();

		//if the source folder is empty, then emulate move
		if (m_bMove && IsFolderEmpty(szSourceFolder, true))
		{
			FileProperties::_AsyncMovedFile(szSourceFolder, szDestFolder);

			::NewTek_DeleteFile(szSourceFolder, false);
		}
		else
		{
			FileProperties::_AsyncCopiedFile(szSourceFolder, szDestFolder);
		}

		m_SourceFoldersBeingCopied.pop();
		delete [] szSourceFolder;

		m_DestFoldersBeingCopied.pop();
		delete [] szDestFolder;
	}

	//call superclass method
	AutoSkinControl::ClosingWindow();
}

void FileCopyMoveHandler::FinishUp()
{
	//destroy progress bar dialog
	this->DeferredDelete();
}

BOOL FileCopyMoveHandler::GetCancelCopyFlagVal() const
{
	FunctionBlock autoLock(&m_CancelCopyFlagLock);
	return m_bCancelCopyFlag;
}

void FileCopyMoveHandler::SetCancelCopyFlagVal(BOOL bVal)
{
	FunctionBlock autoLock(&m_CancelCopyFlagLock);
	m_bCancelCopyFlag = bVal;
}

void FileCopyMoveHandler::SignalNextCopyReady()
{
	::InterlockedExchange(&m_NextCopyReadyLock, 0);
}

bool FileCopyMoveHandler::WaitNextCopyReady(char szSourcePath[], char szDestPath[])
{
	//don't wait if we're done
	if (m_CurrFoldersAndFilesListIdx >= m_SourceFoldersAndFilesList.NoItems())
		return false;

	while (::InterlockedExchange(&m_NextCopyReadyLock, 1))
	{
		//we might finish while waiting
		if (m_CurrFoldersAndFilesListIdx >= m_SourceFoldersAndFilesList.NoItems())
			return false;

		if (this->GetThreadMustExit())
			return false;

		::Sleep(1);
	}

	::strcpy(szSourcePath, m_SourceFoldersAndFilesList[m_CurrFoldersAndFilesListIdx]);
	::strcpy(szDestPath, m_DestFoldersAndFilesList[m_CurrFoldersAndFilesListIdx]);

	return true;
}

void FileCopyMoveHandler::SignalCopyFinished(BOOL bSuccess)
{
	this->DeferredMessage(COPY_FILE_DONE_ID, bSuccess);
}
