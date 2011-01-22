#include "stdafx.h"


bool FileButton_ContextMenu::Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	bool driveFound = false;
	bool fileFound = false;
	bool readingIcon = false;
	bool wantReadIcon = false;
	long status;

	for (unsigned i = 0; i < controlList->NoItems() && !driveFound; i++)
	{
		FileButton* fb = GetInterface<FileButton>((*controlList)[i]);
		if (fb && ((status = fb->GetStatus()) != FileProperties_NoFile) && fb->FileButton_ShouldIShowStandardContextMenu())
		{
			FileButton_ViewMode_Parent* parent = fb->GetViewModeParent();
			if (parent && !parent->ShouldIShowCutCopyPasteContextMenu(fb))
				return false;
			fileFound = true;
			if ((status == FileProperties_Drive) || (status == FileProperties_MyComputer))
				driveFound = true;
			int threadReadStatus = fb->FB_GetThreadReadStatus();
			readingIcon |= (threadReadStatus == FileButton_ThreadRead_Reading);
			wantReadIcon |= (threadReadStatus == FileButton_ThreadRead_PreferedNo);
		}
	}
	if (fileFound)
	{

		if (!driveFound)
		{
			AddMenuItem(menuItems, MenuStr_Clipboard__title);
			AddMenuItem(menuItems, MenuStr_Clipboard_Cut);
			AddMenuItem(menuItems, MenuStr_Clipboard_Copy);
		}

		if (controlList->NoItems() == 1)
		{
			if ((status == FileProperties_Drive) || (status == FileProperties_Directory))
				if (FileProperties::s_editList->m_editList.NoItems())
				{
					if (driveFound)
						AddMenuItem(menuItems, MenuStr_Clipboard__title);
					AddMenuItem(menuItems, MenuStr_Clipboard_PasteInto);
				}
		}
		if (!driveFound)
			AddMenuItem(menuItems, MenuStr_Clipboard_Delete);

		if (!ApplicationInstance_TriCaster())
		{	AddMenuItem(menuItems, MenuStr_File__title);
			AddMenuItem(menuItems, MenuStr_File_Open);
			AddMenuItem(menuItems, MenuStr_File_Properties);
			if (wantReadIcon || readingIcon)
			{
				AddMenuItem(menuItems, MenuStr_IconLoading__title);
				if (wantReadIcon)
					AddMenuItem(menuItems, MenuStr_IconLoading_LoadFileIcon);
				if (readingIcon)
					AddMenuItem(menuItems, MenuStr_IconLoading_CancelLoadFileIcon);
			}
		}
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileButton_ContextMenu::Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	// Get a list of all of the WindowNames
	tList<FileProperties*> WindowNames;
	tList<FileButton*>		FileButtons;
	FileButton* firstFile = NULL;
	for (unsigned i = 0; i < controlList->NoItems(); i++)
	{
		FileButton* fb = NULL;
#ifndef __DISABLE_EXCEPTION_HANDLING
		try
#endif
		{	
			fb = GetInterface<FileButton>((*controlList)[i]);
			if (fb && !fb->FileButton_ShouldIShowStandardContextMenu())
				fb = NULL;
		}
#ifndef __DISABLE_EXCEPTION_HANDLING
		catch (...)
		{}
#endif
		if (fb)
		{
			WindowNames.Add(fb->GetProperties());
			FileButtons.Add(fb);
			if (!firstFile)
			{
				firstFile = fb;
				if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_PasteInto))
				{
					FileProperties::EditPaste(fb->GetFileName());
					return true;
				}
			}
		}
	}

	if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_File_Open))
	{
		if (firstFile)
		{
			tList<const char*> fileNames;
			for (unsigned i = 0; i < WindowNames.NoItems(); i++)
				fileNames.Add( WindowNames[i]->GetFileName() );
			fileNames.Add(NULL);
			NewTek_FileExecute( firstFile->GetWindowHandle() , (char**)&fileNames[0] );
		}
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_File_Properties))
	{
		if (firstFile)
		{
			BaseWindowClass* popupParent = 
				GetWindowInterface<BaseWindowClass>(NewTek_GetPopupParent(firstFile->GetWindowHandle()));
			if (!popupParent) popupParent = firstFile;
			FileProperties::EditProperties(&WindowNames, popupParent);
		}
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Cut))
	{
		FileProperties::EditCut(&WindowNames);
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Copy))
	{
		FileProperties::EditCopy(&WindowNames);
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_Clipboard_Delete))
	{
		FileProperties::EditDelete(&WindowNames);
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_IconLoading_LoadFileIcon))
	{
		for (unsigned i = 0; i < FileButtons.NoItems(); i++)
			FileButtons[i]->FB_ThreadReadIcon();
		return true;
	}
	else if (!strcmp(m_menuList[menuItem]->m_string, MenuStr_IconLoading_CancelLoadFileIcon))
	{
		for (unsigned i = 0; i < FileButtons.NoItems(); i++)
			FileButtons[i]->FB_CancelThreadReading();
		return true;
	}
	
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
