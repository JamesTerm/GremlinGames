#pragma once
#include "../../Controls/Registry.h"

//Returns size of the bytes copied... 
//returns zero for a case index which fails
//returns -1 when no more entries are available
inline size_t GetApplicationPath(size_t index,wchar_t *BufferToFill,size_t size)
{
	const size_t NoKeys=4;
	size_t BytesCopied=0;

	//present the release keys first
	if (index<NoKeys)
	{
		// Get it from the registry
		HKEY hKey = NULL;
		// Get the key values (mwatkins doesn't trust himself to remember to comment out #define __UseSpeedEditBeta__ the last minute before shipping)
			// This is better for the sanity of mwatkins ;-)
		const wchar_t *AppKey[]   = {SE1BETA_REG_KEY, SE1_REG_KEY, VT5_REG_KEY, TC_REG_KEY, VTD_REG_KEY };//{SPEEDEDIT_REG_KEY, VT5_REG_KEY, TC_REG_KEY, VTD_REG_KEY };
		const wchar_t *AppValue[] = {SE1BETA_VALUE_NAME, SE1_VALUE_NAME, VT5_VALUE_NAME, TC_VALUE_NAME, VTD_VALUE_NAME };//{SPEEDEDIT_VALUE_NAME, VT5_VALUE_NAME, TC_VALUE_NAME, VTD_VALUE_NAME };

		LONG lResult = RegOpenKey( HKEY_LOCAL_MACHINE , AppKey[index] , &hKey );

		if (lResult == ERROR_SUCCESS)
		{
			// Read the path value
			DWORD cbBuff = (DWORD)size;
			lResult = RegQueryValueEx(	hKey,		// handle to key
				AppValue[index],					// value name
				NULL,								// reserved
				NULL,								// type buffer
				(LPBYTE)BufferToFill,				// data buffer
				&cbBuff								// size of data buffer
				);

			if (lResult == ERROR_SUCCESS)
			{
				wcscat_s( BufferToFill,size,L"\\" );
				BytesCopied=wcslen(BufferToFill);
			}
			RegCloseKey(hKey);
		}
	}
	else
	{
		//now to present debug paths
		switch (index-NoKeys)
		{
		case 0:
			{
				DWORD VidRootCount=GetEnvironmentVariable(L"VTRoot",BufferToFill,(DWORD)size);
				if (VidRootCount!=0)
				{
					wcscat_s(BufferToFill,size,L"\\Debug25\\");
					BytesCopied=wcslen(BufferToFill);
				}
				break;
			}
		case 1:
			{
				DWORD VidRootCount=GetEnvironmentVariable(L"VTRoot",BufferToFill,(DWORD)size);
				if (VidRootCount!=0)
				{
					wcscat_s(BufferToFill,size,L"\\DebugSW\\");
					BytesCopied=wcslen(BufferToFill);
				}
				break;
			}
		default:
			BytesCopied=-1;
		}
	}

	return BytesCopied;
}

//To Use this you'll need 
//HMODULE RealDriverDLL <--- a global variable to use in the parameter
//GetFunctionPointers()  a callback to get your proc addresses
//For example:
//bool GetFunctionPointers()
//{
//	TedDriverProc=(DriverProc_t) GetProcAddress(TedDriverDLL,"DriverProc");
//	return TedDriverProc!=NULL;
//}
//Return true if you successfully got all your proc addresses

inline bool FindAndLoadRealDLL(HMODULE &RealDriverDLL,wchar_t *RealDllName,bool (*GetFunctionPointers)(void) )
{
	bool Success=false;
	//for now we'll just go down the list in order, but we may wish to sort according to file date

	wchar_t ApplicationPath[MAX_PATH*2];
	size_t index=0,AppPathSizeReturn;
	while ((AppPathSizeReturn=GetApplicationPath(index++,ApplicationPath,MAX_PATH*2)) != -1)
	{
		if (AppPathSizeReturn==0) continue;  //some paths may not exist
		//Set current working directory
		SetCurrentDirectory(ApplicationPath);
		wcscat_s(ApplicationPath,RealDllName);
		RealDriverDLL=LoadLibrary(ApplicationPath);
		if (RealDriverDLL)
		{
			Success=GetFunctionPointers();
			if (!Success) 
				FreeLibrary(RealDriverDLL);
			else
			{
				const size_t BufferSize=MAX_PATH*2;
				wchar_t TempBuf[BufferSize];
				swprintf_s(TempBuf,BufferSize,L"* * * Using %s \n",ApplicationPath);
				OutputDebugString(TempBuf);
				break; //success, we'll use this one
			}
		}
	}

	//Dump a courtesy to output to show we couldn't find the dll
	if (!Success)
	{
		const size_t BufferSize=MAX_PATH*2;
		wchar_t TempBuf[BufferSize];
		swprintf_s(TempBuf,BufferSize,L"* * * Unable to find %s \n",RealDllName);
		OutputDebugString(TempBuf);
	}

	return Success;
}
