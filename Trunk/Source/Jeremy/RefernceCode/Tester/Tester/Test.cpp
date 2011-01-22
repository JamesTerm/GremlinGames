#include "stdafx.h"
#include "Test.h"

#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

using namespace std;

//TESTING
// Example RSS feed addresses -	http://rss.cnn.com/rss/cnn_topstories.rss/
// (don;t forget the end '/')	http://feeds.arstechnica.com/arstechnica/everything/
const wchar_t * wcsz_site = L"http://feeds.arstechnica.com/arstechnica/everything/";

int main(int argc, char** argv)
{	
	CRITICAL_SECTION CS;
	::InitializeCriticalSection(&CS);

	int test = 0;

	for (int i = 0; i < 100; i++)
	{
		::EnterCriticalSection(&CS);
		test++;
		Sleep(1000);
		::LeaveCriticalSection(&CS);

		

	}



// 	// Instantiate with site value.
// 	URLReader reader(wcsz_site);
// 	std::string testing;
// 
// 	// Initialize connection.
// 	if (reader.Initialize())
// 	{
// 		// Create feed.
// 		reader.CreateFeed();
// 		
// 		// Return the Feed string to be processed.
// //		return reader.GetFeed();
// 		Sleep(1);
// 	}
// 	else
// 	{
// 		// Connection problem.
// 		assert(false);
// 	}
	return 0;


///////////////////////////////////////////////////////////////////////////////////////
/*
		//TESTING
		ssFeed = ("airplane dog cat man woman red blue black white dog tree floor Paris light dark dog");

		size_t ssFeedLen = ssFeed.length();
		size_t found = 0;
		size_t found1 = 0;
		size_t found2 = 0;
		size_t found3 = 0;
		size_t found4 = 0;

		// Test: Search for strings
		found = ssFeed.find("dog", 0);
		if (found!=string::npos)
			found1 = int(found);
		found = ssFeed.find("dog",found+1);		// starts looking at +1 of beginning of last found occurrence
							
		if (found!=string::npos)
			found2 = int(found);
		found = ssFeed.find("dog",found+1);
		if (found!=string::npos)
			found3 = int(found);
		found = ssFeed.find("dog",found+1, 3);
		if (found!=string::npos)
			found4 = int(found);

		// Close handles in reverse order.
		InternetCloseHandle(hOpenUrl);
		InternetCloseHandle(hInternet);
	}
	else
		assert(false); // Internet connection attempt failed.

	return 0;
*/
}
