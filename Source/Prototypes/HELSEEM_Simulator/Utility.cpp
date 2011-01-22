// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/Utility.cpp,v 1.1 2004/05/14 18:56:09 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Utility.h"

#include <stdio.h>

// Man, what a HASSLE!
void GetProductInfo(const char* verInfo, char* retBuff, DWORD buffLen)
{
	// Grab the name of this exe
		char exePath[MAX_PATH];
		GetModuleFileName(NULL, exePath, MAX_PATH);

	// Get the length of the version info we want
		DWORD dwHandle;
		DWORD dwLen = GetFileVersionInfoSize(exePath, &dwHandle);

	// Create a buffer of proper length
		byte* data = new byte[dwLen];

	// Get the file version info
		GetFileVersionInfo(exePath, dwHandle, dwLen, data);

	// Read the list of languages and code pages.
		struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
		} *lpTranslate;
		UINT cbTranslate;

		VerQueryValue(data, 
              TEXT("\\VarFileInfo\\Translation"),
              (LPVOID*)&lpTranslate,
              &cbTranslate);

	// Read the file description for each language and code page.
		char SubBlock[128];
		sprintf(SubBlock, "\\StringFileInfo\\%04x%04x\\%s", 
			lpTranslate[0].wLanguage,
            lpTranslate[0].wCodePage, verInfo);

	// Retrieve file description for language and code page 0. 
		char* lpBuffer;
		UINT dwBytes;
		VerQueryValue(data, 
						SubBlock, 
						(LPVOID*)&lpBuffer, 
						&dwBytes);

	// Copy to a string buffer
		strncpy(retBuff, lpBuffer, min(dwBytes, buffLen));

	// Clean up the data
		delete[] data;
}
///////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: Utility.cpp,v $
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//