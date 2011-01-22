#include "stdafx.h"

tList<AnimatedIcon_Interface *> *FileIcon::s_AnimatedIconInterface=NULL;
tList<CustomIcon_Interface *> *FileIcon::s_CustomIconInterface=NULL;
	
FileIcon::FileIcon()
{
	m_directoryLayout = NULL;
	m_useBitmap = NULL;
	m_doIOwn = false;
	m_amIAnimatingNow = false;
	m_showDefAnimFrame = false;
	m_animationInterface = NULL;
	m_fileName = NULL;
	m_icon = NULL;
	m_properties = NULL;
	m_loopStart = 0.0;
	m_loopEnd = -1.0;
	m_defaultTime = 0.0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
FileIcon::~FileIcon()
{
	if (m_properties)
		m_properties->ReleaseHandle(m_properties);
	if (m_useBitmap && m_doIOwn)
		delete m_useBitmap;
	if (m_animationInterface)
	{
		m_animationInterface->DeleteDependant(this);
		m_animationInterface = NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileIcon::FileIcon_StartAnimation()
{
	if (FileIcon_AmIAnimated()) 
		return m_amIAnimatingNow = m_animationInterface->AnimatedIcon_StartAnimation(m_loopStart, m_loopEnd);
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileIcon::FileIcon_StopAnimation()
{
	if (m_amIAnimatingNow)
	{
		m_amIAnimatingNow = false;
		if (FileIcon_AmIAnimated())
		{
			bool ret = m_animationInterface->AnimatedIcon_StopAnimation();
			if (ret)
				Changed();
			return ret;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileIcon::FileIcon_OnTimer()
{
	if (m_amIAnimatingNow && FileIcon_AmIAnimated()) 
		return m_animationInterface->AnimatedIcon_OnTimer();
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::FileIcon_SetIconFile(char* imageFilename)
{
	if (!imageFilename) return NULL;
	if (m_icon->ReadBitmapFile(imageFilename))
	{
		FileIcon_StopAnimation();
		if (m_fileName && !m_properties)
			_throw("FileIcon::FileIcon_SetIconFile there should not be a filename here");
		if (m_properties)
		{
			if (m_fileName) delete[] m_fileName;
			m_fileName = new char[strlen(imageFilename) + 1];
			strcpy(m_fileName, imageFilename);
			Changed(FileIconChanged_NewImageSet);
			m_properties->m_iconRead = m_fileName;
		}		
		return m_fileName;
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::FileIcon_SetFilename(char* filename)
{
	if (filename)
	{
		FileProperties* thisProp = FileProperties::GetHandle(filename);
		char* ret = FileIcon_SetFileProperties(thisProp);
		FileProperties::ReleaseHandle(thisProp);
		return ret;
	}
	else return FileIcon_SetFileProperties(NULL);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::FileIcon_SetFileProperties(FileProperties* fileProps)
{
	// This is our new properties
	if (m_properties)
		m_properties->ReleaseHandle(m_properties);
	m_properties = fileProps;

	if (m_properties)
		m_properties->GetHandle();
	
	// Delete our old animation if there was one
	FileIcon_StopAnimation();
	if (m_animationInterface)
	{
		m_animationInterface->DeleteDependant(this);
		m_animationInterface = NULL;
	}

	// Try the normal method of loading if there is a properties to use
	if (m_properties)
	{
		// We want to look at the image of our new props
		m_fileName = m_properties->m_iconRead;
		m_icon = &m_properties->m_icon;
		m_animationInterface = m_properties->m_animationInterface;
		if (m_animationInterface)
			m_animationInterface->AddDependant(this, (long)m_animationInterface);

		// Perhaps an icon already exists with this FileProperties object
		if (m_fileName) return m_fileName;

		// Is this a drive?
		if (this->ReadDriveIconFile(m_properties))
			return m_fileName;

		// Is this a non file, or MyComputer
		if (this->ReadInvalidIconFile(m_properties))
			return m_fileName;

		// Try loading as an animation (if my fileProps didn't already have one!)
		if (!m_animationInterface)
		{
			m_animationInterface = GetAnimatedIconReader(m_properties->GetFileName());
			if (m_animationInterface)
			{
				m_animationInterface->AddDependant(this, (long)m_animationInterface);
				m_properties->m_animationInterface = m_animationInterface;
			}
		}

		// Try the special icon in the info folder
		if (this->ReadSpecialIconFile(m_properties))
			return m_fileName;

		// Try the auto in the info folder if newer than the file itself (Or first Child);
		if (this->ReadAutoIconFile(m_properties))
			return m_fileName;

		// Is This a folder
		if (this->ReadFolderIconFile(m_properties))
			return m_fileName;

		// Try loading myself
		WindowBitmap imageBMP;
		if (imageBMP.ReadBitmapFile(m_properties->GetFileName()))
			return SaveAutoImage(m_properties, &imageBMP);

		// Allow custom bitmap override
		m_customInterface = GetCustomIconReader(m_properties,&imageBMP);
		if (m_customInterface) return SaveAutoImage(m_properties, &imageBMP);
	}
	else
	{
		m_icon = &m_defaultIcon;
		m_fileName = NULL;
		m_animationInterface = NULL;
	}
	
	return ReadDefaultIconFile(m_properties);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadDriveIconFile(FileProperties* fileProps)
{
	if (fileProps->GetFileStatus() == FileProperties_Drive)
	{
		char* fullFileName = fileProps->GetFileName();

		// Look for a special instance of this Drive in my Icons folder
		char filePath[MAX_PATH];
		sprintf(filePath, "%s%c.%s", DRIVENAMEHEADER, toupper(fullFileName[0]), ICONEXTENSION);

		if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,filePath)))
			return m_fileName;

		UINT driveInfo = GetDriveType(fullFileName);
		switch (driveInfo)
		{
		case DRIVE_REMOVABLE:
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,REMOVABLEDRIVEICON)))
				return m_fileName;
			break;
		case DRIVE_FIXED:
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,FIXEDDRIVEICON)))
				return m_fileName;
			break;
		case DRIVE_REMOTE:
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,NETWORKDRIVEICON)))
				return m_fileName;
			break;
		case DRIVE_CDROM:
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,CDROMDRIVEICON)))
				return m_fileName;
			break;
		case DRIVE_RAMDISK:
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,RAMDISKDRIVEICON)))
				return m_fileName;
			break;
		}

		return (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,UNKNOWNDRIVEICON)));
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadFolderIconFile(FileProperties* fileProps)
{
	// Is this a directory?
	if (fileProps->GetFileStatus() == FileProperties_Directory)
	{
		// Do we have an animation list in here?
		char animSearch[MAX_PATH];
		sprintf(animSearch, "%s\\*.*", fileProps->GetFileName());
		WIN32_FIND_DATA findData;
		HANDLE findHandle = FindFirstFile(animSearch, &findData);
		if(findHandle != INVALID_HANDLE_VALUE)
		{	while(findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
			{	if (!FindNextFile(findHandle,&findData))
				{	FindClose(findHandle);
					return FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,FOLDERICON));
				}
			}

			sprintf(animSearch, "%s\\%s", fileProps->GetFileName(), findData.cFileName);
			FindClose(findHandle);

			// Try to load this image into a bitmap
			WindowBitmap imageBMP;
			if (imageBMP.ReadBitmapFile(animSearch))
			{	WindowBitmap tinyFolder;
				
				tinyFolder.ReadBitmapFile(FindFiles_FindFile(FINDFILE_SKINS,IMAGEFOLDERICON));				

				// Stretch the loaded bitmap to the correct size
				WindowBitmap tinyBMP(tinyFolder.XRes,tinyFolder.YRes);
				RECT dstRect={ 0,0,tinyFolder.XRes,tinyFolder.YRes };
				imageBMP.WindowBitmap_AlphaBlend(tinyBMP.m_hdc,dstRect,BitmapTile_StretchX|BitmapTile_StretchY);

				// Overlay the image correctly
				tinyFolder.AlphaBitBlit(&tinyBMP);

				// Save this image in the Auto Folder
				return SaveAutoImage(fileProps, &tinyBMP);
			}
		}
		else FindClose(findHandle);
		return (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,FOLDERICON)));
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadInvalidIconFile(FileProperties* fileProps)
{
	if (fileProps->GetFileStatus() == FileProperties_MyComputer)
		return (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,MYCOMPUTERICON)));
	else if (fileProps->GetFileStatus() == FileProperties_NoFile)
		return (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,NOFILEICON)));
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadSpecialIconFile(FileProperties* fileProps)
{
	char filePath[MAX_PATH];
	FileProperties::GetIconFromFileName(fileProps->GetFileName(), filePath);
	char *Ret=FileIcon_SetIconFile(filePath);
	return Ret;	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadAutoIconFile(FileProperties* fileProps)
{
	char* ret = NULL;
	// Get the auto icon filename
	char filePath[MAX_PATH];
	FileProperties::GetAutoFromFileName(fileProps->GetFileName(), filePath);

	// See if just the folder exists
	char* lastSlash = strrchr(filePath, '\\');
	if (lastSlash)
	{
		*lastSlash = 0;
		if (CreateDirectory(filePath, NULL))
		{	SetFileAttributes(filePath, FILE_ATTRIBUTE_HIDDEN);
		}
		*lastSlash = '\\';

		char* origFileName = fileProps->GetFileName();
		char innerImage[MAX_PATH];
		if (fileProps->GetFileStatus() == FileProperties_Directory)
		{
			sprintf(innerImage, "%s\\*0.*", origFileName);
			origFileName = innerImage;
		}

		WIN32_FIND_DATA origFile;
		HANDLE findHandle1 = FindFirstFile(origFileName, &origFile);  
		WIN32_FIND_DATA autoFile;
		HANDLE findHandle2 = FindFirstFile(filePath, &autoFile);
		if((findHandle1!=INVALID_HANDLE_VALUE)&&(findHandle2!=INVALID_HANDLE_VALUE))
		{	
			// Check the 2 time stamps
			if (autoFile.ftLastWriteTime.dwHighDateTime > origFile.ftLastWriteTime.dwHighDateTime)
				ret = FileIcon_SetIconFile(filePath);
			else if (autoFile.ftLastWriteTime.dwHighDateTime == origFile.ftLastWriteTime.dwHighDateTime)
				if (autoFile.ftLastWriteTime.dwLowDateTime > origFile.ftLastWriteTime.dwLowDateTime)
					ret = FileIcon_SetIconFile(filePath);
		}
		FindClose(findHandle1);
		FindClose(findHandle2);
	}
	return ret;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::SaveAutoImage(FileProperties* fileProps, WindowBitmap* copyMe)
{
	copyMe->ForceTo32BPP();
	WindowBitmap littleIcon(FILEICONSIZEX, FILEICONSIZEY);
	WindowBitmap* writeMe = copyMe;
	if ((copyMe->XRes > FILEICONSIZEX) || (copyMe->YRes > FILEICONSIZEY))
	{
		RECT dstRect; dstRect.left = dstRect.top = 0;
		dstRect.right = FILEICONSIZEX - 1; dstRect.bottom = FILEICONSIZEY - 1;
		copyMe->WindowBitmap_AlphaBlend(littleIcon.m_hdc, dstRect, BitmapTile_StretchX | BitmapTile_StretchY);
		writeMe = &littleIcon;
	}
	char autoFileName[MAX_PATH];
	FileProperties::GetAutoFromFileName(fileProps->GetFileName(), autoFileName);
	if (writeMe->Write_JPEG(autoFileName))
	{
		if (FileIcon_SetIconFile(autoFileName))
			return this->m_fileName;
	}
	return FileIcon_SetIconFile(fileProps->GetFileName());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileIcon::ReadDefaultIconFile(FileProperties* fileProps)
{
	if (fileProps)
	{
		char* thisChar = strrchr(fileProps->GetFileName(), '.');
		if (thisChar)
		{
			thisChar++;
			char filePath[MAX_PATH];
			sprintf(filePath, "File Icons\\%s.%s", thisChar, ICONEXTENSION);
			if (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,filePath)))
				return m_fileName;
		} 
	}
	return (FileIcon_SetIconFile(FindFiles_FindFile(FINDFILE_SKINS,DEFAULTICON)));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileIcon::ScreenObject_RAW_DrawItem(RECT *rect,HDC hdc,RECT *ClippingRgn)
{
	if (m_useBitmap)
		return m_useBitmap->WindowBitmap_AlphaBlend(hdc, *rect, BitmapTile_StretchX|BitmapTile_StretchY);
	if ((m_amIAnimatingNow || m_showDefAnimFrame) && m_animationInterface)
	{
		if (m_showDefAnimFrame && !m_amIAnimatingNow) m_animationInterface->AnimatedIcon_RenderAtTime(m_defaultTime);
		return m_animationInterface->ScreenObject_DrawItem(rect, hdc);
	}
	if (!m_icon) return true;
	return m_icon->WindowBitmap_AlphaBlend(hdc, *rect, BitmapTile_StretchX|BitmapTile_StretchY);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileIcon::FileIcon_UseThisIcon(WindowBitmap* useThisOne, bool doIOwn)
{
	FileIcon_StopAnimation();
	if (m_useBitmap == useThisOne) return;
	if (m_useBitmap && m_doIOwn)
		delete m_useBitmap;
	if (useThisOne)
	{
		m_useBitmap = useThisOne;
		m_doIOwn = doIOwn;
	}
	else
	{
		m_useBitmap = NULL;
		m_doIOwn = false;
	}
	Changed(FileIconChanged_NewImageSet);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileIcon::CreateCustomIconInterface() {
	CustomIcon_Interface *FIC;
	unsigned NoPluginInfos;
	unsigned i;

	if (s_CustomIconInterface) return;
	s_CustomIconInterface=new tList<CustomIcon_Interface *>;
	// We cycle through all plugins that are of the 
	PluginClassInfo** Infos=NewTek_GetPluginInfo(NoPluginInfos);
	// Look at all the plugins, for the correct type
	for (i=0;i<NoPluginInfos;i++) {
		// Get the item
		PluginClassInfo* Nfo=Infos[i];
		// Is the item the correct type ?
		if (!strcmp(Nfo->Category,"CustomIcon_Interface")) {
			// Create the plugin
			PluginClass *PC=Nfo->New();
			FIC=GetInterface<CustomIcon_Interface>(PC);
			if (FIC) s_CustomIconInterface->Add(FIC);
			}
		}
	}


CustomIcon_Interface* FileIcon::GetCustomIconReader(FileProperties* fileprops,WindowBitmap* windowbmp) {
	CustomIcon_Interface *FIC;
	unsigned i,eoi;

	if ((!fileprops)&&(!windowbmp)) return NULL;
	//This will implicitly init the list only one time
	CreateCustomIconInterface();
	eoi=s_CustomIconInterface->NoItems;
	for (i=0;i<eoi;i++) {
		FIC=(*s_CustomIconInterface)[i];
		if ((FIC)&&(FIC->CustomIcon_CreateBitmap(fileprops,windowbmp))) {
			//Do not need to replace the item as it can be reused
			//(*s_CustomIconInterface)[i]=GetInterface<CustomIcon_Interface>(NewTek_New(GetTypeName(FIC)));
			return FIC;
			}
		}
	return (NULL);
	}


void FileIcon::CreateAnimatedIconInterface() {
	AnimatedIcon_Interface *FIC;
	unsigned NoPluginInfos;
	unsigned i;

	if (s_AnimatedIconInterface) return;
	s_AnimatedIconInterface=new tList<AnimatedIcon_Interface *>;
	// We cycle through all plugins that are of the 
	PluginClassInfo** Infos=NewTek_GetPluginInfo(NoPluginInfos);
	// Look at all the plugins, for the correct type
	for (i=0;i<NoPluginInfos;i++) {
		// Get the item
		PluginClassInfo* Nfo=Infos[i];
		// Is the item the correct type ?
		if (!strcmp(Nfo->Category,"AnimatedIcon_Interface")) {
			// Create the plugin
			PluginClass *PC=Nfo->New();
			FIC=GetInterface<AnimatedIcon_Interface>(PC);
			if (FIC) s_AnimatedIconInterface->Add(FIC);
			}
		}
	}


AnimatedIcon_Interface* FileIcon::GetAnimatedIconReader(char* fileName) {
	AnimatedIcon_Interface *FIC;
	unsigned i,eoi;

	if (!fileName) return NULL;
	//This will implicitly init the list only one time
	CreateAnimatedIconInterface();
	eoi=s_AnimatedIconInterface->NoItems;
	for (i=0;i<eoi;i++) {
		FIC=(*s_AnimatedIconInterface)[i];
		if ((FIC)&&(FIC->AnimatedIcon_CreateAnimatedIcon(fileName))) {
			(*s_AnimatedIconInterface)[i]=GetInterface<AnimatedIcon_Interface>(NewTek_New(GetTypeName(FIC)));
			return FIC;
			}
		}
	return (NULL);
	}


void FileIcon::OnProcessDetach() {
	unsigned i,eoi;
	
	if (s_AnimatedIconInterface) {
		eoi=s_AnimatedIconInterface->NoItems;
		for (i=0;i<eoi;i++) {
			NewTek_Delete((*s_AnimatedIconInterface)[i]);
			}
		delete s_AnimatedIconInterface;
		}
	if (s_CustomIconInterface) {
		eoi=s_CustomIconInterface->NoItems;
		for (i=0;i<eoi;i++) {
			NewTek_Delete((*s_CustomIconInterface)[i]);
			}
		delete s_CustomIconInterface;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileIcon::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (m_animationInterface == ItemChanging)
	{
		if (IsDeletion(String))
			m_animationInterface = NULL;
		else if (m_amIAnimatingNow)
			Changed();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
long FileIcon::ScreenObject_GetPreferedXSize(void)
{
	return -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
long FileIcon::ScreenObject_GetPreferedYSize(void)
{
	return -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileIcon::FileIcon_AmIAnimated()
{
	return !(m_useBitmap || !m_animationInterface);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////