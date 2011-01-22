#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_APPLICATION_
#define _RSSREADER_APPLICATION_

#include <wx/app.h>

#include "Curl.hpp"
#include "XmlGlobalState.hpp"

namespace RssReader
{
	class Application sealed : public wxApp
	{
		const cURL::GlobalState _curlGlobal;
		const XmlGlobalState _xmlGlobal;

	public:
		Application();

	public:
		virtual bool OnInit() override;
	};
}

#endif
