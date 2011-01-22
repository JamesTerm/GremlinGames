#include "stdafx.h"

/*	DirectoryLayout
A UtilLib_SimpleGrid that is tied to a directory in the file system.  When a directory is chosen,
All of the current children are deleted and new FileButton children are added that represent
each file in the directory.
	*/

DirectoryLayout::DirectoryLayout()
{
	m_currentPosition = 0;
	m_history.Add(FileProperties::GetHandle( FILEBIN_MYCOMPUTER ));
	m_history[m_currentPosition]->AddDependant(this, (long)m_history[m_currentPosition]);
	m_amIUsingHistory = false;
	m_layoutToInform = NULL;
	m_willIBeAdjusting = 0;
	m_destroying = false;
	m_sortNextLayout = false;

	// We start by not showing funky files
	m_showHidden = m_showEncrypted = m_showSystem = false;

	//but we will show everything else
	m_showFiles=m_showFolders=true;

	// I am my sons father!
	m_viewMode = new Details_ViewMode;
	m_viewMode->m_parent = this;
	m_viewMode->SetMatchingParentWindow(this);
	m_viewMode->m_matchWidth = true;
	m_viewMode->AddDependant(this);

	// Listen to myself for DragExiting
	AddDependant(this);

	// Setup spacing
	UtilLib_SimpleGrid_PutSpacingX(DirectoryLayout_SpacingX);
	UtilLib_SimpleGrid_PutSpacingY(DirectoryLayout_SpacingY);
}
FileButton_ViewMode* DirectoryLayout::Get_FileButton_ViewMode(FileButton* fileButton){return m_viewMode;}
////////////////////////////////////////////////////////////////////////////////////////////
DirectoryLayout::~DirectoryLayout()
{
	// No longer listen to myself
	DeleteDependant(this);

	// Remove myself as a dependant from my current history position
	m_history[m_currentPosition]->DeleteDependant(this);

	// Release the Handles for all of the History elements
	for (unsigned i = 0; i < m_history.NoItems(); i++)
	{
		FileProperties::ReleaseHandle(m_history[i]);
		m_history[i] = NULL;
	}

	// Delete the view mode
	m_viewMode->DeleteDependant(this);
	delete m_viewMode;
	m_viewMode = NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////
const char* DirectoryLayout::GetDirectory( char *p_directory )
{
	if (m_history.NoItems() == 0) 
		return NULL;
	
	if (p_directory)
	{
		strcpy(p_directory, m_history[m_currentPosition]->GetFileName());
		return p_directory;
	}
	else 
		return (m_history[m_currentPosition]->GetFileName());
}
////////////////////////////////////////////////////////////////////////////////////////////
unsigned DirectoryLayout::GetCurrentPosition(){return m_currentPosition;}
tList<FileProperties*>* DirectoryLayout::GetHistory(){return (&m_history);}
unsigned DirectoryLayout::SetCurrentPosition(unsigned p_newPosition)
{
	if (p_newPosition < m_history.NoItems())
	{
		m_amIUsingHistory = true;
		m_history[m_currentPosition]->DeleteDependant(this);
		m_currentPosition = p_newPosition;
		m_history[m_currentPosition]->AddDependant(this);
		SetDirectory(GetDirectory(), false);	// Don't defer
	}
	return m_currentPosition;
}
////////////////////////////////////////////////////////////////////////////////////////////
//! This is called when the window is create
void DirectoryLayout::InitialiseWindow(void)
{	
	StartTimer(0, DirectoryLayour_ScanForFolderChanges);
	UtilLib_SimpleGrid::InitialiseWindow();
	//OnTimer(0);
}
////////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::OnTimer(unsigned long TimerID)
{	
	if (!m_DirectoryMonitor.Error())
	{	
		if (m_DirectoryMonitor.HasChangeOccured())
		{	
			// Update the current folder
			SetDirectory(NULL, false);

			// We need to validate all icons, the update could be occuring because
			// a file has been changed in the folder, and so it needs a new icon.
			for( size_t i=0; i<m_activeButtons.NoItems(); i++ )
				m_activeButtons[i]->FB_RefreshIcon( true );
		}
	}

	// Call my predecessor
	UtilLib_SimpleGrid::OnTimer(TimerID);
}
////////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::SetInitialDirectory(const char *p_directory)
{
	// Preconditions
	assert(m_currentPosition < m_history.NoItems());		// Sanity check

	/*
	if (m_willIBeAdjusting)								// Can't be in the middle of a change
		return false;
	*/

	if (DirectoryLayout_InvalidDirectory == this->ValidateNewDirectory(const_cast<char*>(p_directory)))	// Must be a valid dir
		return false;

	m_willIBeAdjusting = 0;	// Blow away all changes coming up!!!

	this->SetDirectory(p_directory, false);	// Do the directory layout (Don't defer)
	
	// Postcondition - The directory that I just set should be at the top of the history stack
	assert(m_currentPosition == m_history.NoItems()-1);
	if (m_currentPosition != (m_history.NoItems()-1))
		return false;

	// Clear everything below the top of the history stack
	if (1 < m_history.NoItems())
	{
		FileProperties *pCurrentDir = m_history[m_history.NoItems()-1];
		
		// Release the Handles for all of the History elements I'm about to blow away
		for (unsigned i = 0; i < m_history.NoItems()-1; i++)
		{
			if (!m_history[i])
				continue;
		
			FileProperties::ReleaseHandle(m_history[i]);
			m_history[i] = NULL;
		}

		m_history.DeleteAll();	// Clear the history list

		// Add my current directory back to the history as the sole element
		m_history.Add(pCurrentDir);

	}
	m_currentPosition = 0;	// Set my current history position to the beginning of the list

	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::SetDirectory(const char *p_directory, bool defer)
{
	// If I am about to place the directory anyway, and this is just a refresh, just ignore it
	if (!p_directory && (0 < m_willIBeAdjusting)/*m_tempDirectory[0]*/)
		return;

	// Get rid of the temp directory if it is already there
	m_tempDirectory[0] = 0;

	char* newPath = NULL;
	if (p_directory)
	{	strcpy(m_tempDirectory, p_directory);
		NewTek_FixFilename( m_tempDirectory, m_tempDirectory );
		newPath = m_tempDirectory;
	}

	// Try to keep from making multiple changes
	m_willIBeAdjusting++;

	if (defer)
	{
		// I want to defer the message
		DeferredMessage((unsigned)newPath, (unsigned)this);
	}
	else
	{
		// I am going to call my deferred Message Handler directly
		ReceiveDeferredMessage((unsigned)newPath, (unsigned)this);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::DisplayThisFile(WIN32_FIND_DATA findData, char* filename)
{
	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && !m_showSystem) 
		return false;

	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !m_showHidden) 
		return false;

	if ((findData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) && !m_showEncrypted) 
		return false;

	if (!strcmp(ICONINFOFOLDER, findData.cFileName)) 
		return false;
	
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////
void	DirectoryLayout::ShowSystem(bool showSystem)
{
	if (showSystem != m_showSystem)
	{
		m_showSystem = showSystem;
		SetDirectory(GetDirectory(), false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
void	DirectoryLayout::ShowHidden(bool showHidden)
{
	if (showHidden != m_showHidden)
	{
		m_showHidden = showHidden;
		SetDirectory(GetDirectory(), false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
void	DirectoryLayout::ShowEncrypted(bool showEncrypted)
{
	if (showEncrypted != m_showEncrypted)
	{
		m_showEncrypted = showEncrypted;
		SetDirectory(GetDirectory(), false);
	}
}

void	DirectoryLayout::ShowFiles(bool showFiles)
{
	if (showFiles != m_showFiles)
	{
		m_showFiles = showFiles;
		SetDirectory(GetDirectory(), false);
	}
}

void	DirectoryLayout::ShowFolders(bool showFolders)
{
	if (showFolders != m_showFolders)
	{
		m_showFolders = showFolders;
		SetDirectory(GetDirectory(), false);
	}
}



////////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::CreateDirectoryListing(	tList<FileProperties*>* fileList, 
												FileProperties* searchProps, 
												const char* filter, 
												bool recursive, 
												bool includeFolders,
												bool includeFiles)
{
	bool trulyRecursive = false;
	char searchPath[MAX_PATH];
	char thisFile[MAX_PATH];
	const char* p_directory = searchProps->GetFileName();
	if (!filter) filter = "*";

	if (strcmp(p_directory, FILEBIN_MYCOMPUTER ) != 0)
	{
		if (includeFolders)
		{
			// Make a pass looking for each folder
			sprintf(searchPath, "%s\\*", p_directory);
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
					sprintf(thisFile, "%s\\%s", p_directory, findData.cFileName);
					if (foundValid && DisplayThisFile(findData,thisFile) && 
						(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						FileProperties* fp = FileProperties::GetHandle(thisFile, searchProps);
						fileList->Add(fp);
						if (recursive)
						{
							bool r = CreateDirectoryListing(fileList, fp, filter, recursive, includeFolders, includeFiles);
							trulyRecursive |= r;
						}
					}
				}while (foundValid && FindNextFile(findHandle, &findData));
			}
			FindClose(findHandle);
		}

		if (includeFiles) 
		{
			WIN32_FIND_DATA findData2;
			//Just to be safe we'll only apply the new change for filters which have a ';' detected in them
			bool HasMultipleMasks=filter?(strrchr(filter,';')!=NULL):false;
			if (!HasMultipleMasks)
				sprintf(searchPath, "%s\\%s", p_directory, filter);
			else //the filter implementation will be handled below since FindFirstFile (or ex)  doesn't support multiple wild card searches -James
				sprintf(searchPath, "%s\\%s", p_directory, "*");

			HANDLE findHandle2 = FindFirstFile(searchPath, &findData2);
			if(findHandle2 != INVALID_HANDLE_VALUE)
			{
				bool foundValid = true;
				const char *ext;//, *filtermatch;
				char *lp_filter = NULL;
				bool filtermatch = false;
				do
				{
					while( foundValid && findData2.cFileName[0] == '.')
					{
						if (!FindNextFile(findHandle2, &findData2 ))
							foundValid = false;
					}

					if (HasMultipleMasks)
					{
						// Now look for only the files that match the filter
						// Nathan - I changed this from a simple string in string test because .dv was popping up for .dve
						ext=NewTek_GetLastDot(findData2.cFileName);
						filtermatch = false;
						char delims[] = "#; *";
						lp_filter=(char *)malloc(strlen(filter)+1);
						char *tokout = strtok(strcpy(lp_filter,filter), delims);
						while(( tokout != NULL ) && (!filtermatch) && (ext))
						{
							if (!stricmp(tokout, ext))
							  filtermatch = true;
							tokout = strtok( NULL, delims );
						}        
						if (lp_filter)
							free(lp_filter);
						if (filtermatch)
						{
							sprintf(thisFile, "%s\\%s", p_directory, findData2.cFileName);
							foundValid=true;
						}
						else
							foundValid=false;
					}
					else
						sprintf(thisFile, "%s\\%s", p_directory, findData2.cFileName);

					if (foundValid && DisplayThisFile(findData2,thisFile))
					{
						if ((findData2.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && includeFolders){}
						else
						{
							// If this is a directory, it was already added with includeFolders
							FileProperties* fp = FileProperties::GetHandle(thisFile, searchProps);
							fileList->Add(fp);
						}
					}
				}while (FindNextFile(findHandle2, &findData2));
			}
			FindClose(findHandle2);
		}
	}
	else
	{	// There is a major performance issue to look at floppy drives, so ont he first pass through
		// we store whether the a: and b: drives exist.
		static bool g_ADriveFound = true;
		static bool g_BDriveFound = true;

		// We are dealing with the each drive on the system
		// Iterate through the different drives
		// WE NEVER want recursive here, searches that want it can look one drive at a time!
		DWORD drives = GetLogicalDrives();
		for (char i = 0; i < (sizeof(DWORD)*8); i++)
		{	// No drives ?
			if ((i==0)&&(!g_ADriveFound)) continue;
			if ((i==1)&&(!g_BDriveFound)) continue;

			DWORD mask = (DWORD)1 << i;
			if (drives & mask)
			{
				strcpy(thisFile, "A:\\");
				thisFile[0] = 'A' + i;

				// Do not provide output warnings
#ifndef VT5_No_Driver_Dependancies
				const bool OldFlag = GetMessageLoop_HeartBeat()->FlagAsExiting( true );
#endif VT5_No_Driver_Dependancies

				// Check for drives that are not mounted, e.g. the a: drive which is always
				// mounted on a system.
				char VolumeNameBuffer[MAX_PATH];
				char FileSystemNameBuffer[MAX_PATH];
				DWORD VolumeSerialNumber , MaximumComponentLength , FileSystemFlags;
				if (!GetVolumeInformation(	thisFile , VolumeNameBuffer , sizeof(VolumeNameBuffer) ,
											&VolumeSerialNumber , &MaximumComponentLength ,
											&FileSystemFlags , FileSystemNameBuffer , sizeof(FileSystemNameBuffer) ))
				{	// Udpate the drive settings
					if (i==0) g_ADriveFound=false;
					if (i==1) g_BDriveFound=false;
					continue;
				} 

				thisFile[2]=0;

				FileProperties* fp = FileProperties::GetHandle(thisFile, searchProps);
				fileList->Add(fp);

				// Restore the output warning state
#ifndef VT5_No_Driver_Dependancies
				GetMessageLoop_HeartBeat()->FlagAsExiting( OldFlag );
#endif VT5_No_Driver_Dependancies
			}
		}
	}
	return trulyRecursive;
}
////////////////////////////////////////////////////////////////////////////////////////////
e_NewDirectoryType DirectoryLayout::ValidateNewDirectory(const char* newDirectory)
{
	// Precondition
	if (!newDirectory) 
		return DirectoryLayout_InvalidDirectory;

	// Init retVal
	e_NewDirectoryType ret = DirectoryLayout_InvalidDirectory;

	// Using History?
	if (m_amIUsingHistory)
		return DirectoryLayout_NewDirectory;

	// Same as my current directory?
	if (!strcmp(newDirectory, this->GetDirectory()))
		return DirectoryLayout_RefreshCurrent;

	// FILEBIN_MYCOMPUTER ?
	if (!strcmp(newDirectory, FILEBIN_MYCOMPUTER))
		return DirectoryLayout_NewDirectory;

	// A Drive?
	UINT type = GetDriveType(newDirectory);
	if (type != DRIVE_NO_ROOT_DIR)
		return DirectoryLayout_NewDirectory;

	// A folder???
	WIN32_FIND_DATA findData;
	HANDLE findHandle = FindFirstFile(newDirectory, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ret = DirectoryLayout_NewDirectory;
	}
	FindClose(findHandle);

	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::RecycleAllChildren()
{
	// Do not do any layouts until we are done!
	MultipleLayouts_Start();

	tList<FileButton*> childList;
	childList.CopyFrom(&m_activeButtons);
	m_activeButtons.DeleteAll();

	for (unsigned long index = 0; index < childList.NoItems(); index++)
		RecycleChild(childList[index]);

	// We are done moving things out of the way, show an empty screen
	MultipleLayouts_Finished(true, false);
}
///////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::RecycleChild(FileButton* fb)
{
	if (!fb) 
		return;
	
	m_activeButtons.Delete(fb);	// Remove it from my active (shown) lilst
	
	if (m_recycledButtons.Exists(fb))	// No dups in recycled list
		return;
		
	m_recycledButtons.Add(fb);						// Add it to my recycled lists

	// Stop waiting for the preview
	fb->FB_StopThreadReading();

	// Clear the selection state of this file
	fb->DragAndDrop_SelectMe(false);

	// Hide it
	fb->ShowWindow(SW_HIDE);
}
///////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::IsActiveButton(HWND fb)
{
	// It's recycled if it's not in my active list
	FileButton *pFB = ::GetWindowInterface<FileButton>(fb);
	if (!pFB)
		return false;
	else
		return m_activeButtons.Exists(pFB);
}
///////////////////////////////////////////////////////////////////////////////////////////
FileButton* DirectoryLayout::GetRecycledFileButton(FileProperties* pf)
{
	FileButton* fb = NULL;

	long width = this->m_viewMode->FileButton_ViewMode_GetPreferedXSize();
	long height = this->m_viewMode->FileButton_ViewMode_GetPreferedYSize();

	if (m_recycledButtons.NoItems())
	{
		// Here is my new button
		fb = m_recycledButtons[0];

		// Delete it from both lists
		m_recycledButtons.DeleteEntryInOrder(0);

		fb->ShowWindow(SW_RESTORE);
	}
	else
	{	
		// If I can't find a button in my recycled list, create a new one
		fb = GetInterface<FileButton>
			(NewTek_New("FileButton",GetWindowHandle(),-width,-height,width,height,pf->GetFileName()));
	}

	// Set the properties and the proper size
	if (fb)
	{
		fb->SetFileButtonProperties(pf, false, false);

		m_activeButtons.Add(fb);	// Add it to my active shown buttons
	}
	
	return fb;
}
///////////////////////////////////////////////////////////////////////////////////////////
DirectoryLayout* DirectoryLayout::s_sortingDetailsLayout = NULL;
int DirectoryLayout::SortProperties(const void *arg1, const void *arg2)
{
	// Watch for sort order
	Details_ViewMode* viewMode = s_sortingDetailsLayout ? s_sortingDetailsLayout->m_viewMode : NULL;
	DetailsView_Info* dvInfo = viewMode ? viewMode->DVM_GetDetailsInfo() : NULL;
	int sortOrder = dvInfo ? dvInfo->DVI_GetSortIndex() : 1;

	// The int we are returning
	int ret = 0;

	DirectoryLayout_WindowLayout_Item* Item1=(DirectoryLayout_WindowLayout_Item*)arg1;
	DirectoryLayout_WindowLayout_Item* Item2=(DirectoryLayout_WindowLayout_Item*)arg2;
	FileButton* fb1 = Item1 ? Item1->m_fileButton : NULL;
	FileButton* fb2 = Item2 ? Item2->m_fileButton : NULL;
	FileProperties* fp1 = fb1 ? fb1->GetProperties() : NULL;
	FileProperties* fp2 = fb1 ? fb2->GetProperties() : NULL;
	
	// Watch for missing files
	if (!fp1 && !fp2) 
		return 0;
	else if (!fp1) 
		ret = -1;
	else if (!fp2) 
		ret = 1;

	if (!ret)
	{
		int status1 = fp1->GetFileStatus();
		int status2 = fp2->GetFileStatus();

		if (status1 != status2)
			ret = (status2 - status1);
		else if (viewMode)
		{
			// Use a dynamic string because GetTextFromFilename() uses a static string
			bool missing1, missing2;
			double sv1, sv2;
			DynamicString str1;
			char TempBuffer[MAX_PATH];
			char TempBuffer2[MAX_PATH];
			viewMode->GetSortTextFromFilename(TempBuffer,MAX_PATH,fb1, missing1, sv1);
			str1.Set(TempBuffer);
			viewMode->GetSortTextFromFilename(TempBuffer2,MAX_PATH,fb2, missing2, sv2);
			char* str2 = TempBuffer2;
			if (missing1 == missing2)
			{
				// If sorting by value AND both exist, compare the two values
				DetailsView_ColumnInfo* dvci = dvInfo ? dvInfo->DVI_GetSortColumn() : NULL;
				if (!missing2 && !missing1 && dvci && 
					(dvci->DVCI_GetSortType() == DVCI_SortType_Numeric))
				{
					if (sv1 != sv2)
						ret = (sv1 < sv2) ? -1 : 1;
				}
				else
					ret = stricmp(str1.Get(), str2);
			}
			else 
				ret = missing1 ? 1 : -1;
		}
	}

	// If nothing else worked, sort by the full file name
	if (!ret)
		ret = stricmp(fp1->GetFileName(), fp2->GetFileName());
	
	return (ret * sortOrder);
}
///////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::DeferredSetDirectory(const char *p_directory)
{	// ADJC :	If there is no change in directory, there is no need to proceed
	//			I wonder if this is a safe thing to do.
	// James:   Unfortunately not it caused case 4050-
	//			Where when switching to the media tab the file bin did not get populated
	//if ( ( p_directory ) && (!stricmp( p_directory, GetDirectory() ) ) )
	//	return true;

	e_NewDirectoryType newDir = ValidateNewDirectory(p_directory);
	if (DirectoryLayout_InvalidDirectory == newDir) // If it's an invalid directory
		return false;	

	// Do not do any layouts until we are done!
	MultipleLayouts_Start();

	// We are listening to the new folder
	bool bMonitorSuccess = m_DirectoryMonitor.ChangeFolder(p_directory);	// mwatkins - I think this might fail sometimes

	// I don't want any messages while I am doing this!
	ReceiveMessagesOff();

	// DEBUGGING TIMER
	//long currentTickCount = GetTickCount();

	m_sortNextLayout = false;

	// Get the proper size of the view mode
	long vmXsize = m_viewMode->FileButton_ViewMode_GetPreferedXSize();
	long vmYsize = m_viewMode->FileButton_ViewMode_GetPreferedYSize();

	// Get the list of files
	tList<FileProperties*> fileList;
	bool recursive = false;
	FileProperties* fp = FileProperties::GetHandle(p_directory);
	recursive = CreateDirectoryListing(&fileList, fp, GetFileFilter(), GetRecursiveSearch(), m_showFolders, m_showFiles);
	fp->ReleaseHandle();

	if (DirectoryLayout_NewDirectory == newDir)
	{
		// clear all of the existing Files
		RecycleAllChildren();

		if (!m_amIUsingHistory)	// If I'm not using the history buttons
		{
			// Delete all the items after m_currentPosition
			for (size_t i = m_history.NoItems()-1; i > m_currentPosition; i--)
			{
				FileProperties::ReleaseHandle(m_history[i]);
				m_history.DeleteEntry(i);
			}

			// Add the new directory to the end of the history
			m_history.Add(FileProperties::GetHandle(p_directory));
			m_history[m_currentPosition]->DeleteDependant(this);
			m_currentPosition = m_history.NoItems()-1;
			m_history[m_currentPosition]->AddDependant(this, (long)m_history[m_currentPosition]);
		}
		else 
			m_amIUsingHistory = false;	// Reset the using history buttons flag

		// Call changed and notify the path layout if it exists
		Changed(DirectoryLayout_NewFolder, p_directory);
		if (m_layoutToInform)
			m_layoutToInform->SetNewPath(p_directory, this);

		// Set the Scroll bar to the top
		BaseWindowLayoutManager_Y_Slider_GetVariable()->Set(0.0);
	}
	else
	{
		// ValidateChildren recycles buttons w/ files not in the fileList.
		// Upon return, the fileList ONLY contais files that require new fileButtons to be created.
		m_sortNextLayout = ValidateChildren(&fileList);	
	}

	if (recursive)
	{
		// TODO: Make a way to easily set mutliple attributes in View Mode
		// Set the View Mode to SmallIcon if it is in LargeIcon OR IconOnly
		// Set the name Mode to full path relative to p_directory
		// Set recompute layout to true if there was a change
	}

	// Create new fileButtons for remaining files in the fileList
	for (size_t i = 0; i < fileList.NoItems(); i++)
	{	
		FileButton* thisButton = GetRecycledFileButton(fileList[i]);
		if (thisButton)
		{
			fileList[i] = NULL;
			thisButton->SetWindowSize(vmXsize, vmYsize);
		}
	}
	m_sortNextLayout = m_sortNextLayout || (fileList.NoItems() > 0);

	MultipleLayouts_Finished(m_sortNextLayout, false);

	// Debugging:
	//float time = (GetTickCount() - currentTickCount) / 1000.0f;
	//static char outputString[MAX_PATH];
	//sprintf(outputString, "DirectoryTime: %s (%f s.)\n",p_directory, time);
	//OutputDebugString(outputString);

	// OK, I will take messages now
	ReceiveMessagesOn();

	// Notify everyone that the directory has changed
	Changed(DirectoryLayout_DirectoryChanged);

	// Finished

	return (DirectoryLayout_NewDirectory == newDir);
}
//////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::DirLayout_CreateNewFolder(char* p_folderName, bool p_moveSelectedInto)
{
	if (strcmp(GetDirectory(), FILEBIN_MYCOMPUTER) == 0) return false;
	char buffer[MAX_PATH];
	char* fullName = buffer;

	if (!p_folderName)
	{
		// Get a text string from the user
		long reasonForExit;
		p_folderName = NewTek_GetTextFromPopupWindow("New Folder Name", "New Folder", *this, reasonForExit);
		if (p_folderName)
		{
			sprintf(fullName, "%s\\%s", GetDirectory(), p_folderName);
			NewTek_free(p_folderName);
			p_folderName = NULL;
			if (reasonForExit == Popup_Escape_EscapeKey)	// They really didn't want it!
				return false;
		}
	}
	else
	{
		// Use just what was passed in
		sprintf(fullName, "%s\\%s", GetDirectory(), p_folderName);
	}

	// Watch for copies that are already there
	fullName = (char*)FileProperties::GetNextAvailableFileName(fullName);
	
	m_willIBeAdjusting++;
	if (CreateDirectory(fullName, NULL))
	{
		if (p_moveSelectedInto)
		{
			// Iterate through all of the children that are FileButtons
			for (unsigned i = 0; i < m_activeButtons.NoItems(); i++)
			{
				FileButton* thisButton = m_activeButtons[i];
				if (thisButton)
				{
					if (thisButton->DragAndDrop_AmISelected())
						thisButton->MoveFileTo(fullName);
				}
			}
		}

		m_willIBeAdjusting--;
		SetDirectory(NULL, false);

		// Lets select the new FileButton and De-Select everything else
		FileButton* newFolderButton = GetFileButton(fullName);
		if (newFolderButton)
		{
			if (!p_moveSelectedInto)
				this->DragAndDrop_SelectAllChildren(false);
			newFolderButton->DragAndDrop_SelectMe(true);

			// Be sure we can see the new window
			tList<HWND> windowList;
			windowList.Add(newFolderButton->GetWindowHandle());
			this->CenterOnWindows(windowList);
		}
		return true;
	}
	else
	{
		m_willIBeAdjusting--;
		SetDirectory(NULL, false);	
		return false;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
FileButton* DirectoryLayout::GetFileButton(const char* p_filename)
{
	// Iterate through each child
	for (unsigned long index = 0; index < m_activeButtons.NoItems(); index++)
	{
		FileButton* thisButton = m_activeButtons[index];
		if (!strcmp(thisButton->GetFileName(), p_filename))
			return thisButton;
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::ValidateFileButton(FileButton* p_fileButton, tList<FileProperties*>* fileList)
{
	if (!p_fileButton) return false;
	for (unsigned i = 0; i < fileList->NoItems(); i++)
	{
		FileProperties* listFile = (*fileList)[i];
		if (listFile == p_fileButton->GetProperties())
		{
			// We no longer need this entry
			if (listFile) listFile->ReleaseHandle();
			fileList->DeleteEntryInOrder(i);
			return true;
		}
	}

	// ASSERT: This is NOT A GOOD BUTTON, so RECYCLE IT
	RecycleChild(p_fileButton);
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::ValidateChildren(tList<FileProperties*>* fileList)
{
	// Do not do any layouts until we are done!
	MultipleLayouts_Start();
	bool redoLayout = false;

	HWND lastHWND = NULL;
	for (unsigned long index = 0; index < m_activeButtons.NoItems();)
	{
		FileButton* thisButton = m_activeButtons[index];

		// Recycled buttons whose file isn't in the fileList
		if (ValidateFileButton(thisButton, fileList))		// Returns true if thisButton's file is in the fileList
		{
			lastHWND = thisButton->GetWindowHandle();
			index++;	// Increment the index here to be sure we have covered everything
		}
		else 
			redoLayout = true;	// Do not increment, One was deleted and this index contains a new item!
	}

	// We are done, perhaps we should layout now.
	MultipleLayouts_Finished(redoLayout, false);

	return redoLayout;
}
//////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::MouseRButtonRelease(long Flags,long x,long y)
{
	// Create a Context Menu
	tList<BaseWindowClass*>	Windows;
	Windows.Add(this);
	unsigned OldNoPopups=BWC_GetNoPopupWindows();
	NewTek_ExecuteContextMenu(&Windows);
	if (OldNoPopups==BWC_GetNoPopupWindows())
		SetKBFocus();
}
//////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::FileButton_DeletedProperties(FileButton* fb)
{
	RecycleChild(fb);
	this->Layout_PerformLayout(this->GetWindowHandle());
}
//////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (ItemChanging == this)
	{
		if ((!strcmp(String, DragNDropContext_DragFinishedExited)) ||
			(!strcmp(String, DragNDropContext_DragReturnToOriginalFinished)))
		{
			SetDirectory(NULL, true);
		}
	}
	else if (ItemChanging == m_history[m_currentPosition])
	{
		// Something has happened in my own directory
		if (!strcmp(String, FileProperties_Changed_MovedFile))
		{
			// Simply tell the PathLayout that we moved
			if (m_layoutToInform)
				m_layoutToInform->SetNewPath(GetDirectory(), this);
		}
		else if ((!strcmp(String, FileProperties_Changed_FileEnteredDir)) || 
			(!strcmp(String, FileProperties_Changed_FileLeftDir)))
		{
			// We might want to re-load if we are not about to do it anyway
			SetDirectory(NULL, false);
		}
		else if ((!strcmp(String, FileProperties_Changed_FileDeletedInfo)) ||
				(!strcmp(String, FileProperties_Changed_FileDeletedNotInfo))) 
		{
			// Recurse up my parent chain until we find a dir that works
			FileProperties* parentDir = m_history[m_currentPosition]->GetParentHandle();
			// mwatkins - Add NULL check
			if (parentDir)
			{
				while (!parentDir->DoesFileExist())
				{
					FileProperties* grandParentDir = parentDir->GetParentHandle();
					FileProperties::ReleaseHandle(parentDir);
					parentDir = grandParentDir;
				}
				SetDirectory(parentDir->GetFileName(), true);
				FileProperties::ReleaseHandle(parentDir);
			}
		}
		return;
	}
	else if (ItemChanging == m_viewMode)
	{
		// Refresh the layout to show the change
		if (!stricmp(String, DVI_Changed_Sort))
		{
			m_sortNextLayout = true;
			this->Layout_PerformLayout(this->GetWindowHandle());
		}
		return;
	}
	UtilLib_SimpleGrid::DynamicCallback(ID, String, args,ItemChanging);
}
///////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::DestroyWindow()
{	RemoveAllDependantsToThis();
	m_destroying = true;
	UtilLib_SimpleGrid::DestroyWindow();
}
///////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{
	if (ID2 == (unsigned)this)
	{
		if (m_destroying) 
			return;

		if (m_willIBeAdjusting > 0)
		{
			m_willIBeAdjusting--;

			if (0 == m_willIBeAdjusting)
			{
				const char *NewPath=(char*)ID1;
				if (!NewPath)
					NewPath = GetDirectory();
				if (NewPath[0])
					DeferredSetDirectory(NewPath);
				if (NewPath == m_tempDirectory)
					m_tempDirectory[0] = 0;
			}
		}

		return;
	}
	UtilLib_SimpleGrid::ReceiveDeferredMessage(ID1,ID2);
}
///////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{
	// If the item being dropped is already one of my children, let me drop it
	if (this->GetWindowHandle() == ::GetParent(Dropped->hWnd))
		return true;

	return CanItemBeDroppedOnProperties(GetDirectoryProps(), Dropped, this);
}
///////////////////////////////////////////////////////////////////////////////////////

bool CanItemBeDroppedOnProperties(FileProperties* fp, Control_DragNDrop_DropInfo *Dropped, BaseWindowClass* DroppedOn)
{
	// Be sure the thing I am droppin on is agreeable
	long status = fp ? fp->GetFileStatus() : FileProperties_NoFile;
	if ((status != FileProperties_Drive) && (status != FileProperties_Directory))
		return false;

	// Do not allow if not a FileButton OR BaseLoad Save
	FileButton* thisButton = GetWindowInterface<FileButton>(Dropped->hWnd);
	if (!thisButton || strcmp("FileButton", GetTypeName(thisButton)))	// TODO: ???
	{
		// Try for a BaseLoadSave Item
		BaseLoadSave* bls = GetWindowInterface<BaseLoadSave>(Dropped->hWnd);
		if (bls) 
		{	
			DirectoryLayoutSaveDisable *BLSD=GetInterface<DirectoryLayoutSaveDisable>(bls);
			if (BLSD) 
				return BLSD->DirectoryLayoutSaveDisable_ShouldISave();

			return true;
		}
		else 
			return false;
	}

	// Be sure the file this Button represents can be dropped here as well
	status = thisButton->GetStatus();
	return ((status == FileProperties_File) || (status == FileProperties_Directory));
}
///////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::EditCut()
{
	// If I am in FILEBIN_MYCOMPUTER, Send a Message that this won't work
	if (!stricmp(this->GetDirectory(), FILEBIN_MYCOMPUTER))
	{
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "The Cut operation is not available to drives.", "The Cut operation is not available to drives." ), 
			TL_GetString( "TR::" "Cannot Cut a Drive", "Cannot Cut a Drive" ), MB_OK, NULL);
		return false;
	}
	tList<FileProperties*> fileNames;
	GetSelectedFileNames(&fileNames);
	return FileProperties::EditCut(&fileNames);
}
/////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::GetSelectedFileNames(tList<FileProperties*>* fileList)
{
	if (!fileList) 
		return;
	
	tList<FileButton*> Windows;

	for(unsigned i=0;i<m_activeButtons.NoItems();i++)
	{	
		FileButton *FB=m_activeButtons[i];
		if ((FB)&&(FB->DragAndDrop_AmISelected()))
			Windows.Add(FB);
	}

	if (Windows.NoItems())
		qsort(&Windows[0],Windows.NoItems(),sizeof(FileButton*),FileButton::FileButton_MouseLButtonDblClick_compare);
	
	for (unsigned i = 0; i < Windows.NoItems(); i++)
		fileList->Add(Windows[i]->GetProperties());
}
/////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout::EditCopy()
{
	if (!stricmp(this->GetDirectory(), FILEBIN_MYCOMPUTER))
	{
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "The Copy operation is not available to drives.", "The Copy operation is not available to drives." ), 
			TL_GetString( "TR::" "Cannot Copy a Drive", "Cannot Copy a Drive" ), MB_OK, NULL);
		return false;
	}
	tList<FileProperties*> fileNames;
	GetSelectedFileNames(&fileNames);
	return FileProperties::EditCopy(&fileNames);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool	DirectoryLayout::EditPaste()
{	if (!stricmp(this->GetDirectory(), FILEBIN_MYCOMPUTER))
	{
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "The Paste operation is not available to this location.", "The Paste operation is not available to this location." ), 
			TL_GetString( "TR::" "Cannot Paste", "Cannot Paste" ), MB_OK, NULL);
		return false;
	}

	this->ReceiveMessagesOff();
	bool ret = FileProperties::EditPaste( GetDirectory() );
	this->ReceiveMessagesOn();
	SetDirectory(NULL, false);
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
bool	DirectoryLayout::EditDelete()
{	if (!stricmp(this->GetDirectory(), FILEBIN_MYCOMPUTER))
	{
		NewTek_MessageBox(NULL,
			TL_GetString( "TR::" "The Delete operation is not available to drives.", "The Delete operation is not available to drives." ), 
			TL_GetString( "TR::" "Cannot Delete a Drive", "Cannot Delete a Drive" ), MB_OK, NULL);
		return false;
	}

	this->ReceiveMessagesOff();
	tList<FileProperties*> fileNames;
	GetSelectedFileNames(&fileNames);
	bool ret = FileProperties::EditDelete(&fileNames);
	this->ReceiveMessagesOn();
	SetDirectory(NULL, false);
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
void DropOntoProperties(FileProperties* fp, WindowLayout_Item *ObjectsDropped, BaseWindowClass* DroppedOn)
{
	const char* folderName = fp ? fp->GetFileName() : NULL;
	long status = fp ? fp->GetFileStatus() : FileProperties_NoFile;
	if ((status != FileProperties_Drive) && (status != FileProperties_Directory))
		return;

	// We keep a list of all of the file buttons that are being dropped here
	tList<FileProperties*> fileNames;

	// We scan through all the items
	WindowLayout_Item *Scan=ObjectsDropped;
	while(Scan)
	{	
		if (Scan->hWnd)
		{				
			// See if this is really a FileButton
			FileButton			*FB=GetWindowInterface<FileButton>(Scan->hWnd);
			if (FB && (strcmp(GetTypeName(FB), "FileButton")))
				FB = NULL;

			if (FB)
			{
				// Be sure I can move this button
				long status = FB->GetStatus();
				if ((status == FileProperties_NoFile) ||
					(status == FileProperties_Drive) ||
					(status == FileProperties_MyComputer)) { }
				else
				{
					FileProperties* fp = FB->GetProperties();
					fp->GetHandle();
					fileNames.Add(fp);
				}
			}
			else
			{
				// Write to a dummy file to start with
				char tempFile[MAX_PATH];
				sprintf(tempFile, "%s\\%s.%s", folderName, TEMPBLSFILE, BLS_FileExtension);
				
				// Create a SaveData to write to
				SaveData* sd = new SaveData(tempFile, SaveData_Flag_DragInto);

				// Check to see if this item inherits from BaseLoadSave
				BaseLoadSave* bls = GetWindowInterface<BaseLoadSave>(Scan->hWnd);
				BaseLoadSave_Proxy* blsProxy = GetWindowInterface<BaseLoadSave_Proxy>(Scan->hWnd);
				BaseLoadSave* proxy = NULL;

				// Recursively check for proxy
				while (blsProxy)
				{
					proxy = blsProxy->GetProxyItem(sd);
					BaseLoadSave_Proxy* newProxy = GetInterface<BaseLoadSave_Proxy>(proxy);
					if (newProxy != blsProxy)
						blsProxy = newProxy;
					else newProxy = NULL;
				}
				if (proxy) 
					bls = proxy;

				if (bls)
				{
					// Use this buffer to create a good filename to move the temp file to
					char buffer[MAX_PATH];

					// Figure out what the name of our file should be
					BaseLoadSave_FileNaming* blsn = GetInterface<BaseLoadSave_FileNaming>(bls);
					char* type = blsn ? blsn->BLS_GetDroppedFileNameBase() : GetTypeName(bls);
					sprintf(buffer, "%s\\%s.%s", folderName, type, BLS_FileExtension);
					const char* filename = FileProperties::GetNextAvailableFileName(buffer);
					
					// ASSERT: We have a good file to write to.
					if (!SavePlugin(bls, sd))
					{
						char error[MAX_PATH + 40];
						sprintf(error, TL_GetString( "TR::" "There was an error writing to the file %s.", "There was an error writing to the file %s." ), filename);
						NewTek_MessageBox(DroppedOn ? DroppedOn->GetWindowHandle() : NULL, error, TL_GetString( "TR::" "Error Writing File", "Error Writing File" ), MB_OK, NULL);
					}
					else
					{
						// We need to release the file handle to do the move I think
						delete sd; sd = NULL;
						// Move the temp file to the good file
						if (!MoveFile(tempFile, filename))
							DWORD error = GetLastError();
					}
				}
				if (sd) 
				{
					delete sd; sd = NULL;
				}
			}
		}
		
		// Look at the next item
		Scan=Scan->Next;
	}

	// Are we copying or moving anything
	if (fileNames.NoItems())
	{
		if (GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)))	// Ctrl is pressed, do a copy
		{
			// mwatkins - do the async boogie
			FileProperties::CopyToDirectory(&fileNames, folderName, true/*async it!!!*/);
		}
		else
		{
			// mwatkins - Mikey likes to move asyncronously
			FileProperties::MoveToDirectory(&fileNames, folderName, true/*async it!!!*/);
		}

		// Release the reference counts
		for (unsigned i = 0; i < fileNames.NoItems(); i++)
			fileNames[i]->ReleaseHandle();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////

WindowLayout_Item* DirectoryLayout::DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									)
{
	if (DroppedHere)
	{
		// Check to be sure the windows (Just the first is needed) is already in my list
		FileButton* fb = GetWindowInterface<FileButton>(ObjectsDropped->hWnd);
		if (fb && m_activeButtons.Exists(fb));
		else
		{
			// Hold off on all changes to the directory updating this layout
			m_willIBeAdjusting++;

			// Do the work of dropping it in to my directory
			DropOntoProperties(GetDirectoryProps(), ObjectsDropped, this);


			// Delete ALL Windows dropped in to me, they will be re-created
			WindowLayout_Item* scan = ObjectsDropped;
			while (scan)
			{
				BaseWindowClass* bwc = GetWindowInterface<BaseWindowClass>(scan->hWnd);
				NewTek_Delete(bwc);
				scan->hWnd = NULL;
				scan = scan->Next;
			}

			// Re-layout the window
			m_willIBeAdjusting--;
			SetDirectory(NULL, true);
		}
	}
	return UtilLib_SimpleGrid::DragNDrop_DropItemsHere
		(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);
}
/////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::EditProperties(long XPos, long YPos)
{
	tList<FileProperties*> fileNames;
	GetSelectedFileNames(&fileNames);

	if (XPos < 0) 
		XPos = this->GetWindowPosX() + 10;
	if (YPos < 0) 
		YPos = this->GetWindowPosY() + 10;
	
	BaseWindowClass* popupParent = GetWindowInterface<BaseWindowClass>(NewTek_GetPopupParent(GetWindowHandle()));
	if (!popupParent) 
		popupParent = this;
	
	FileProperties::EditProperties(&fileNames, popupParent, XPos, YPos);
}
/////////////////////////////////////////////////////////////////////////////////////////
void DirectoryLayout::ViewModeSizeChanged(FileButton_ViewMode* viewMode)
{
	MultipleLayouts_Start();
	long newX = viewMode->FileButton_ViewMode_GetPreferedXSize();
	long newY = viewMode->FileButton_ViewMode_GetPreferedYSize();
	for (unsigned i = 0; i < m_activeButtons.NoItems(); i++)
	{
		// We are assuming ALL children are File Buttons here
		FileButton* bwc = m_activeButtons[i];
		bwc->SetWindowSize(newX, newY);
	}
	MultipleLayouts_Finished(true, false);
}
//////////////////////////////////////////////////////////////////////////////////////////
WindowLayout_Item *DirectoryLayout::Layout_ComputeLayout(WindowLayout_Item *Children,long Width,long Height)
{
	// What will be the size of the good ones?
	long vmXsize = m_viewMode->FileButton_ViewMode_GetPreferedXSize();
	long vmYsize = m_viewMode->FileButton_ViewMode_GetPreferedYSize();
	
	// Break the list up in to items that are recycled and those that are not
	WindowLayout_Item *recycledHead = NULL, *recycledTail = NULL;
	WindowLayout_Item *goodHead = NULL, *goodTail = NULL;
	unsigned numGoodChildren = 0;

	WindowLayout_Item* scan = Children;
	while (scan)
	{
		// Grab The Next before we mess up the pointers
		WindowLayout_Item* thisChild = scan;
		scan = scan->Next;
		
		// Set up some sizes
		thisChild->XSize = vmXsize;
		thisChild->YSize = vmYsize;
		thisChild->Visible = true;
		
		// Which list are we adding to?
		WindowLayout_Item** listHead = &goodHead;
		WindowLayout_Item** listTail = &goodTail;
		
		if (!IsActiveButton(thisChild->hWnd))
		{
			// Add to the recycled list, make it non-visible and put away
			listHead = &recycledHead;
			listTail = &recycledTail;
			thisChild->Visible = false;
			thisChild->XPosn = -thisChild->XSize;
			thisChild->YPosn = -thisChild->YSize;
		}
		else 
			numGoodChildren++;
		
		// Do the list management
		if (*listTail)
			(*listTail)->Next = thisChild;
		else
			(*listHead) = thisChild;
		
		(*listTail) = thisChild;
		thisChild->Next = NULL;
	}
	
	// Sort the Good Ones if we need to
	if (m_sortNextLayout)
	{
		if (numGoodChildren > 1)
		{
			// Make a list to sort properly
			tList<DirectoryLayout_WindowLayout_Item> windowList;
			windowList.SetSize(numGoodChildren);
			unsigned index = 0;
			
			// Loop through the good list
			WindowLayout_Item* scan = goodHead;
			while (scan)
			{
				// Grab the File Properties for this window and put it in the list to sort
				FileButton* fb = GetWindowInterface<FileButton>(scan->hWnd);
				FileProperties* fp = fb ? fb->GetProperties() : NULL;
				windowList[index].m_file = fp;
				windowList[index].window = scan;
				windowList[index].m_fileButton = fb;
				
				// Next Set
				scan = scan->Next;
				index++;
			}
			
			// Sort the list
			s_sortingDetailsLayout = this;
			qsort(&windowList[0],windowList.NoItems(),sizeof(DirectoryLayout_WindowLayout_Item),SortProperties);
			s_sortingDetailsLayout = NULL;
			
			// Recreate the sorted list
			goodHead = scan = windowList[0].window;
			m_activeButtons[0] = windowList[0].m_fileButton;
			for (unsigned i = 1; i < numGoodChildren; i++)
			{
				m_activeButtons[i] = windowList[i].m_fileButton;
				scan->Next = windowList[i].window;
				scan = scan->Next;
			}
			scan->Next = NULL;
			goodTail = scan;
		}

		// Do not do it again
		m_sortNextLayout = false;
	}
	
	
	// Have My Base Class sort the positions of the good list
	goodHead = UtilLib_SimpleGrid::Layout_ComputeLayout(goodHead, Width, Height);
	
	// Put the lists back together
	if (recycledHead)
	{
		recycledTail->Next = goodHead;
		Children = recycledHead;
	}
	else 
		Children = goodHead;
	
	// All done
	return Children;
}

void DirectoryLayout::GetChildrenInSelectionOrder(tList<HWND> &ChildrenList)
{
	// PEDANTIC
	if (!GetWindowHandle()) 
		return;

	HWND CurrentWindow=GetWindow(GetWindowHandle(),GW_CHILD);
	while (CurrentWindow)
	{	
		// We have found one more child, only add it if it is not 
		if (::IsWindowVisible(CurrentWindow))
			ChildrenList.Add(CurrentWindow);

		// Get the next child
		CurrentWindow=GetWindow(CurrentWindow,GW_HWNDNEXT);
		
		// No idea why this happens !
		if (CurrentWindow==m_hWnd) 
			break;
	}
}

void DirectoryLayout::LayoutManager_GetChildExtents(WindowLayout_Item *Children, long windowWidth, long windowHeight,
																long & MinX, long & MaxX, long & MinY, long & MaxY)
{
	UtilLib_SimpleGrid::LayoutManager_GetChildExtents(Children, windowWidth, windowHeight, MinX, MaxX, MinY, MaxY);
	
	// The mins will always be 0, keep the hidden stuff hidden
	if (MinX < 0) 
		MinX = 0;
}

void DirectoryLayout::Child_AboutToLeave(HWND hWnd, HWND NewParent)
{
	FileButton* FB = GetWindowInterface<FileButton>(hWnd);
	m_recycledButtons.DeleteInOrder(FB);
	m_activeButtons.DeleteInOrder(FB);
}

void DirectoryLayout::DVRC_SetColumnHeader(DetailsView_ColumnLayout* detailsViewHeader)
{
	if (m_detailsViewHeader != detailsViewHeader)
	{
		DetailsView_RowCollection::DVRC_SetColumnHeader(detailsViewHeader);
		m_viewMode->DVM_SetDetailsInfo(detailsViewHeader ? detailsViewHeader->DVCL_GetDetailsViewInfo() : NULL);
	}
}

void DirectoryLayout::DVRC_GetDetailsViewRowList(tList<DetailsView_Row*>& rowList)
{
	// Just copy over my list of active buttons
	unsigned numItems = m_activeButtons.NoItems();
	rowList.SetSize(numItems);
	
	for (unsigned i = 0; i < numItems; i++)
		rowList[i] = m_activeButtons[i];
}
