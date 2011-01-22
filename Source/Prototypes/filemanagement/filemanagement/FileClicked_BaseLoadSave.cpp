#include "StdAfx.h"

//****************************************************************************************************************************************************
bool LoadSaveDropOnSkin::DragNDropEx_CanItemBeDroppedHere(BaseWindowClass *BWC,Control_DragNDrop_DropInfo *Dropped)
{	// you can only tint popup windows !
	if (!NewTek_IsPopup(BWC->GetWindowHandle())) return false;
	
	FileButton		*l_FileButton	=GetWindowInterface<FileButton>(Dropped->hWnd);
	BaseLoadSave	*l_BaseLoadSave	=GetInterface<BaseLoadSave>(BWC);
	if ((l_FileButton)&&(l_BaseLoadSave)) 
	{	
		return NewTek_IsLSDFile( (char*)l_FileButton->GetFileName() );
	}
	return false;
}

WindowLayout_Item *LoadSaveDropOnSkin::DragNDropEx_DropItemsHere(	BaseWindowClass *BWC,int Width,int Height,
																	int MousePosnX,int MousePosnY,
																	WindowLayout_Item *ObjectsInWindow,
																	WindowLayout_Item *ObjectsDropped,
																	bool Resizing,bool DroppedHere
																)
{	if ((DroppedHere)&&(ObjectsDropped))
	{	// Get the loadsave class !
		BaseLoadSave *l_BaseLoadSave=GetInterface<BaseLoadSave>(BWC);

		if (l_BaseLoadSave)
		{	// Do it !
			WindowLayout_Item *Scan=ObjectsDropped;

			while(Scan)
			{	// Get information about the filebutton
				FileButton *l_FileButton=GetWindowInterface<FileButton>(Scan->hWnd);				
				if (l_FileButton)
				{	// Get the plugin information
					LoadPlugin_GetType_Info PluginInfo;
					LoadData				FileToLoad(l_FileButton->GetFileName(),LoadData_Flag_DragInto);

					if (!FileToLoad.Error())
					{	if (LoadPlugin_Type(&FileToLoad,&PluginInfo))
						{	// We <NEED> to use strong type checking or things can go disastrously wrong
							LoadPlugin_Data(&PluginInfo,l_BaseLoadSave,&FileToLoad,NULL,true);
						}
					}
				}

				// look at the next item
				Scan=Scan->Next;
			}
		}

		// Delete all the objects
		while(ObjectsDropped)
		{	WindowLayout_Item *Next=ObjectsDropped->Next;
			BaseWindowClass *BWC=GetWindowInterface<BaseWindowClass>(ObjectsDropped->hWnd);
			if (BWC)	BWC->DeferredDelete();
			else		CloseWindow(ObjectsDropped->hWnd);
			ObjectsDropped=Next;
		}
	}
	else
	{	// Merge the lists
		while(ObjectsDropped)
		{	WindowLayout_Item *Next=ObjectsDropped->Next;
			ObjectsDropped->Next=ObjectsInWindow;
			ObjectsInWindow=ObjectsDropped;
			ObjectsDropped=Next;
		}
	}
	
	return ObjectsInWindow;
}

NewTek_RegisterPluginClass(LoadSaveDropOnSkin);
NewTek_RegisterPluginType (LoadSaveDropOnSkin,Control_DragNDropEx,Andrew Cross,(c)2000 NewTek,1,/*Priority*/1000);

//****************************************************************************************************************************************************
bool BaseLoadSave_FileClick::Interface_FileClick_Execute(HWND hWnd,char **FileName)
{	// Was a file loaded
	bool FileLoaded=false;
	
	// Cycle through all the filenames
	unsigned i=0;
	while(FileName[i])
	{	// Try getting plugin info on this type
		LoadData				FileToLoad(FileName[i],LoadData_Flag_Launch);
		if (!FileToLoad.Error())
		{	if (Desktop_LaunchPlugin_FromLoadContext(&FileToLoad)) 
					FileLoaded=true;
		}

		// Look at the next filename
		i++;
	}

	// Was the file loaded ?
	return FileLoaded;
}

NewTek_RegisterPluginClass(BaseLoadSave_FileClick);
NewTek_RegisterPluginType(BaseLoadSave_FileClick,Interface_FileClick,Andrew Cross,(c)2000 NewTek,1,/*Priority*/2001);