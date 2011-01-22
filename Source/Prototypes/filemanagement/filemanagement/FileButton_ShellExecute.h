#ifndef __FileButton_ShellExecuteH_
#define __FileButton_ShellExecuteH_

class FileButton_ShellExecute : public Interface_FileClick
{
public:			
	bool Interface_FileClick_Execute	//! Return whether you succeeded
							(	HWND hWnd,				//! The control of the (first) window clicked upon
								char **FileName);		//! The list of filenames that where clicked upon
};

#endif // __FileButton_ShellExecuteH_