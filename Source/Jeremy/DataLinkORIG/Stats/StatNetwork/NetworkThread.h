#pragma once

#include "..\StatCommon\Main.h"

// Global constants
static const wchar_t * cwsz_sitesPath = L"Software\\NewTek\\DataLink\\Active\\Sites";
static const size_t c_MaxQueueDepth = 50;	// Maximum queue depth from key/value queue.

// Typedef used to store key/value pairs.
typedef std::pair<std::wstring, std::wstring> key_value_pair;

class NetworkThread
{	
	public:
		// Constructor and destructor
		NetworkThread(void);
		~NetworkThread(void);

		// The handle to use
		HANDLE NetworkThread::get_handle(void) const;

		// Has the key actually changed since the last time
		const wchar_t *p_get_changed(void);

		// Get the value
		const wchar_t *p_get_value(const wchar_t key[]);

		// Release the value
		void release_value(void);
		
		// Release the key
		void release_changed_key(void);

	protected:
	
	private:
		// Thread Implementation.  SetEvent must be called
		// at some point in the thread for TriCaster™ to ask for Keys and Values.
		static void NetworkThreadProc(void *p_this);
		void NetworkThreadRunner(void);
		
		// Returns site values from the registry.
		DWORD RegEnumSites(const wchar_t * path);
		const wchar_t * RegGetSiteText(const wchar_t * path, const wchar_t * val,const wchar_t * retVal);
		const DWORD RegGetSiteNum(const wchar_t * path, const wchar_t * val);


		// Should the thread exit
		bool m_thread_must_exit;

		// The handles
		HANDLE m_key_changed;
		HANDLE m_thread_handle;

		// Critical section
		CriticalSection m_key_lock;


		//// Item / tag parsing and XML tree ////
		// Initialize the defined sites.
		bool GetSites();

		// Get an Item for parsing from each site.
		bool GetItem();

		// Get a tag from each item for each site.
		bool GetTag();

		// Create an XML Tree of an item.
		bool CreateItemXMLTree(DWORD siteIndex);


		//// Queue functions for writing key/value pairs ////
		// Add to queue.
		void QWrite();

		// Get queue size.
		size_t QSize() {return m_KeyValueQueue.size();}

		// Check if queue is empty.
		bool QEmpty() {return m_KeyValueQueue.empty();}


		// Vector of sites
		std::vector<URLReader> m_vSiteElement;
		
		// XML tree of current item.
		FrameWork::xml::tree m_treeItem;
		FrameWork::xml::tree m_childItem;

 		// The key (first) / value (second) pair.
		key_value_pair m_KeyValue;

		// Iterator for the total number of sites.
		DWORD m_numSites;

		// Queue for key/value pairs incoming from site instances (URLReader).
		std::queue<key_value_pair> m_KeyValueQueue;
};