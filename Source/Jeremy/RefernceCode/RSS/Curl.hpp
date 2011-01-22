#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_CURL_
#define _RSSREADER_CURL_

#include <vector>
#include <string>

#include <curl/curl.h>

namespace RssReader { namespace cURL
{
	class GlobalState sealed
	{
	public:
		GlobalState();
		~GlobalState();

	private:
		GlobalState(const GlobalState& globalState) { } // disallow copying
	};

	class EasyInterface sealed
	{
		CURL* const _curlHandle;
		const ::std::string _url;
		curl_slist* _requestHeaders;
		::std::string _responseData;
		::std::vector< ::std::string > _responseHeaders;

	public:
		EasyInterface(const ::std::string& url, const ::std::string& etag);
		~EasyInterface();

	private:
		EasyInterface(const EasyInterface& easy) : _curlHandle(nullptr) { } // disallow copying

	public:
		const ::std::string& GetResponseData() const;
		const ::std::vector< ::std::string >& GetResponseHeaders() const;

	public:
		long PerformRequest();
		::std::string GetResponseEtag() const;

	private:
		static size_t WriteFunction(void* data, size_t mult, size_t size, void* state);
		static size_t HeaderFunction(void* data, size_t mult, size_t size, void* state);

	private:
		long GetResponseCode() const;
		void HandleWriteData(const char* data, const size_t dataSize);
		void HandleHeaderData(const char* data, const size_t dataSize);
	};
}}

#endif
