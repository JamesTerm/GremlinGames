// ReleaseDebugFile.h
#pragma once
#include <map>

namespace GG_Framework
{
	namespace Base
	{
		class FRAMEWORK_BASE_API ReleaseDebugFile
		{
		public:
			ReleaseDebugFile();
			~ReleaseDebugFile();

			std::string MESSAGE;

			static void OutputDebugFile(const char* titlePrefix);
			static ReleaseDebugFile* GetCurrent();

			static bool FilesWritten;

		private:
			int m_threadID;
			ReleaseDebugFile* m_parent;
			static std::map<int, ReleaseDebugFile*> s_DebugFileMap;
			void OutputToFile(FILE* file);
		};
	}
}