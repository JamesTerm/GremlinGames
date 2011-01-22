#ifndef __FileButton_ViewMode_ContextMenuH_
#define __FileButton_ViewMode_ContextMenuH_

/*	Some windows do not want to have children display the srandard elements for a 
	context "File" context menu. If you are such a window, inheriting of this itnterface
	will allow you to dynamically choose whether you want this menu to show or not.
*/

class FileButton_FileContextMenuQuery
{	public:		virtual bool FileContextMenuQuery_ShowMenu( const FileButton *FB ) = 0;
};

class FileManagementDLL FileButton_ViewMode_ContextMenu : public Interface_ContextMenu
{
public:
	virtual bool Interface_GetMenuItems(tList<StretchyMenuGroup*>* menuItems, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
	virtual bool Interface_ExecuteMenuItem(int menuItem, tList<BaseWindowClass*>* controlList, BaseWindowClass* clickedWindow);
};


#endif	//! #ifndef __FileButton_ViewMode_ContextMenuH_