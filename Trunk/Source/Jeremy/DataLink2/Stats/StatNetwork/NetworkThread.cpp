#include "stdafx.h"
#include "..\StatCommon\FrameWork.XML.h"

#pragma region Info
//Here are some test feeds:
// ArsTechnica:		http://feeds.arstechnica.com/arstechnica/everything/
// CNN:				http://rss.cnn.com/rss/cnn_topstories.rss/
// MySA:			http://www.mysanantonio.com/rss?c=y&path=%2Fabout_us%2Fexpress-news
// Science Daily:	http://feeds.sciencedaily.com/sciencedaily/
// HS Football:		http://www.woai.com/rss/643.rss
// Fox Sports:		http://feeds.pheedo.com/feedout/syndicatedContent_categoryId_0/
#pragma endregion

#pragma region NetworkThread ctor/dtor
//	________________________________________________________________________
// O)_______________________________________________________________________)
// \								NetworkThread							\
//	\________________________________________________________________________\
//	O)________________________________________________________________________)
NetworkThread::NetworkThread(void) :	m_thread_must_exit(false),
										m_numSites(0),
										m_key_changed(::CreateEvent(NULL, FALSE, FALSE, NULL))
{
	// Start the thread
	m_thread_handle = ::CreateThread(NULL, 64*1024, (LPTHREAD_START_ROUTINE)NetworkThreadProc, (void*)this, 0, NULL);
}

NetworkThread::~NetworkThread()
{
	// Signal the thread to exit
	m_thread_must_exit = true;

	// Wait for the thread to exit
	if (m_thread_handle) 
	{
		::WaitForSingleObject(m_thread_handle, 2000);
		::CloseHandle(m_thread_handle);
	}

	// CLose the handle
	::CloseHandle(m_key_changed);
}
#pragma endregion

#pragma region Exported Functions
//	________________________________________
// O)_______________________________________)
// \			Exported Functions			\
//	\________________________________________\
//	O)________________________________________)
HANDLE NetworkThread::get_handle(void) const
{
	// Return handle.
	return m_key_changed;
}

const wchar_t *NetworkThread::p_get_changed(void)
{
	// Return changed key from queue.
	AutoLock keyLock(&m_key_lock);
	if (!QEmpty())
		return m_KeyValueQueue.front().first.c_str();
	else
		return NULL;
}

const wchar_t *NetworkThread::p_get_value(const wchar_t key[])
{
	// Return key value from queue.
	AutoLock valLock(&m_key_lock);
	if (!QEmpty())
		return m_KeyValueQueue.front().second.c_str();
	else
		return NULL;
}

void NetworkThread::release_value(void)
{
}

void NetworkThread::release_changed_key(void)
{
	// Release the key.
	AutoLock popLock(&m_key_lock);
	m_KeyValueQueue.pop();
}
#pragma endregion

#pragma region Main Thread Runner
//	________________________________________
// O)_______________________________________)
// \			Main Thread Runner			\
//	\________________________________________\
//	O)________________________________________)
void NetworkThread::NetworkThreadProc(void *p_this)
{
	// Get the pointer
	NetworkThread* ntp=(NetworkThread*)p_this;

	// Call It
	ntp->NetworkThreadRunner();
}

void NetworkThread::NetworkThreadRunner(void)
{	
	// Read the registry and create the vector of sites from values
	// If there are no site definitions, the thread has no work to do and exits.
	m_numSites = RegEnumSites(cwsz_sitesPath);
	
	if (m_numSites == 0)
		m_thread_must_exit = true;

	// MAIN LOOP:
	//  If any elements are initialized, process them until all element sites are completely read and processed.
	//  Iterate through each element and process 1 item for each while checking if thread should exit each loop.
	while (!m_thread_must_exit)
	{
		// Initialize the defined sites by opening internet connection handles.
		GetSites();

		// Get an Item for parsing from each site, create an XML tree, and process.
		GetItem();

		// Set up one tag at a time from each item for each site.
		GetTag();
	}
}
#pragma endregion

#pragma region Queue Methods
//	________________________________________
// O)_______________________________________)
// \			  Queue methods				\
//	\________________________________________\
//	O)________________________________________)
bool NetworkThread::QWrite()
{
	// Write the key/value pair to the processing queue to be picked-up.
	while (QSize() >= c_MaxQueueDepth)
		Sleep(10);
	
	AutoLock pushLock(&m_key_lock);
	m_KeyValueQueue.push(m_KeyValue);

	return true;
}
#pragma endregion

