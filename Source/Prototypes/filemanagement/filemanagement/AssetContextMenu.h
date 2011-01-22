#ifndef __AssetContextMenuH_
#define __AssetContextMenuH_

class FileManagementDLL AssetContextMenu : public Interface_ContextMenu
{
private:
	int m_index;
public:
	virtual bool Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
	virtual bool Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
};
/////////////////////////////////////////////////////////////////////////////////////////////

#endif	// #ifndef __AssetContextMenuH_