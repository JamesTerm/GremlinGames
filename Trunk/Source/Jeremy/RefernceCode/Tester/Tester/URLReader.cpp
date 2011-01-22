//

#include "stdafx.h"
#include "URLReader.h"

// Constructor.
URLReader::URLReader(const wchar_t * feed_URL) : m_FeedURL(feed_URL)
{
}

// Destructor.
URLReader::~URLReader(void)
{
}

// Initialize by checking for internet connection and creating handles.
bool URLReader::Initialize(void)
{
	size_t result = 0;
	bool ret = false;
	
	// Check Internet connectivity.
	result = InternetAttemptConnect(NULL);

	// Open Internet handle for connection.
	if (result == 0)
	{
		m_hInternet = InternetOpen(NULL, LOCAL_INTERNET_ACCESS, NULL, 0, 0);
		result += GetLastError();
	}

	// Open URL handle.
	if (result == 0)
	{
		m_hOpenUrl = InternetOpenUrl(	m_hInternet, m_FeedURL, NULL, 0,
										INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, 0);
		result += GetLastError();
	}
	
	// If connection and opens were successful, return true.
	if (result == 0) ret = true;

	return ret;
}

// Close handles.
bool URLReader::CloseHandles(void)
{
	size_t result = 0;
	bool ret = false;

	// Close handles in reverse order.
	InternetCloseHandle(m_hOpenUrl);
	result += GetLastError();

	InternetCloseHandle(m_hInternet);
	result += GetLastError();

	// If both closes were successful, return true.
	if (result == 0) ret = true;

	return ret;
}

// Creates feed in std::string m_Feed.
bool URLReader::CreateFeed(void)
{
	DWORD dwBlockSize = 4096;
	DWORD dwBytesRead = 0;
	char * cFeedBlock = (char *)_alloca(dwBlockSize + 1);
	bool more_data = true;
	bool ret = true;

	// Read feed in blocks and add to string.
	while (more_data)
	{
		if (InternetReadFile(m_hOpenUrl, cFeedBlock, dwBlockSize, &dwBytesRead))
		{
			// Grab another chunk if there is more data.
			if (dwBytesRead > 0)
			{
				// Zero term each block and add to string.
				cFeedBlock[dwBytesRead] = '\0';
				m_Feed += cFeedBlock;
			}
			else	// All done.
				more_data = false;
		}
		else	// Cannot read file.
			ret = false;
	}
	assert(ret);
	return ret;
}