#include "stdafx.h"
#pragma comment (lib, "wininet")

#pragma region URLReader ctor/dtor
//	________________________________________________________________________
// O)_______________________________________________________________________)
// \								URLReader								\
//	\________________________________________________________________________\
//	O)________________________________________________________________________)
URLReader::URLReader(const wchar_t * name,
					 const wchar_t * site,
					 DWORD blockSize,
					 DWORD refresh,
					 DWORD timeout)
 :	m_name(name),
	m_feedURL(site),
	m_blockSize(blockSize),
	m_refresh(refresh),
	m_timeout(timeout),
	m_hInternet(NULL),
	m_hOpenUrl(NULL),
	m_strFeed(L"\0"),
	m_filePosition(0),
	m_strFeedPartial(L"\0"),
	m_isConnected(false),
	m_timeLastUpdated(0),
	m_itemNum(0),
	m_codePage(0)
{
}

// Destructor.
URLReader::~URLReader(void)
{
	//CloseHandles();
	Sleep(1);
}
#pragma endregion

#pragma region Initialize / close functions
//	________________________________________________________________________
// O)_______________________________________________________________________)
// \						Initialize / close functions					\
//	\________________________________________________________________________\
//	O)________________________________________________________________________)
// Initialize by checking for internet connection and creating handles.
bool URLReader::Initialize(void)
{
	DWORD result = 0;
	bool ret = false;

	// Check Internet connectivity.
	result = InternetAttemptConnect(NULL);

	// Open Internet handle for connection.
	if (result == 0)
	{
		m_hInternet = InternetOpen(NULL, LOCAL_INTERNET_ACCESS, NULL, NULL, 0);
		result = GetLastError();
		// Some other options...
		//	m_hInternet = InternetOpen(L"Some Text", INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0);
		//	result = GetLastError();
		//	m_hInternet = InternetOpen(NULL, LOCAL_INTERNET_ACCESS, NULL, 0, 0);
		//	result = GetLastError();
		//	m_hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, 0, 0);
		//	result = GetLastError();
		//	m_hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, 0, 0);
		//	result = GetLastError();
 		//	m_hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG_WITH_NO_AUTOPROXY, NULL, 0, 0);
		//	result = GetLastError();
 		//	m_hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PROXY, NULL, 0, 0);
		//	result = GetLastError();
	}

	// Open URL handle.
	if (result == 0)
	{
		m_hOpenUrl = InternetOpenUrl(	m_hInternet, m_feedURL, NULL, 0,
										INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0);
		result += GetLastError();
	}

	// If connection and opens were successful, set timeout, set connection state, and set ret to true.
	if (result == 0)
	{
		ret = InternetSetOption(m_hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &m_timeout, sizeof(DWORD));
		SetConnectedState(true);
	}

	assert(ret);
	return ret;
}

// Close handles.
bool URLReader::CloseHandles(void)
{
	int result = 0;
	bool ret = false;
	DWORD loopCount = 0;
	DWORD loopLimit = 200;

	// Close handles in reverse order.
	do 
	{
		// If handle didn't return close (almost never does), try again with delay.
		result = InternetCloseHandle(m_hOpenUrl);
		loopCount++;
		Sleep(10 + (2 * loopCount));
	} while (result && loopCount < loopLimit);

	// First handle closed successfully, then...
	if (loopCount < loopLimit)
	{
		loopCount = 0;
		do 
		{
			// If handle didn't return close (almost never does), try again with delay.
			result = InternetCloseHandle(m_hInternet);
			loopCount++;
			Sleep(10 + (2 * loopCount));
		} while (result && loopCount < loopLimit);

		// Both handles closed successfully. Set connected state flag to false and return.
		if (loopCount < loopLimit)
		{
			ret = true;
			SetConnectedState(false);
		}
	}

	return ret;
}
#pragma endregion