#pragma region Item, Tag, and XMLTree creator
//	________________________________________
// O)_______________________________________)
// \	 Item, Tag, and XMLTree creator		\
//	\________________________________________\
//	O)________________________________________)
bool NetworkThread::GetSites()
{
	bool ret = false;

	// INITIALIZE: Try to initialize the Internet connection and website for any site elements found.
	for (DWORD siteIndex = 0; siteIndex < m_numSites && !m_thread_must_exit; siteIndex++)
	{
		// If iterated site is disconnected and has made it past it's refresh time, initialize.
		if ((m_vSiteElement[siteIndex].GetConnectedState() == false) &&
			((::GetTickCount() - m_vSiteElement[siteIndex].GetTimeLastUpdated()) >= (m_vSiteElement[siteIndex].GetRefreshTime() * 1000) ||
			m_vSiteElement[siteIndex].GetTimeLastUpdated() == 0))
		{
			if (m_vSiteElement[siteIndex].Initialize())
				ret = true;
		}
	}
	// If any successfully initialize, return true.
	return ret;
}

bool NetworkThread::GetItem()
{
	bool ret = false;

	// Parse an item for each connected site.
	for (DWORD siteIndex = 0; (siteIndex < m_numSites && !m_thread_must_exit); siteIndex++)
	{
		// Check to see if a site is connected or will connect. If no, bail and go to next in loop.
		if (m_vSiteElement[siteIndex].GetConnectedState())
		{
			ret = true;
			// Parse an item - if nothing read, close this connection and set time for refresh rate.
			if (m_vSiteElement[siteIndex].ParseItem() == 0)
			{
				m_vSiteElement[siteIndex].CloseHandles();
				m_vSiteElement[siteIndex].SetTimeLastUpdated(::GetTickCount());
			}
		}
	}

	return ret;
}


bool NetworkThread::GetTag()
{
	// Iterate through all site instances and get an item for each.
	for (DWORD siteIndex = 0; siteIndex < m_numSites; siteIndex++)
	{
		if (m_vSiteElement[siteIndex].GetConnectedState())
		{
			// Create an XML tree.
			CreateItemXMLTree(siteIndex);

			// Get components of key name.
			const std::wstring itemName = m_vSiteElement[siteIndex].GetName();
			DWORD numItem = m_vSiteElement[siteIndex].GetItemNum();
			wchar_t * numItemStr = (wchar_t*)(_alloca(sizeof(wchar_t) * sizeof(numItem) + 1));
			_itow(numItem, numItemStr, 10);

			// Iterate through the # of tags for an item.
			for (int tag = 1; tag <= (m_treeItem.no_children()); tag++)
			{
				// Reset values.
				m_KeyValue.second = L"";
				m_KeyValue.first = L"";

				// Get the child tag name.
				const wchar_t * tagName = m_treeItem.child(tag - 1).type();
				
				// Create the key name. Example of how a key will look in LiveText:   CNN:3.4(description). NEW:  CNN:3[description]
				//   where CNN is the user-defined name for this site, 3 is the item number, and 4 is the 
				//   item's tag number. The word in parenthesis is the name of the item's tag.

				// Set the text for the key.
				m_KeyValue.first = itemName;
				m_KeyValue.first += L":";
				m_KeyValue.first += numItemStr;
				m_KeyValue.first += L"[";
				m_KeyValue.first += tagName;
				m_KeyValue.first += L"]";

				if ((!wcscmp(tagName, L"title") == 0) && (!wcscmp(tagName, L"link") == 0) && (!wcscmp(tagName, L"description") == 0))
				{
					// Convert the tag # to text for use in the key name.
					wchar_t * tagNum = (wchar_t*)(_alloca(sizeof(wchar_t) * sizeof(tag) + 1));
					_itow(tag, tagNum, 10);					
					
					m_KeyValue.first += L".";
					m_KeyValue.first += tagNum;
				}

				// Set the text for the tag or set to null if none.
				if (m_treeItem.child(tag - 1).text())
					m_KeyValue.second = m_treeItem.child(tag - 1).text();
				else
					m_KeyValue.second = L"";

				// Corrects value (HTML that was embedded in the XML), writes key/value pair to queue and process edited tag.
				GetTag_CleanBrackets();
				GetTag_FixAmps();
				GetTag_Process();
			}
		}
	}
	return true;
}

bool NetworkThread::CreateItemXMLTree(DWORD siteIndex)
{
	std::wstring::size_type i=0;

	// Convert from std::string to wchar_t *
 	std::wstring tempString = m_vSiteElement[siteIndex].GetItem();
	wchar_t * theItem = new wchar_t[tempString.size() + 1];
 	for(i = 0; i<=tempString.size(); i++)
 		theItem[i]=tempString[i];

	// Create a tree.
	bool ret = m_treeItem.read_from_string(theItem, m_vSiteElement[siteIndex].GetItem().size());

	delete [] theItem;

	return ret;
}

