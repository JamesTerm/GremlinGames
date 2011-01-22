#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef _RSSREADER_STDAFX_
#define _RSSREADER_STDAFX_

#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_WARNINGS

#define NTDDI_VERSION 0x05010200
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0600
#define WIN32_LEAN_AND_MEAN
#define STRICT 1
#endif

#define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG
#define CURL_STATICLIB

#pragma warning(push)

#pragma warning(disable : 4267)
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>

#pragma warning(disable : 4244)
#pragma warning(disable : 4793)
#include <boost/date_time/posix_time/time_serialize.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <wx/wxprec.h>
#include <wx/utils.h>
#include <wx/fs_inet.h>
#include <wx/imaglist.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/listctrl.h>
#include <wx/treectrl.h>
#include <wx/html/htmlwin.h>
#include <wx/xrc/xmlres.h>

#ifdef _WIN32
#include <shlobj.h>
#endif

#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xalanc/Include/PlatformDefinitions.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

#include <curl/curl.h>

#pragma warning(pop)

#define nullptr 0

#if !defined(_MSC_VER) || (_MSC_VER < 1400)
#define sealed
#define override
#endif

#endif
