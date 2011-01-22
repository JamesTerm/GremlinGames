// DLL Base Header

#pragma once

// DLL Exports

// Wrapping Framework Communications 3
#include "..\..\..\Framework\Communication3\FrameWork.Communication3.h"

namespace unmanaged_FC3_xml = FrameWork::Communication3::xml;
namespace unmanaged_xml = FrameWork::xml;

#pragma managed


namespace FrameWork
{
	namespace Communication3
	{
		namespace CLR
		{
			#include "ClrMessageXML.h"
			#include "ClrUtilities.h"
			#include "ClrRecvServer.h"
			#include "ClrXmlTree.h"

			#include "ClrLifetimeControl.h"
		};
	};
};