#include "stdafx.h"

HINSTANCE g_FileManagementhInst=NULL;

//**** The DLL Entry Point ****************************************************************
BOOL WINAPI DllMain(HINSTANCE hInst,ULONG ul_reason_for_call,LPVOID lpReserved)
{	switch(ul_reason_for_call)
	{								//**************************************************************************
		case DLL_PROCESS_ATTACH:	// Perform any initialisation here
									g_FileManagementhInst=hInst;
									UtilLib_FileButton_ViewMode::OnProcessAttach();
									FileProperties::OnProcessAttach();
									StretchyFileBin::OnProcessAttach();
									DisableThreadLibraryCalls(hInst);
									return true;

									//**************************************************************************
		case DLL_PROCESS_DETACH:	// Perform any destruction here
			
									//Note: I have moved the process detach code to a global hack function
									//apparently some of the dependant dlls have already been closed before
									//these can work properly

									g_FileManagementhInst=NULL;
									return true;

									//**************************************************************************
		default:					return true;
	}

	// Return Success
	return true;
}

void FilePropsCloseHack()
{
	FavoriteFileList::FreeInstance();	// Must do this before FileProperties::OnProcessDetach
	StretchyFileBin::OnProsessDetach();
	FileProperties::OnProcessDetach();
	UtilLib_FileButton_ViewMode::OnProcessDetach();

	Scripting_FavoriteFileList::OnProcessDetach();
};


NewTek_RegisterPluginControl(FileButton);
NewTek_RegisterPluginControl(FileButton_NoMenu);

NewTek_RegisterPluginControl(DirectoryLayout);
NewTek_RegisterPluginClass(Details_ViewMode);
NewTek_RegisterPluginControl(StretchyFileBin);
NewTek_RegisterPluginControl(PathLayout);

NewTek_RegisterPluginControl(FilePropertiesDlg);

NewTek_RegisterPluginClass(FolderClick);
NewTek_RegisterPluginType(FolderClick,Interface_FileClick,Rick Pingry,(c)2000 NewTek,1,/*Priority*/1001);

NewTek_RegisterPluginClass(FileButton_ShellExecute);
NewTek_RegisterPluginType(FileButton_ShellExecute,Interface_FileClick,Rick Pingry,(c)2000 NewTek,1,/*Priority*/1);	// When nothing else works

NewTek_RegisterPluginClass(FileButton_ContextMenu);
NewTek_RegisterPluginType(FileButton_ContextMenu,Interface_ContextMenu,Rick Pingry,(c)2000 NewTek,1,/*Priority*/20);

NewTek_RegisterPluginClass(DirectoryLayout_ContextMenu);
NewTek_RegisterPluginType(DirectoryLayout_ContextMenu,Interface_ContextMenu,Rick Pingry,(c)2000 NewTek,1,/*Priority*/1000);


NewTek_RegisterPluginClass(UtilLib_FileButton_ViewMode);

NewTek_RegisterPluginClass(FileButton_ViewMode_ContextMenu);
NewTek_RegisterPluginType(FileButton_ViewMode_ContextMenu,Interface_ContextMenu,Rick Pingry,(c)2000 NewTek,1,/*Priority*/1000);

