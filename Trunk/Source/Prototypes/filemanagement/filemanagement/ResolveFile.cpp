#include "stdafx.h"

// This global function allows you to resolve a filename
// Pass in a char* pointer with a filename
// This function will try to resolve the existance of the file
// It will also resolve the existance of a folder or drive
// If the file does not exist, it will look for missing file information from AssetManagement
// If the AssetManagement replacement exists, prompt the user for its use and recursivley search
// If that does not exist, Prompt the User to Browse for the file, bring up a FileSelector, Maybe do a search
// Finally, the function will replace the filename parameter with the one it found


ResolveFileType NewTek_ResolveFile( const char *FileNameWithPath, unsigned BLSFlags, char *DestBuffer, 
									long bufferSize, bool suppresDialogs )
{
	ResolveFileType ret=ResolveFile_FileNotFound;
	if (NewTek_DoesFileExist(FileNameWithPath))
	{
		ret=ResolveFile_FileFound;
		strncpy(DestBuffer,FileNameWithPath,bufferSize);
	}
	else if (BLSFlags&(LoadData_Flag_LoadMenu|SaveData_Flag_SaveMenu))
	{	// See if the file is in the users default search path
		const char *Source=Preferences::GetValue_StringDirectory("Missing files search path","\\");

		// There needs to be some sanity in the world, we simply cannot search the entire root
		// path for files
		if ( (!strcmp(Source,"\\")) || (!strcmp(Source,"/")) )
		{	
			// DO not seatch from the root on the application drive !
			int vh=1;
		}
		else if ((Source)&&(Source[0]!=0))
		{
			tList<char*> FileNames;
			char Temp[MAX_PATH];
			
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext [_MAX_EXT];

			//Create project path with file to search for
			_splitpath(FileNameWithPath,NULL,NULL,fname,ext);	//Look for this file
			_splitpath(Source,drive,dir,NULL,NULL);				//In this path
			strcpy(Temp,drive);
			strcat(Temp,dir);
			strcat(Temp,fname);
			strcat(Temp,ext);									//constructing the new full path and name

			if (drive[0]!=0) //we must have a drive otherwise this search will take forever!
				FindFiles_Search(Temp,FileNames,false,false,3);				//Search recursively for it

			if (FileNames.NoItems()!=0)
			{
				ret=ResolveFile_FileFound;
				//Now to find out what the path was for the first file found
				_splitpath(FileNames[0],NULL,dir,NULL,NULL);
				FindFiles_SearchBelowApplication_Free(FileNames); //Im done with this list
				strcpy(Temp,drive);
				strcat(Temp,dir);
				strcat(Temp,fname);
				strcat(Temp,ext);								//constructing the new full path and name

				//Now to send this to the dest buffer
				strncpy(DestBuffer,Temp,bufferSize);
			}
		}
		
	}
	return ret;
}