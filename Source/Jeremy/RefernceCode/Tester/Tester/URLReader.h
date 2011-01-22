//

#pragma once

class URLReader
{
	public:
		// Constructor - pass the wchar_t* address.
		URLReader(const wchar_t * feed_URL);		
		~URLReader(void);

		// Return the feed string.
		std::string GetFeed(void) { return m_Feed; }
		
		// Open handles.
		bool Initialize(void);

		// Close handles.
		bool CloseHandles(void);

		// Create the feed.
		bool CreateFeed(void);

	protected:

	private:
		std::string m_Feed;
		const wchar_t * m_FeedURL;
		HINTERNET m_hInternet, m_hOpenUrl;
};