bool NetworkThread::GetTag_CleanBrackets()
{
	DWORD openBracketPos = 0;
	DWORD closeBracketPos = 0;
	DWORD crlfPos = 0;
	DWORD crPos = crlfPos;
	DWORD lfPos = crlfPos;
	bool moreBrackets = true;
	bool moreCRLF = true;
	bool ret = true;

	// Go through the value to remove all unnecessary whitespace and all angle bracketed HTML tags.
	while (moreBrackets)
	{
		openBracketPos = m_KeyValue.second.find(L"<", 0);
		closeBracketPos = m_KeyValue.second.find(L">", 0);

		if (openBracketPos != std::wstring::npos && closeBracketPos != std::wstring::npos)
			m_KeyValue.second.erase(openBracketPos, closeBracketPos - openBracketPos + 1);
		else
			moreBrackets = false;
	}

	// Delete all whitespace before the beginning of the remaining real text.
	EraseWhitespace(0);

	// Get rid of extra CR/LF.
	while (moreCRLF)
	{
		crPos = lfPos = crlfPos;

		// If we find a cr or lf, nuke all of the white space (including the one we found) and replace all with 2 new lines.
		if ((crPos=m_KeyValue.second.find(L"\r", crlfPos), crPos!=std::wstring::npos) || (lfPos=m_KeyValue.second.find(L"\n", crlfPos), lfPos!=std::wstring::npos))
		{	
			(crPos < lfPos) ? (crlfPos = crPos) : (crlfPos = lfPos);
			EraseWhitespace(crlfPos);
			m_KeyValue.second.insert(crlfPos, L"\n\n");
			crlfPos+=2;
		}
		else
		{
			// Erase the last 2 whitespace chars after the last of the real text to avoid wasting space in LiveText
			if (crlfPos)
				EraseWhitespace(crlfPos - 2);

			// Bail
			moreCRLF = false;
		}
	}
	return ret;
}

bool NetworkThread::EraseWhitespace(DWORD nPos)
{
	bool ret = false;
	size_t spaceCount = 0;
	WhiteSpace typeWS = eNONE;
	
	// Erases whitespace at nPos until there are none. Returns true when done.
	//  Tracks spaces so that we don't accidentally remove a space that actually needs to be there.
	while (!ret)
	{
		typeWS = IsWhitespace(nPos);
		
		switch (typeWS)
		{
		case eNONE:
			ret = true;
			if (spaceCount && nPos)
				m_KeyValue.second.insert(nPos, L" ", 1);
			break;
		case eSPACE:
		case eTAB:
			spaceCount++;
		case eCARRIAGE_RETURN:
		case eLINE_FEED:
			m_KeyValue.second.erase(nPos, 1);
			break;
		}
	}

	return ret;
}

WhiteSpace NetworkThread::IsWhitespace(DWORD nPos)
{	
	// Detects whitespace character and returns the kind.
	WhiteSpace ret = eNONE;

	if (m_KeyValue.second.compare(nPos, 1, L" ", 0, 1) == 0)
		ret = eSPACE;
	else if (m_KeyValue.second.compare(nPos, 1, L"\r", 0, 1) == 0)
		ret = eCARRIAGE_RETURN;
	else if (m_KeyValue.second.compare(nPos, 1, L"\n", 0, 1) == 0)
		ret = eLINE_FEED;
	else if (m_KeyValue.second.compare(nPos, 1, L"\t", 0, 1) == 0)
		ret = eTAB;

	return ret;
}

bool NetworkThread::GetTag_FixAmps()
{
	// Fix the tag by removing unwanted embedded HTML or replacing supported embedded HTML 
	//   ampersand tags with equivalent characters - dashes, apostrophes, etc.

	bool ret = true;
	DWORD startPos = 0;
	DWORD endPos = 0;

	// First pass, change all "&amp;" to "&".
	while (startPos = m_KeyValue.second.find(L"&amp;", endPos), startPos != std::wstring::npos)
	{
		m_KeyValue.second.replace(startPos, 5, L"&", 1);
		endPos = startPos;
		Sleep(1);
	}

	// Second pass, find and replace all "&" HTML codes (that we support) with actual or usable values.
	startPos = endPos = 0;
	while (startPos = m_KeyValue.second.find(L"&", endPos), startPos != std::wstring::npos)
	{
		// If we find a semicolon that marks the end of a code, process it.
		if ((endPos = m_KeyValue.second.find(L";", startPos), endPos != std::wstring::npos) && (endPos - startPos < 10))
		{
			// Iterate through all of the HTML codes and check to see if we have a match - if so, replace.
			for (size_t i=0; i<COUNTOF(HTMLCode); i++)
			{
				// Found match?
				if (m_KeyValue.second.compare(startPos, endPos - startPos + 1, HTMLCode[i].alphaCode, 0, wcslen(HTMLCode[i].alphaCode)) == 0 ||
					m_KeyValue.second.compare(startPos, endPos - startPos + 1, HTMLCode[i].numberCode, 0, wcslen(HTMLCode[i].numberCode)) == 0)
				{
					// Replace code.
					m_KeyValue.second.replace(startPos, endPos - startPos + 1, HTMLCode[i].replaceCode, wcslen(HTMLCode[i].replaceCode));
				}
				// Continue out even if we didn't replace anything (due to valid literal "&").
				continue;
			}
		}
		else
		{
			// The ampersand was just an ampersand with no code - ignore and continue.
			endPos = startPos + 1;
		}
	}
	return ret;
}

