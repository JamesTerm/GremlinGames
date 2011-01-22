// GG_Framework.Base ArgumentParser.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"

using namespace GG_Framework::Base;

std::vector<std::string> IArgumentParser::ProcessArguments(unsigned argc, const char** argv)
{
	std::vector<std::string> ret;
	for (unsigned index = 0; index < argc; )
	{
		if (argv[index][0] == '-')
		{
			unsigned oldIndex = index;
			if (!ProcessFlag(index, argc, argv))
			{
				printf("Invalid Flag: %s\n", argv[oldIndex]);
				++index;
			}
			ASSERT(oldIndex != index);	//Make sure the flag was incremented
		}
		else
		{
			ret.push_back(argv[index]);
			++index;
		}
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////

