#include "stdafx.h"

bool DirectoryLayout_ContextMenu::Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	// Find the first Directory Layout
	DirectoryLayout* dl = NULL;
	for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
		dl = GetInterface<DirectoryLayout>((*controlList)[i]);

	if (!dl)
	{	StretchyFileBin* sfb = NULL;
		for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
			sfb = GetInterface<StretchyFileBin>((*controlList)[i]);
		if (sfb) dl=sfb->GetFileLayout();
	}
	
	if (!dl)
	{	PathLayout* sfb = NULL;
		for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
			sfb = GetInterface<PathLayout>((*controlList)[i]);
		if (sfb) dl=sfb->GetDirectoryLayout();
	}

	if (dl)
	{


		if (strcmp(dl->GetDirectory(), FILEBIN_MYCOMPUTER))
		{

			bool filesSelected = false;
			tList<HWND> ChildrenList;
			dl->GetChildren(ChildrenList);
			for(unsigned i=0;i<ChildrenList.NoItems()&&!filesSelected;i++)
			{	
				FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
				if ((FB)&&(FB->DragAndDrop_AmISelected())&&(FB->IsWindowVisible()))
					filesSelected = true;
			}
			
			if (filesSelected || (FileProperties::s_editList->m_editList.NoItems()))
			{
				AddMenuItem(menuItems, MenuStr_Clipboard__title);

				if (filesSelected)
				{
					AddMenuItem(menuItems, MenuStr_Clipboard_Cut);
					AddMenuItem(menuItems, MenuStr_Clipboard_Copy);
				}

				if (FileProperties::s_editList->m_editList.NoItems())
					AddMenuItem(menuItems, MenuStr_Clipboard_Paste);

				if (filesSelected)
				{
					AddMenuItem(menuItems, MenuStr_Clipboard_Delete);

					if (!ApplicationInstance_TriCaster())
					{	AddMenuItem(menuItems, MenuStr_File__title);
						AddMenuItem(menuItems, MenuStr_File_Open);
						AddMenuItem(menuItems, MenuStr_File_Properties);
					}
				}
			}
		}
		// We can always open a Open New Window
		AddMenuItem(menuItems, MenuStr_FileBin__title);

		// ADJC
		//if (!ApplicationInstance_TriCaster())
		//{	AddMenuItem(menuItems, MenuStr_FileBin_NewWindow);
		//	AddMenuItem(menuItems, "%Divider");
		//}
		
		if (strcmp(dl->GetDirectory(), FILEBIN_MYCOMPUTER))
			AddMenuItem(menuItems, MenuStr_FileBin_NewFolder);		

		if (!ApplicationInstance_TriCaster())
		{	if (strcmp(dl->GetDirectory(), FILEBIN_MYCOMPUTER))
				AddMenuItem(menuItems, MenuStr_FileBin_PropertiesFileBin);
		
			AddMenuItem(menuItems, MenuStr_FileBin_SetDirectoryPath);
			AddMenuItem(menuItems, MenuStr_FileBin__FileBinTools__title);
			AddMenuItem(menuItems, MenuStr_FileBin_CleanUpMissingFiles);
			AddMenuItem(menuItems, MenuStr_FileBin_CleanUpTemporaryFiles);
		}

		return true;
	}

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool DirectoryLayout_ContextMenu::Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	// Find the first Directory Layout
	DirectoryLayout* dl = NULL;
	for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
		dl = GetInterface<DirectoryLayout>((*controlList)[i]);

	if (!dl)
	{	StretchyFileBin* sfb = NULL;
		for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
			sfb = GetInterface<StretchyFileBin>((*controlList)[i]);
		if (sfb) dl=sfb->GetFileLayout();
	}	

	if (!dl)
	{	PathLayout* sfb = NULL;
		for (unsigned i = 0; i < controlList->NoItems() && !dl; i++)
			sfb = GetInterface<PathLayout>((*controlList)[i]);
		if (sfb) dl=sfb->GetDirectoryLayout();
	}

	if (dl)
	{
		if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_NewFolder))
			dl->DirLayout_CreateNewFolder();

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_NewWindow))
		{
			// Open up a new FileBin here
			StretchyFileBin* fileBin = GetWindowInterface<StretchyFileBin>
				(Desktop_LaunchPlugin("StretchyFileBin", "File Bin",dl->GetWindowPosX()+10,dl->GetWindowPosY()+10,
				994,440,true));
			if (fileBin) fileBin->StretchyFileBin_InitDirectory(dl->GetDirectory());
		}

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_PropertiesFileBin))
		{
			tList<FileProperties*> WindowNames;
			tList<FileProperties*>* hist = dl->GetHistory();
			WindowNames.Add((*hist)[dl->GetCurrentPosition()]);
			BaseWindowClass* popupParent = 
				GetWindowInterface<BaseWindowClass>(NewTek_GetPopupParent(dl->GetWindowHandle()));
			if (!popupParent) popupParent = dl;
			FileProperties::EditProperties(&WindowNames, popupParent);
		}

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Paste))
			FileProperties::EditPaste(dl->GetDirectory());

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_File_Open))
		{
			// Get the first selected file in the dl and execute it
			tList<HWND> ChildrenList;
			dl->GetChildren(ChildrenList);
			for(unsigned i=0;i<ChildrenList.NoItems();i++)
			{	
				FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
				if ((FB)&&(FB->DragAndDrop_AmISelected())&&(FB->IsWindowVisible()))
				{
					FB->MouseLButtonDblClick(0,0,0);
					return true;
				}
			}
		}

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Cut))
			dl->EditCut();

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Copy))
			dl->EditCopy();

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Delete))
			dl->EditDelete();

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_File_Properties))
			dl->EditProperties();
		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_SetDirectoryPath))
		{	
			long reasonForExit;
			char *NewText=NewTek_GetTextFromPopupWindow("Set the Directory Path",dl->GetDirectory(),*dl, reasonForExit);
			if (NewText)
			{
				// Set the directory here
				if (reasonForExit != Popup_Escape_EscapeKey)
					dl->SetDirectory(NewText);
				NewTek_free(NewText);
			}
		}

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_CleanUpTemporaryFiles))
		{
			NewTek_UIThreadIsBusy_Inst Thinking;
			FileProperties::CleanUp_NewTekInfo
				(dl->GetDirectory(), true, Delete_EditCache | CleanUp_AllFiles | Delete_TempFiles, false);
		}

		else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_FileBin_CleanUpMissingFiles))
		{
			NewTek_UIThreadIsBusy_Inst Thinking;
			FileProperties::CleanUp_NewTekInfo
				(dl->GetDirectory(), true, CleanUp_MissingFiles | Delete_AssetManagement | Delete_TempFiles | Delete_NonRegenerated, false);
		}
		return true;

	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////