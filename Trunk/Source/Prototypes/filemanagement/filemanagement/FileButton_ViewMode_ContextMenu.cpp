#include "stdafx.h"

bool FileButton_ViewMode_ContextMenu::Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	for (unsigned i = 0; i < controlList->NoItems(); i++)
	{
		FileButton* fb = GetInterface<FileButton>((*controlList)[i]);
		if (fb && !fb->GetViewModeParent() && (&(fb->m_internalViewMode) == fb->GetCurrentViewMode()))
		{
			AddMenuItem(menuItems, MenuStr_FileBinViewModes__ViewModes__title);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_LargeIcon);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_SmallIcon);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_NameOnly);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_IconOnly);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes__FileNameDisplay__title);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_ShowAlias);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_NameWOExtension);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_NameWithExtension);
			AddMenuItem(menuItems, MenuStr_FileBinViewModes_ShowFullPath);

			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool FileButton_ViewMode_ContextMenu::Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	for (unsigned i = 0; i < controlList->NoItems(); i++)
	{
		FileButton* fb = GetInterface<FileButton>((*controlList)[i]);
		if (fb && !fb->GetViewModeParent() && (&(fb->m_internalViewMode) == fb->GetCurrentViewMode()))
		{
			if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_LargeIcon))
				fb->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_SuperLargeIcon))	// mwatkins
				fb->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_SuperLargeIcon);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_SmallIcon))
				fb->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_SmallIcon);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_NameOnly))
				fb->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_NameOnly);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_IconOnly))
				fb->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_IconOnly);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_ShowAlias))
				fb->m_internalViewMode.FileButton_ViewMode_SetSortString(FileButton_Naming_AltName);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_NameWOExtension))
				fb->m_internalViewMode.FileButton_ViewMode_SetSortString(FileButton_Naming_NameOnly);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_NameWithExtension))
				fb->m_internalViewMode.FileButton_ViewMode_SetSortString(FileButton_Naming_FullName);
			else if (!strcmp(this->m_menuList[menuItem]->m_string, MenuStr_FileBinViewModes_ShowFullPath))
				fb->m_internalViewMode.FileButton_ViewMode_SetSortString(FileButton_Naming_Path);
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////