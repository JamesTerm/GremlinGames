#include "StdAfx.hpp"
#include "MemParseHandlers.hpp"

MemParseHandlers::MemParseHandlers() :
	xercesc_2_7::HandlerBase(),
	_success(true)
{
}

bool MemParseHandlers::GetSuccess()
{
	return _success;
}

void MemParseHandlers::error(const xercesc_2_7::SAXParseException& ex)
{
	//xercesc_2_7::HandlerBase::error(ex);
	_success = false;
}

void MemParseHandlers::fatalError(const xercesc_2_7::SAXParseException& ex)
{
	//xercesc_2_7::HandlerBase::fatalError(ex);
	const XMLCh* foo = ex.getMessage();
	_success = false;
}
