#include "stdafx.h"

bool AssetContextMenu::Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{	// Changed by Andrew
	return false;

	m_index = 0;
	if (controlList->NoItems == 1)
	{
		AssetListListener* sc = GetInterface<AssetListListener>((*controlList)[0]);
		if (sc)
		{
			FileProperties* fp = GetInterface<FileProperties>(sc->GetAssetList());
			if (fp)
			{
				if (menuItems->NoItems)
				{
					m_index = 1;
					AddMenuItem(menuItems, Context_Menu_Separator);
				}
				AddMenuItem(menuItems, "Edit Skin Assets");
				return true;
			}
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool AssetContextMenu::Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow)
{
	AssetListListener* sc = GetInterface<AssetListListener>((*controlList)[0]);
	if ((sc) && (menuItem == m_index))
	{
		tList<FileProperties*> fileList;
		FileProperties* fp = GetInterface<FileProperties>(sc->GetAssetList());
		if (fp)
		{
			fp->GetHandle();
			fileList.Add(fp);
			DirectoryLayout::EditProperties(&fileList);
			fp->ReleaseHandle();
			return true;
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
