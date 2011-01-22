#pragma once

#include "..\StatCommon\Main.h"

#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

// Global constants
static const wchar_t * cwsz_sitesPath = L"Software\\NewTek\\DataLink\\Active\\Sites";
static const size_t c_MaxQueueDepth = 50;	// Maximum queue depth from key/value queue.

// Used to store key/value pairs.
typedef std::pair<std::wstring, std::wstring> key_value_pair;

enum WhiteSpace
{
	eNONE,
	eSPACE,
	eCARRIAGE_RETURN,
	eLINE_FEED,
	eTAB
};

// Struct that holds the text and # HTML codes as well as the actual replacement string
//   for text incoming from the XMLTree.
struct Code_Elements
{
	const wchar_t* alphaCode;
	const wchar_t* numberCode;
	const wchar_t* replaceCode;
};

// Elements that we want to replace to maintain the most amount of intended data in the presentation.
//  The order of the elements is optimized according to likelihood of occurrence.
const Code_Elements HTMLCode[] = {
	{	L"&amp;",		L"&#38;",	L"&"		},	// Ampersand (&)
	{	L"&mdash;",		L"&#8212;",	L"\x2014"	},	// Em dash
	{	L"&copy;",		L"&#169;",	L"\xa9"		},	// Copyright (©)
	{	L"&reg;",		L"&#174;",	L"\xae"		},	// Registered trademark (®)
	{	L"&trade;",		L"&#8482;",	L"\x2122"	},	// Trademark
	{	L"&deg;",		L"&#176;",	L"\xb0"		},	// Degree (°)
	{	L"&cent;",		L"&#162;",	L"\xa2"		},	// Cent (¢)
	{	L"&nbsp;",		L"&#160;",	L" "		},	// Non-breaking space
	{	L"&lsquo;",		L"&#8216;",	L"'"		},	// Left single quote
	{	L"&rsquo;",		L"&#8217;",	L"'"		},	// Right single quote
	{	L"&ldquo;",		L"&#8220;",	L"\""		},	// Left double quote
	{	L"&rdquo;",		L"&#8221;",	L"\""		},	// Right double quote
	{	L"&quote",		L"&#34;",	L"\""		},	// Double quote
	{	L"&emsp;",		L"&#8195;",	L" "		},	// Em space
	{	L"&euro;",		L"&#8364;",	L"\xa5"		},	// Euro
	{	L"&pound;",		L"&#163;",	L"\xa3"		},	// British pound (£)
	{	L"&yen;",		L"&#165;",	L"\xa5"		},	// Yen (¥)
	{	L"&ndash;",		L"&#8211;",	L"-"		},	// En dash
	{	L"&iexcl;",		L"&#161;",	L"\xa1"		},	// Inverted exclamation (¡)
	{	L"&iquest;",	L"&#191;",	L"\xbf"		},	// Inverted question (¿)
	{	L"&bull",		L"&#8226;",	L"\x2022"	},	// Bullet
	{	L"&fnof;",		L"&#402;",	L"\x192"	},	// Florin
	{	L"&amp;",		L"&#38;",	L"..."		},	// Elipsis
	{	L"&ensp;",		L"&#8194;",	L" "		},	// En space
	{	L"&thinsp;",	L"&#8201;",	L" "		},	// Thin space
	{	L"&curren;",	L"&#164;",	L"\xa4"		}	// Currency (¤)
};


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

		// Should the thread exit
		bool m_thread_must_exit;

		// The handles
		HANDLE m_key_changed;
		HANDLE m_thread_handle;

		// Critical section
		CriticalSection m_key_lock;


		//// Queue functions for writing key/value pairs ////
		// Add to queue.
		bool QWrite();

		// Get queue size.
		size_t QSize() {return m_KeyValueQueue.size();}

		// Check if queue is empty.
		bool QEmpty() {return m_KeyValueQueue.empty();}


		//// Item / tag parsing and XML tree ////
		// Initialize the defined sites.
		bool GetSites();

		// Get an Item for parsing from each site.
		bool GetItem();

		// Get a tag from each item for each site.
		bool GetTag();

		// Create an XML Tree of an item.
		bool CreateItemXMLTree(DWORD siteIndex);

		// Wipes out all data in angled brackets as we do not need.
		bool GetTag_CleanBrackets();

		// Fix value to remove/replace embedded HTML stuff.
		bool GetTag_FixAmps();

		// Gets rid of white space at nPos if exits. Loops and deletes until a non-whitespace is found at nPos.
		bool EraseWhitespace(DWORD nPos);

		// Detects whitespace and returns type.
		WhiteSpace IsWhitespace(DWORD nPos);

		// Process the finalized tag.
		bool GetTag_Process();


		// Returns site values from the registry.
		DWORD RegEnumSites(const wchar_t * path);
		const wchar_t * RegGetSiteText(const wchar_t * path, const wchar_t * val,const wchar_t * retVal);
		const DWORD RegGetSiteNum(const wchar_t * path, const wchar_t * val);


		// Vector of sites
		std::vector<URLReader> m_vSiteElement;
		
		// XML tree of current item.
		FrameWork::xml::tree m_treeItem;

 		// The key (first) / value (second) pair.
		key_value_pair m_KeyValue;

		// Iterator for the total number of sites.
		DWORD m_numSites;

		// Queue for key/value pairs incoming from site instances (URLReader).
		std::queue<key_value_pair> m_KeyValueQueue;
};