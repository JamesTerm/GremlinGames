#include "stdafx.h"

bool FolderClick::Interface_FileClick_Execute	// Return whether you succeeded
						(	HWND hWnd,				// The control of the (first) window clicked upon
							char **FileName)		// The list of filenames that where clicked upon
{
	FileButton* fb = GetWindowInterface<FileButton>(hWnd);
	DirectoryLayout* dl = NULL;
	if (fb)
	{
		dl = GetWindowInterface<DirectoryLayout>(fb->GetParent());
		if (!dl)
		{
			PathLayout* pl = GetWindowInterface<PathLayout>(fb->GetParent());
			if (pl)
				dl = pl->GetDirectoryLayout();
		}
	}

	unsigned i = 0;
	bool foundFolder = false;
	while (FileName[i] && !foundFolder)
	{
		FileProperties* thisFile = FileProperties::GetHandle(FileName[i]);
		long status = thisFile->GetFileStatus();
		if ((status == FileProperties_Drive) || 
			(status == FileProperties_Directory) ||
			(status == FileProperties_MyComputer))
			{
				if (dl)
				{
					dl->SetDirectory(FileName[i]);
					foundFolder = true;
				}
				else
				{
					StretchyFileBin* fileBin = GetWindowInterface<StretchyFileBin>
						(Desktop_LaunchPlugin("StretchyFileBin", "File Bin",100,100,994,440,true));
					if (fileBin)
					{
						fileBin->StretchyFileBin_InitDirectory(FileName[i]);
						foundFolder = true;
					}
				}
			}
		FileProperties::ReleaseHandle(thisFile);
		i++;
	}
	return foundFolder;
}
/////////////////////////////////////////////////////////////////////////////////////////////
