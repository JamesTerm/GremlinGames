#ifndef __FILECLICKED_BASELOADSAVE__
#define __FILECLICKED_BASELOADSAVE__

//*************************************************************************************************************************************************
//! A file has been double clicked upon
class FileManagementDLL BaseLoadSave_FileClick : public Interface_FileClick
{	public:		bool Interface_FileClick_Execute(HWND hWnd,char **FileName);
};

//******************************************************************************************************************************************
//! A file has been dropped into a window
class FileManagementDLL LoadSaveDropOnSkin : public Control_DragNDropEx
{	public:		virtual bool DragNDropEx_CanItemBeDroppedHere(BaseWindowClass *BWC,Control_DragNDrop_DropInfo *Dropped);
				virtual WindowLayout_Item *DragNDropEx_DropItemsHere(	BaseWindowClass *BWC,int Width,int Height,
																		int MousePosnX,int MousePosnY,
																		WindowLayout_Item *ObjectsInWindow,
																		WindowLayout_Item *ObjectsDropped,
																		bool Resizing,bool DroppedHere
																	);

				//! We only select these options <if> the window does not want them already
				bool DragNDropEx_IsHigherPriorityThanWindowDefault(void) { return false; } 
				
};

#endif