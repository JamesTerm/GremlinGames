#include "StdAfx.hpp"
#include "Curl.hpp"

#include <cstdlib>
#include <boost/algorithm/string.hpp>

namespace RssReader { namespace cURL
{
	GlobalState::GlobalState()
	{
		curl_global_init(CURL_GLOBAL_WIN32);
	}

	GlobalState::~GlobalState()
	{
		curl_global_cleanup();
	}

	EasyInterface::EasyInterface(const std::string& url, const std::string& etag) :
		_curlHandle(curl_easy_init()),
		_url(url),
		_requestHeaders(nullptr),
		_responseData(),
		_responseHeaders()
	{
		_responseHeaders.reserve(20);

		curl_easy_setopt(_curlHandle, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(_curlHandle, CURLOPT_NOPROGRESS, 1);
		curl_easy_setopt(_curlHandle, CURLOPT_URL, _url.c_str());

		if (!etag.empty())
		{
			std::string etagHeader("If-None-Match: ");
			etagHeader += etag;
			_requestHeaders = curl_slist_append(_requestHeaders, etagHeader.c_str());
			curl_easy_setopt(_curlHandle, CURLOPT_HTTPHEADER, _requestHeaders);
		}

		curl_easy_setopt(_curlHandle, CURLOPT_HEADERFUNCTION, &EasyInterface::HeaderFunction);
		curl_easy_setopt(_curlHandle, CURLOPT_HEADERDATA, this);

		curl_easy_setopt(_curlHandle, CURLOPT_WRITEFUNCTION, &EasyInterface::WriteFunction);
		curl_easy_setopt(_curlHandle, CURLOPT_WRITEDATA, this);
	}

	EasyInterface::~EasyInterface()
	{
		if (_requestHeaders)
			curl_slist_free_all(_requestHeaders);
		curl_easy_cleanup(_curlHandle);
	}

	const std::string& EasyInterface::GetResponseData() const
	{
		return _responseData;
	}

	const std::vector<std::string>& EasyInterface::GetResponseHeaders() const
	{
		return _responseHeaders;
	}

	long EasyInterface::PerformRequest()
	{
		return curl_easy_perform(_curlHandle) == CURLE_OK ? GetResponseCode() : 0;
	}

	std::string EasyInterface::GetResponseEtag() const
	{
		for (std::vector<std::string>::const_iterator i = _responseHeaders.begin(); i != _responseHeaders.end(); ++i)
			if (boost::algorithm::istarts_with(*i, "etag:"))
				return i->substr(6, i->size() - 8); // subtract 2 extra bytes for the trailing CRLF
		return std::string();
	}

	size_t EasyInterface::WriteFunction(void* data, size_t mult, size_t size, void* state)
	{
		const size_t dataSize = size * mult;
		if (dataSize && state)
			reinterpret_cast<EasyInterface*>(state)->HandleWriteData(reinterpret_cast<char*>(data), dataSize);
		return dataSize;
	}

	size_t EasyInterface::HeaderFunction(void* data, size_t mult, size_t size, void* state)
	{
		const size_t dataSize = size * mult;
		if (dataSize && state)
			reinterpret_cast<EasyInterface*>(state)->HandleHeaderData(reinterpret_cast<char*>(data), dataSize);
		return dataSize;
	}

	long EasyInterface::GetResponseCode() const
	{
		long code = 0;
		return curl_easy_getinfo(_curlHandle, CURLINFO_RESPONSE_CODE, &code) == CURLE_OK ? code : 0;
	}

	void EasyInterface::HandleWriteData(const char* data, const size_t dataSize)
	{
		_responseData.append(data, dataSize);
	}

	void EasyInterface::HandleHeaderData(const char* data, const size_t dataSize)
	{
		_responseHeaders.push_back(std::string(data, dataSize));

		data = _responseHeaders.back().c_str();
		if (!boost::algorithm::istarts_with(data, "content-length:"))
			return;

		int contentLength = std::atoi(data + 16);
		if (contentLength)
			_responseData.reserve(contentLength);
	}
}}
