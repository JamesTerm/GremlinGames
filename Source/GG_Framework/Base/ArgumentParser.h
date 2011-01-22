// GG_Framework.Base ArgumentParser.h
#pragma once

namespace GG_Framework
{
	namespace Base
	{
		class FRAMEWORK_BASE_API IArgumentParser
		{
		public:
			//! Call this from the executable to process available arguments
			std::vector<std::string> ProcessArguments(unsigned argc, const char** argv);

		protected:
			//! Override this function to process a flag, return false for an unknown flag
			//! increment index to the next position past the current flag and options
			virtual bool ProcessFlag(unsigned& index, unsigned argc, const char** argv)
			{
				return false;
			}
		};
		//////////////////////////////////////////////////////////////////////////
	}
}