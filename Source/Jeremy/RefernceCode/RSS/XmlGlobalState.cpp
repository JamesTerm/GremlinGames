#include "StdAfx.hpp"
#include "XmlGlobalState.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/XalanTransformer/XalanTransformer.hpp>

namespace RssReader
{
	XmlGlobalState::XmlGlobalState()
	{
		xercesc_2_7::XMLPlatformUtils::Initialize();
		xalanc_1_10::XalanTransformer::initialize();
	}

	XmlGlobalState::~XmlGlobalState()
	{
		xalanc_1_10::XalanTransformer::terminate();
		xercesc_2_7::XMLPlatformUtils::Terminate();
	}
}
