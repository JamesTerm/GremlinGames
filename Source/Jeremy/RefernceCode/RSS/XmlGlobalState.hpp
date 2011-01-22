#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_XMLGLOBALSTATE_
#define _RSSREADER_XMLGLOBALSTATE_

namespace RssReader
{
	class XmlGlobalState sealed
	{
	public:
		XmlGlobalState();
		~XmlGlobalState();

	private:
		XmlGlobalState(const XmlGlobalState& xmlGlobalState) { } // disallow copying
	};
}

#endif
