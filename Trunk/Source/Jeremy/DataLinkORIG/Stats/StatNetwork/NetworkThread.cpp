#include "stdafx.h"
#include "..\StatCommon\FrameWork.XML.h"

#pragma region Info
//Test feeds:
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
										/*m_itemKey(NULL),*/
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
	return m_key_changed;
}

const wchar_t *NetworkThread::p_get_changed(void)
{
	AutoLock keyLock(&m_key_lock);
	if (!QEmpty())
		return m_KeyValueQueue.front().first.c_str();
	else
		return NULL;
}

const wchar_t *NetworkThread::p_get_value(const wchar_t key[])
{
	AutoLock valLock(&m_key_lock);
	if (m_KeyValueQueue.front().second.length() == 0)
		Sleep(1);
	return m_KeyValueQueue.front().second.c_str();
}

void NetworkThread::release_value(void)
{
}

void NetworkThread::release_changed_key(void)
{
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
	// If any elements are initialized, process them until all element sites are completely read and processed.
	// Iterate through each element and process 1 item for each while checking if thread should exit each loop.
	while (!m_thread_must_exit)
	{
		// Initialize the defined sites by opening internet connection handles.
		GetSites();

		// Get an Item for parsing from each site, create an XML tree, and process.
		GetItem();

		// Get one tag at a time from each item for each site.
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
void NetworkThread::QWrite()
{
	while (QSize() >= c_MaxQueueDepth)
		Sleep(10);
	
	AutoLock pushLock(&m_key_lock);
	m_KeyValueQueue.push(m_KeyValue);
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
			{
				ret = true;
#ifdef _VERBOSE_TESTING_
				printf("INITIALIZED: %ls.\n", m_vSiteElement[siteIndex].GetName());
#endif
			}
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
		// Parse an item
			if (m_vSiteElement[siteIndex].GetConnectedState())
				if (m_vSiteElement[siteIndex].ParseItem() == 0)
				{
					m_vSiteElement[siteIndex].CloseHandles();
					m_vSiteElement[siteIndex].SetTimeLastUpdated(::GetTickCount());
				}
	}

	ret = true;
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

				// Convert the tag # to text for use in the key name.
				wchar_t * tagNum = (wchar_t*)(_alloca(sizeof(wchar_t) * sizeof(tag) + 1));
				_itow(tag, tagNum, 10);

				// Create a child in order to get its type.
				m_childItem = m_treeItem.child(tag - 1);
				const wchar_t * tagName = m_childItem.type();
				
				// Create the key name. Example of how a key will look in LiveText:   SITE:[CNN]_item12_(link).
				//   where SITE is the network plugin prefix, CNN is the user-defined name for this site,
				//   item_1 is the iterated item in the feed, and link is the text of the individual item tags.

				// Set the text for the key.
				m_KeyValue.first = itemName;
				m_KeyValue.first += L":";
				m_KeyValue.first += numItemStr;
				m_KeyValue.first += L".";
				m_KeyValue.first += tagNum;
				m_KeyValue.first += L"[";
				m_KeyValue.first += tagName;
				m_KeyValue.first += L"]";

				// Set the text for the tag. (some times there is no data for a tag, so we null it out if that is the case)
				if (m_childItem.text())
					m_KeyValue.second = m_childItem.text();
				else
					m_KeyValue.second = L"";

				// Write key/value to queue.
				QWrite();
				SetEvent(m_key_changed);

#ifdef _VERBOSE_TESTING_
				wprintf(L"ITEM: %ls  tag: %d\n", m_KeyValue.first.c_str(), tag);
#endif	
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
	bool success = m_treeItem.read_from_string(theItem, m_vSiteElement[siteIndex].GetItem().size());

	delete [] theItem;

	return success;
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
	DWORD retCode;
	DWORD ret;

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

//		TCHAR achValue[MAX_VALUE_NAME];
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
					
					// Create instance with registry values and push back into vector.
					URLReader instanceReader(_achKey, _lSite, lBlockSize, lrefresh, lTimeout);
					m_vSiteElement.push_back(instanceReader);
				}
			}
		}
	}
	return ret;
}
#pragma endregion
