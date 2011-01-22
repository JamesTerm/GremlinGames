// DataLinkUI.cpp : main project file.

#include "stdafx.h"
#include "DataLinkForm.h"

using namespace DataLinkUI;
  /////////////////////////////////////////////////////
 //						Registrar					//
/////////////////////////////////////////////////////

// Class for interfacing with the registry.
Registrar::Registrar(void)
{
}

Registrar::~Registrar(void)
{
}

// Read from the HKCU in the registry.
const wchar_t * const Registrar::RegRead(	const wchar_t regPath[MAX_KEY_LENGTH],
											const wchar_t subType[MAX_VALUE_NAME],
											const DWORD valType,
											const wchar_t valName[MAX_VALUE_NAME],
											const wchar_t * retVal)	// Returns retVal as result.
{
	HKEY hKey;
	DWORD retCode = 0;
	TCHAR value[MAX_VALUE_NAME] = L"\0";
	DWORD BufferSize = MAX_VALUE_NAME;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_ALL_ACCESS,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{

		retCode=(::RegGetValue(	hKey,
								subType,
								valName,
								RRF_RT_ANY,
								NULL,
								LPBYTE(&value),
								&BufferSize));

	}
// 	else
//  		assert(false);

	retVal=value;
	RegCloseKey(hKey);
	return retVal;
}

// Reads a HKLM value from the registry. Windows 7 (and Vista) restrict writing to this area of the registry under normal means.
const wchar_t * const Registrar::RegRead2(	const wchar_t regPath[MAX_KEY_LENGTH],
											const wchar_t subType[MAX_VALUE_NAME],
											const DWORD valType,
											const wchar_t valName[MAX_VALUE_NAME],
											const wchar_t * retVal)
{
	HKEY hKey;
	DWORD retCode = 0;
	TCHAR value[MAX_VALUE_NAME] = L"\0";
	DWORD BufferSize = MAX_VALUE_NAME;

	retCode=(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							regPath,
							NULL,
							KEY_READ,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{

		retCode=(::RegGetValue(	hKey,
								subType,
								valName,
								RRF_RT_ANY,
								NULL,
								LPBYTE(&value),
								&BufferSize));

	}

	retVal=value;
	RegCloseKey(hKey);
	return retVal;
}

DWORD Registrar::RegReadNum(	const wchar_t regPath[MAX_KEY_LENGTH],
								const wchar_t valName[MAX_VALUE_NAME])
{
	HKEY hKey;
	DWORD retCode = 0;
	DWORD value = 0;
	DWORD BufferSize = 64;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_ALL_ACCESS,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{

		retCode=(::RegGetValue(	hKey,
								NULL,
								valName,
								RRF_RT_ANY,
								NULL,
								LPBYTE(&value),
								&BufferSize));

	}

	RegCloseKey(hKey);
	return value;
}

// Writes the currently selected site to registry.
bool Registrar::RegWriteNum(const wchar_t* path,
							const wchar_t* name,
							const DWORD value)
{
	HKEY hHKCU = NULL, hMyKey = NULL;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, NULL, 0, KEY_WRITE, &hHKCU) == ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(hHKCU, path, 0, NULL, 0, KEY_WRITE, NULL, &hMyKey, NULL) == ERROR_SUCCESS)
		{
			RegSetValueEx(hMyKey, name, 0, REG_DWORD, (BYTE*)&value, sizeof(DWORD));
			RegCloseKey(hMyKey);
		}
	}
	Sleep(1);
	return true;
}

bool Registrar::RegReadBool(const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME])
{
	HKEY hKey;
	DWORD retCode = 0;
	bool value = false;
	DWORD BufferSize = 8;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_ALL_ACCESS,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{

		retCode=(::RegGetValue(	hKey,
								NULL,
								valName,
								RRF_RT_ANY,
								NULL,
								LPBYTE(&value),
								&BufferSize));
		
	}

	RegCloseKey(hKey);
	return value;
}

// Writes to a HKCU value in the registry.
bool Registrar::RegWrite(const wchar_t regPath[MAX_KEY_LENGTH],
						 const DWORD valType,
						 const wchar_t valName[MAX_VALUE_NAME],
						 const wchar_t valValue[MAX_VALUE_NAME])
{
	bool ret = false;
	HKEY hKey;
	DWORD retCode = 0;
	DWORD exists = 0;
	DWORD size = (_tcslen(valValue) + 1) * sizeof(TCHAR);

	retCode = (::RegCreateKeyEx(HKEY_CURRENT_USER,
								regPath,
								NULL, NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey,
								&exists));
	if ((retCode == ERROR_SUCCESS) && (exists == REG_CREATED_NEW_KEY) || (exists == REG_OPENED_EXISTING_KEY))
	{
		//Entry was either successfully created or already exists. Now will try to create/update key value and data.
		retCode = (::RegSetValueEx(	hKey,
									valName,
									NULL,
									valType,
									(BYTE*)valValue,
									size));
		ret=true;
	}

	RegCloseKey(hKey);
	return ret;
}

