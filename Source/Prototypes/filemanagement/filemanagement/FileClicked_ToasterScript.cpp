#include "StdAfx.h"

//******************************************************************************************************************************************
bool ToasterScriptDropOnSkin::DragNDropEx_CanItemBeDroppedHere(BaseWindowClass *BWC,Control_DragNDrop_DropInfo *Dropped)
{	FileButton			*l_FileButton		=GetWindowInterface<FileButton>(Dropped->hWnd);
	Script_Interface	*l_Script_Interface	=GetInterface<Script_Interface>(BWC);
	if ((l_FileButton)&&(l_Script_Interface	)) 
	{	// Is it a toasterscript file !
		const char *Extension=NewTek_GetLastDot(l_FileButton->GetFileName());

		// See if it is of the correct extension
		if ( (Extension)&&
			 ((!stricmp(Extension,".ToasterScript"))||
              (!stricmp(Extension,".TScript"))||
			  (!stricmp(Extension,".Script"))) ) 
						return true;
	}
	return false;
}

WindowLayout_Item *ToasterScriptDropOnSkin::DragNDropEx_DropItemsHere(	BaseWindowClass *BWC,int Width,int Height,
																		int MousePosnX,int MousePosnY,
																		WindowLayout_Item *ObjectsInWindow,
																		WindowLayout_Item *ObjectsDropped,
																		bool Resizing,bool DroppedHere
																		)
{	if ((DroppedHere)&&(ObjectsDropped))
	{	// Get the loadsave class !
		Script_Interface *l_Script_Interface=GetInterface<Script_Interface>(BWC);

		if (l_Script_Interface)
		{	// Do it !
			WindowLayout_Item *Scan=ObjectsDropped;

			while(Scan)
			{	// Get information about the filebutton
				FileButton *l_FileButton=GetWindowInterface<FileButton>(Scan->hWnd);				
				if (l_FileButton)
				{	// We need to execute this toasterscript on the global interpretor context !
					Script_Run *ScriptToRun=Script_Run::LoadFromFile( (char*)l_FileButton->GetFileName() );

					// If we loaded it, run it and then delet it
					if (ScriptToRun)
					{	ScriptToRun->Run(l_Script_Interface);

						// Delete it
						delete ScriptToRun;
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
			if (BWC)	NewTek_Delete(BWC);
			else		CloseWindow(ObjectsDropped->hWnd);
			ObjectsDropped=Next;
		}
	}
	else
	{	// Delete all the objects
		while(ObjectsDropped)
		{	WindowLayout_Item *Next=ObjectsDropped->Next;
			ObjectsDropped->Next=ObjectsInWindow;
			ObjectsInWindow=ObjectsDropped;
			ObjectsDropped=Next;
		}
	}
	
	return ObjectsInWindow;
}

NewTek_RegisterPluginClass(ToasterScriptDropOnSkin);
NewTek_RegisterPluginType (ToasterScriptDropOnSkin,Control_DragNDropEx,Andrew Cross,(c)2000 NewTek,1,/*Priority*/1000);

//******************************************************************************************************************************************
bool BaseLoadSave_ToasterScript::Interface_FileClick_Execute(HWND hWnd,char **FileName)
{	// Was a file loaded
	bool FileLoaded=false;
	
	// Cycle through all the filenames
	unsigned i=0;
	while(FileName[i])
	{	// Is it a toasterscript file !
		const char *Extension=NewTek_GetLastDot(FileName[i]);

		// See if it is of the correct extension
		if ( (Extension)&&
			 ((!stricmp(Extension,".ToasterScript"))||
              (!stricmp(Extension,".TScript"))||
			  (!stricmp(Extension,".Script"))) )
		{	// We need to execute this toasterscript on the global interpretor context !
			Script_Run *ScriptToRun=Script_Run::LoadFromFile(FileName[i]);

			// If we loaded it, run it and then delet it
			if (ScriptToRun)
			{	ScriptToRun->Run(NULL);

				// Delete it
				delete ScriptToRun;
			}

			// Ok, a file was executed
			FileLoaded=true;
		}

		// Look at the next filename
		i++;
	}

	// Return success or failure
	return FileLoaded;
}


NewTek_RegisterPluginClass(BaseLoadSave_ToasterScript);
NewTek_RegisterPluginType(BaseLoadSave_ToasterScript,Interface_FileClick,Andrew Cross,(c)2000 NewTek,1,/*Priority*/1000);