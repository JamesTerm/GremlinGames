#include "stdafx.h"

bool FileButton_ShellExecute::Interface_FileClick_Execute	//! Return whether you succeeded
							(	HWND hWnd,				//! The control of the (first) window clicked upon
								char **FileName)		//! The list of filenames that where clicked upon
{
	if (ApplicationInstance_TriCaster()) return false;

	// Iterate through the list of files and call Shell Execute on each item
	// This Command should be called dead last (Has a priority of 1)
	char* thisFile = *FileName;
	static char directory[128];
	char* errorMessage = directory;
	bool ret = false;
	while (thisFile)
	{
		strcpy(directory, thisFile);
		char* nameOnly = NewTek_GetFileNameFromPath(directory);

		// Do not relaunch ourselves
		char *pExtension = NewTek_GetLastDot( nameOnly );
		if ((pExtension)&&(stricmp(pExtension, TLF_BASE_FILEEXT )))
		{	// Now check its not a folder
			if (nameOnly && (nameOnly != directory))
			{
				// Seperate the name from the directory
				*(nameOnly - 1) = 0;

				int errRet = (int)ShellExecute(NewTek_GetGlobalParentHWND(), NULL, nameOnly, NULL, directory, SW_SHOW);
				if (errRet < 33)
				{
					bool ShowErrorMessage = true;
					char SystemPath[MAX_PATH];
					if (errRet == SE_ERR_NOASSOC && GetSystemDirectory(SystemPath, MAX_PATH) != 0)
					{
						const size_t RunParametersLength = MAX_PATH + 64;
						char RunParameters[RunParametersLength];

						_snprintf(RunParameters, RunParametersLength, "shell32.dll,OpenAs_RunDLL %s\\%s", directory, nameOnly);
						ShowErrorMessage = ((int)ShellExecute(NewTek_GetGlobalParentHWND(), "open", "rundll32.exe", RunParameters, SystemPath, SW_SHOW) < 33);
					}

					if (ShowErrorMessage)
					{
						sprintf(errorMessage, TL_GetString( "TR::" "There was an error performing Shell Execute on \"%s\"", "There was an error performing Shell Execute on \"%s\"" ), thisFile);
						NewTek_MessageBox(NULL, errorMessage, TL_GetString( "TR::" "Error Executing File", "Error Executing File" ), MB_OK, NULL);
					}
				}
				else ret = true;
			}
		}

		// Look at the next file
		FileName++;
		thisFile = *FileName;
	}
	return ret;
}