#ifndef __DirectoryLayout_ContextMenuH_
#define __DirectoryLayout_ContextMenuH_


class FileManagementDLL DirectoryLayout_ContextMenu : public Interface_ContextMenu
{
public:
	virtual bool Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
	virtual bool Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
};
/////////////////////////////////////////////////////////////////////////////////////////////

#endif	// #ifndef __DirectoryLayout_ContextMenuH_