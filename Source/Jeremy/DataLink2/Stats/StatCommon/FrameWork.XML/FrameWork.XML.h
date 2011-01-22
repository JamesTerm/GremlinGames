#pragma once
#define FRAMEWORKXML_API

// We need std::queue implementations
#include <stack>
#include <vector>

namespace FrameWork
{
	namespace xml
	{
		#include "xml_node.h"
		#include "xml_parser.h"
		#include "xml_file.h"
		#include "xml_string.h"
		#include "xml_tree.h"

		namespace utilities
		{
			#include "xml_string_utility.h"
		}
	};
	
#if 0
	// test
	namespace xml2
	{
		typedef	xml::node	node;
	
		#include "xml2_parser.h"
		#include "xml2_file.h"
		#include "xml2_string.h"
		#include "xml2_tree.h"
	};
#endif
};

namespace FXML = FrameWork::xml;