#pragma region Parsing functions
//	________________________________________________________________________
// O)_______________________________________________________________________)
// \							Parsing functions							\
//	\________________________________________________________________________\
//	O)________________________________________________________________________)
// Get a block of the feed - returns bytes read.
DWORD URLReader::GetBlock()
{
	DWORD bytesRead = 0;
	char * feedBlock = (char *)_alloca(m_blockSize + 1);
	wchar_t * tempBlock = L"\0";

	// Read a block of the RSS feed, zero term, and add to m_strFeed.
	InternetReadFile(m_hOpenUrl, feedBlock, m_blockSize, &bytesRead);
	feedBlock[bytesRead] = '\0';

	// If we don't yet have the codepage, get the encoding from the XML.
	if (!m_codePage)
	{
		wchar_t * encBlock = new wchar_t[strlen(feedBlock) + 1];
		for(size_t i = 0; i<=wcslen(encBlock); i++)
			encBlock[i]=feedBlock[i];
		std::wstring encString = encBlock;
		delete [] encBlock;

		int cStart = encString.find(L"encoding=\"");
		if (cStart != std::wstring::npos)
		{
			cStart+=10;
			int cEnd = encString.find(L"\"?>");
			
			// Go with UTF-8 unless UTF-7 specified.
			std::wstring tempCP (encString, cStart, cEnd - cStart);
			if (wcsicmp(tempCP.c_str(), L"UTF-7") == 0)
				m_codePage = CP_UTF7;
			else
				m_codePage = CP_UTF8;
		}
		else
			m_codePage = CP_UTF8;
	}
	else
		m_codePage = CP_UTF8;

	// Adjustments based upon codepage used
	switch (m_codePage)
	{
		case CP_UTF7:
		case CP_UTF8:
			// Adjust multi-byte to wide.
			tempBlock = (wchar_t *)_alloca((m_blockSize + 1) * sizeof(wchar_t));
			MultiByteToWideChar(m_codePage, 0, feedBlock, -1, tempBlock, bytesRead + 1);

			m_strFeed += tempBlock;
			break;

		default:
			tempBlock = new wchar_t[strlen(feedBlock) + 1];
			for(size_t i = 0; i<=wcslen(tempBlock); i++)
				tempBlock[i]=feedBlock[i];

			m_strFeed += tempBlock;

			delete [] tempBlock;
	}

	// Return bytes read.
	return bytesRead;
}

// Creates an item from the feed in std::string m_Feed.
DWORD URLReader::ParseItem()
{
 	DWORD dwBytesRead = 0;
	DWORD ret = 0;

	// Clear to start on the next item...
	m_filePosition = 0;
	m_strFeed = m_strFeedPartial;
	m_strFeedPartial = L"";

	// Read a block.
	dwBytesRead = GetBlock();
	
	// Markers for the begin and ending of RSS items.
	int itemStart_begin = -1;
	int itemStart_end = -1;
	int itemStop_begin = -1;
	int itemStop_end = -1;

	// Initial search for beginning of an item.
	itemStart_begin = m_strFeed.find(L"<item", m_filePosition);

	// Process if we have something.
	if ((dwBytesRead == 0 && m_strFeed.length() != 0 && itemStart_begin != -1) || (dwBytesRead != 0))
	{
		// Find beginning of an item start tag - if none found, get another block.
		while (itemStart_begin == -1 && (dwBytesRead != 0 || m_strFeed.length() != 0))
		{
			dwBytesRead = GetBlock();
			itemStart_begin = m_strFeed.find(L"<item", m_filePosition);
		}
		if (dwBytesRead != 0 || m_strFeed.length() != 0)
		{
			m_filePosition = (itemStart_begin + 5);
			itemStart_end = m_strFeed.find(L">", m_filePosition);
		}

		// Find end of item start tag.
		while (itemStart_end == -1 && (dwBytesRead != 0 || m_strFeed.length() != 0))
		{
			dwBytesRead = GetBlock();
			itemStart_end = m_strFeed.find(L">", m_filePosition);
		}
		if (dwBytesRead != 0 || m_strFeed.length() != 0)
		{
			m_filePosition = (itemStart_end + 1);
			itemStop_begin = m_strFeed.find(L"</item", m_filePosition);
		}

		// Find beginning of end tag.
		while (itemStop_begin == -1 && (dwBytesRead != 0 || m_strFeed.length() != 0))
		{
			dwBytesRead = GetBlock();
			itemStop_begin = m_strFeed.find(L"</item", m_filePosition);
		}
		if (dwBytesRead != 0 || m_strFeed.length() != 0)
		{
			m_filePosition = (itemStop_begin + 6);
			itemStop_end = m_strFeed.find(L">", m_filePosition);
		}

		// Find end of item end tag.
		while (itemStop_end == -1 && (dwBytesRead != 0 || m_strFeed.length() != 0))
		{
			dwBytesRead = GetBlock();
			itemStop_end = m_strFeed.find(L">", m_filePosition);
		}
		if (dwBytesRead != 0 || m_strFeed.length() != 0)
			m_filePosition = (itemStop_end + 1);

		if ((dwBytesRead != 0 || m_strFeed.length() != 0) && itemStop_end != -1)
		{
			// Get the string to send to XML tree and copy the partial block for next loop.
			std::wstring tempFeed (m_strFeed, itemStart_begin, (itemStop_end - itemStart_begin) + 1);
			std::wstring tempPartial (m_strFeed, itemStop_end + 1);
			m_strFeed = tempFeed;
			m_strFeedPartial = tempPartial;

			// Return the length of the RSS item just for fun.
			ret = m_strFeed.length();
			m_itemNum++;
		}
	}
	else
	{
		m_strFeed = L"";
		m_itemNum = 0;
	}
	
	ret = m_strFeed.length();
	return ret;
}
#pragma endregion