//
#include "..\StatCommon\Main.h"

typedef unsigned __int64 TICK;

class URLReader
{
public:
	// Constructor - destructor
	//  Expects user name for site, site address, block size to eat, refresh time in SECONDS, timeout time in MILLI seconds.
	URLReader(void);
	URLReader(	const wchar_t * name,
				const wchar_t * site,
				DWORD blockSize,
				DWORD refresh,
				DWORD timeout);
	~URLReader(void);

	// Open handles.
	bool Initialize(void);

	// Close handles.
	bool CloseHandles(void);

	// Get another block - returns bytes read.
	DWORD GetBlock(void);

	// Assemble the feed and send.
	DWORD ParseItem(void);

	// Accessors.
	TICK GetRefreshTime() {return m_refresh;}				// Get the refresh interval.
	TICK GetTimeLastUpdated() {return m_timeLastUpdated;}	// Get the last time the site was finished being processed.
	bool GetConnectedState() {return m_isConnected;}		// Get connected flag.
	const wchar_t * GetName() {return m_name;}				// Get the name.
	std::wstring GetItem() {return m_strFeed;}				// Gets the item string.
	DWORD GetItemNum() {return m_itemNum;}					// Gets the number of the current item.

	// Mutators.
	void SetTimeLastUpdated(DWORD timeUpdate) {m_timeLastUpdated = timeUpdate;}	// Set the next time site should be checked.
	void SetConnectedState(bool isConnected) {m_isConnected = isConnected;}		// Set connected flag.

protected:

private:
	// URLReader
	const wchar_t * m_feedURL, * m_name;
	DWORD m_refresh, m_timeout, m_blockSize, m_itemNum;
	bool m_isConnected;
	TICK m_timeLastUpdated;

	HINTERNET m_hInternet, m_hOpenUrl;

	int m_filePosition;

	std::wstring m_strFeed, m_strFeedPartial;

};