// Writes a bool value - used for flags.
bool Registrar::RegWriteBool(	const wchar_t regPath[MAX_KEY_LENGTH],
								const wchar_t valName[MAX_VALUE_NAME],
								const int valValue)
{
	bool ret = false;
	HKEY hKey;
	DWORD retCode = 0;
	DWORD exists = 0;
	DWORD size = 4;
	
	retCode = (::RegCreateKeyEx(HKEY_CURRENT_USER,
								regPath,
								NULL, NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey,
								&exists));
	if ((retCode == ERROR_SUCCESS) && (exists == REG_CREATED_NEW_KEY) || (exists == REG_OPENED_EXISTING_KEY))
	{
		//Entry was either successfully created or already exists. Now will try to create/update key value and data.
		retCode = (::RegSetValueEx(	hKey,
									valName,
									NULL,
									REG_DWORD,
									(LPBYTE)&valValue,
									size));
		ret=true;
	}

	RegCloseKey(hKey);
	return ret;
}

// Delete a registry entry.
bool Registrar::RegDelete(	const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME])
{
	// Delete the settings entry.
	bool ret = false;
	HKEY hKey;
	DWORD retCode = 0;
	
	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_ALL_ACCESS,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode = (::RegDeleteKey(hKey, valName));
		if (retCode == ERROR_SUCCESS)
			ret = true;
	}
	return ret;
}

// Delete a tree in the registry.
bool Registrar::RegDeleteATree(const wchar_t regPath[MAX_KEY_LENGTH], const wchar_t valName[MAX_VALUE_NAME])
{
	bool ret = false;
	HKEY hKey;
	DWORD retCode = 0;
	
	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_ALL_ACCESS,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode=(::RegDeleteTree(	hKey,
									valName));
		if (retCode == ERROR_SUCCESS)
			ret = true;
	}
	return ret;
};

// Delete a registry entry with sub-keys. Enumerates through and deletes the subkeys only.
bool Registrar::RegDeleteSubs(	const wchar_t regPath[MAX_KEY_LENGTH])
{
	HKEY hKey;
	DWORD retCode;

	retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
							regPath,
							0,
							KEY_READ,
							&hKey);
	if (retCode == ERROR_SUCCESS)
	{
		TCHAR		achKey[MAX_KEY_LENGTH];			// buffer for subkey name
		DWORD		cbName;							// size of name string 
		TCHAR		achClass[MAX_PATH] = TEXT("");	// buffer for class name 
		DWORD		cchClassName = MAX_PATH;		// size of class string 
		DWORD		cSubKeys=0;						// number of subkeys 
		DWORD		cbMaxSubKey;					// longest subkey size 
		DWORD		cchMaxClass;					// longest class string 
		DWORD		cValues;						// number of values for key 
		DWORD		cchMaxValue;					// longest value name 
		DWORD		cbMaxValueData;					// longest value data 
		DWORD		cbSecurityDescriptor;			// size of security descriptor 
		FILETIME	ftLastWriteTime;				// last write time 

// 		TCHAR achValue[MAX_VALUE_NAME]; 
		DWORD cchValue = MAX_VALUE_NAME; 

		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			hKey,                    // key handle 
			achClass,                // buffer for class name 
			&cchClassName,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			&cbMaxSubKey,            // longest subkey size 
			&cchMaxClass,            // longest class string 
			&cValues,                // number of values for this key 
			&cchMaxValue,            // longest value name 
			&cbMaxValueData,         // longest value data 
			&cbSecurityDescriptor,   // security descriptor 
			&ftLastWriteTime);       // last write time 

		if (cSubKeys)
		{
			for (size_t i=0; i<cSubKeys; i++) 
			{ 
				cbName = MAX_KEY_LENGTH;
				retCode = RegEnumKeyEx(	hKey, 0,
										achKey, 
										&cbName, 
										NULL, 
										NULL, 
										NULL, 
										&ftLastWriteTime); 
				if (retCode == ERROR_SUCCESS) 
				{
					MyReg.RegDelete(regPath, achKey);
				}
			}
		}
	}
	return true;
}


  /////////////////////////////////////////////////////
 //					Main Application				//
/////////////////////////////////////////////////////
[STAThreadAttribute]
int main(array<System::String ^> ^args)
{	
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew DataLinkForm());
	return 0;
}
