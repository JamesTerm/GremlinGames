#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_MEMPARSEHANDLERS_
#define _RSSREADER_MEMPARSEHANDLERS_

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXParseException.hpp>

class MemParseHandlers sealed : public ::xercesc_2_7::HandlerBase
{
	bool _success;

public:
	MemParseHandlers();

public:
	bool GetSuccess();

public:
	virtual void error(const ::xercesc_2_7::SAXParseException& ex) override;
	virtual void fatalError(const ::xercesc_2_7::SAXParseException& ex) override;
};

#endif
