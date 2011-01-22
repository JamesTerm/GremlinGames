//! My Pre-Declaration
class FolderClick;

#ifndef __FolderClickH_
#define __FolderClickH_

/*	FolderClick
The Interface_FileClick that manages opening a folder or Drive in a StretchyFileBin
*/

class FileManagementDLL FolderClick : public Interface_FileClick
{	public:			
	bool Interface_FileClick_Execute	//! Return whether you succeeded
							(	HWND hWnd,				//! The control of the (first) window clicked upon
								char **FileName);		//! The list of filenames that where clicked upon
};
/////////////////////////////////////////////////////////////////////////////////////////////

#endif	//#ifndef __FolderClickH_