bool NetworkThread::GetTag_Process()
{
	bool ret = false;

	// Write key/value to queue and trigger update.
	ret = QWrite();
	if (ret)
		SetEvent(m_key_changed);

	return ret;
}
#pragma endregion

#pragma region Registry Methods
//    ______________________________________
//	 O)_____________________________________)
//	 \			  Registry methods			\
//	  \______________________________________\
//	  O)______________________________________)
const wchar_t * NetworkThread::RegGetSiteText(const wchar_t * path, const wchar_t * val, const wchar_t * retVal)
{

	HKEY hKey;
	DWORD retCode = 0;
	TCHAR value[16383] = L"\0";
	DWORD BufferSize = 16383;

	// Open Key and get handle.
	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER, path, NULL, KEY_ALL_ACCESS, &hKey));

	// If open was successful, get the value.
	if (retCode == ERROR_SUCCESS)
		retCode = (::RegQueryValueEx(hKey, val, NULL, NULL, (LPBYTE)&value, &BufferSize));

	retVal=value;
	RegCloseKey(hKey);
	return retVal;
}

const DWORD NetworkThread::RegGetSiteNum(const wchar_t * path, const wchar_t * val)
{

	HKEY hKey;
	DWORD retCode=0;
	DWORD value=0;
	DWORD BufferSize = 256;

	// Open Key and get handle.
	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER, path, NULL, KEY_ALL_ACCESS, &hKey));

	// If open was successful, get the value.
	if (retCode == ERROR_SUCCESS)
		retCode = (::RegQueryValueEx(hKey, val, NULL, NULL, (LPBYTE)&value, &BufferSize));

	RegCloseKey(hKey);
	return value;
}

// Creates a vector of sites objects.
// Returns the number of sites found defined in the registry.
DWORD NetworkThread::RegEnumSites(const wchar_t * path)
{
	HKEY hKey;
	DWORD retCode = 0;
	DWORD ret = 0;

	retCode=::RegOpenKeyEx(	HKEY_CURRENT_USER,
							path,
							0,
							KEY_READ,
							&hKey);
	if (retCode == ERROR_SUCCESS)
	{
		TCHAR		achKey[255];					// buffer for subkey name
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

		DWORD cchValue = 16383;

		// Get the class name and the value count. 
		retCode = ::RegQueryInfoKey(hKey,                    // key handle
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

		ret = cSubKeys;
		if (cSubKeys)
		{
			for (size_t i=0; i<cSubKeys; i++)
			{ 
				cbName = 255;
				retCode = ::RegEnumKeyEx(	hKey, i,
											achKey,
											&cbName,
											NULL,
											NULL,
											NULL,
											&ftLastWriteTime);
				if (retCode == ERROR_SUCCESS)
				{
					const TCHAR value[16383] = L"\0";
					const wchar_t * lSite = L"\0";
					wchar_t * _lSite = L"\0";
					wchar_t * _achKey = L"\0";
					wchar_t * sitePath=0;
					DWORD lTimeout=0;
					DWORD lrefresh=0;
					DWORD lBlockSize=0;
					DWORD BufSize=(wcslen(path) + wcslen(achKey) + 3);

					// Enumerate through each registry entry for sites, get the values, and push back to instance vector.
					sitePath=(wchar_t*)(_alloca(sizeof(wchar_t)*(BufSize)));
					wcscpy(sitePath, path);
					wcscat(sitePath, L"\\");
					wcscat(sitePath, achKey);
					
					_achKey = (wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(achKey)+1)));
					wcscpy(_achKey, achKey);
					lSite = RegGetSiteText(sitePath, L"Address", lSite);
					_lSite = (wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(lSite)+1)));
					wcscpy(_lSite, lSite);
					lrefresh = RegGetSiteNum(sitePath, L"Refresh");
					lBlockSize = RegGetSiteNum(sitePath, L"Blocksize");
					lTimeout = RegGetSiteNum(sitePath, L"Timeout");
					
					// Create instance with registry values and push back to vector.
					URLReader instanceReader(_achKey, _lSite, lBlockSize, lrefresh, lTimeout);
					m_vSiteElement.push_back(instanceReader);
				}
			}
		}
	}
	return ret;
}
#pragma endregion
