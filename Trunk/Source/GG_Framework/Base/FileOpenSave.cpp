// FileOpenSave.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"

//! Returns a static string with the filename or NULL if user canceled
std::string GG_Framework::Base::OpenFileDialog(
	const char* title,
	const char* startFile)
{
	static OpenThreads::Mutex mutex;
	GG_Framework::Base::RefMutexWrapper rmw(mutex);
	OPENFILENAME ofn; 
	memset(&ofn, 0, sizeof(OPENFILENAME)); 
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = NULL; 
	ofn.lpstrFilter = NULL; 
	ofn.nFilterIndex = 0; 
	static char dest[MAX_PATH]; 
	strcpy(dest, startFile?startFile:"");
	ofn.lpstrFile = dest; 
	ofn.nMaxFile = MAX_PATH; 
	ofn.lpstrFileTitle = NULL; 
	ofn.nMaxFileTitle = MAX_PATH; 
	ofn.lpstrDefExt = NULL; 
	ofn.lpstrTitle  = title; 
	ofn.Flags = OFN_HIDEREADONLY; 

	std::string ret;
	if(GetOpenFileName(&ofn) != FALSE)
		ret = dest; 

	return ret;
}

//! Returns a static string with the filename or NULL if user canceled
std::string GG_Framework::Base::SaveFileDialog(
	const char* title,
	const char* startFile)
{
	static OpenThreads::Mutex mutex;
	GG_Framework::Base::RefMutexWrapper rmw(mutex);
	OPENFILENAME ofn; 
	memset(&ofn, 0, sizeof(OPENFILENAME)); 
	ofn.lStructSize = sizeof(OPENFILENAME); 
	ofn.hwndOwner = NULL; 
	ofn.lpstrFilter = NULL; 
	ofn.nFilterIndex = 0; 
	static char dest[MAX_PATH]; 
	strcpy(dest, startFile?startFile:"");
	ofn.lpstrFile = dest; 
	ofn.nMaxFile = MAX_PATH; 
	ofn.lpstrFileTitle = NULL; 
	ofn.nMaxFileTitle = MAX_PATH; 
	ofn.lpstrDefExt = NULL; 
	ofn.lpstrTitle  = title; 
	ofn.Flags = OFN_HIDEREADONLY; 

	std::string ret;
	if(GetSaveFileName(&ofn) != FALSE)
		ret = dest; 
	
	return ret;
}