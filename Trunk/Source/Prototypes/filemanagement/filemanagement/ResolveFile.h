#ifndef __ResolveFileH_
#define __ResolveFileH_

//! This global function allows you to resolve a filename
//! Pass in a char* pointer with a filename
//! This function will try to resolve the existance of the file
//! It will also resolve the existance of a folder or drive
//! If the file does not exist, it will look for missing file information from AssetManagement
//! If the AssetManagement replacement exists, prompt the user for its use and recursivley search
//! If that does not exist, Prompt the User to Browse for the file, bring up a FileSelector, Maybe do a search
//! Finally, the function will replace the filename parameter with the one it found

enum	ResolveFileType
		{	ResolveFile_FileNotFound,
			ResolveFile_FileFound,
			ResolveFile_FileFoundWithChangedName
		};

FileManagementDLL ResolveFileType NewTek_ResolveFile
	( const char *FileNameWithPath, unsigned BLSFlags, char *DestBuffer, long bufferSize=MAX_PATH, bool suppresDialogs=false );

#endif	//! #ifndef __ResolveFileH_