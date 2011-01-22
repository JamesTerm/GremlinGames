// ReleaseDebugFile.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"
#include <OpenThreads/Thread>

int OutputFileNumber = 0;

int GetThreadID()
{
	OpenThreads::Thread* thisThread = OpenThreads::Thread::CurrentThread();
	if (thisThread) 
		return thisThread->getThreadId()+1;
	else return 0;
}
namespace GG_Framework
{
	namespace Base
	{
		std::map<int, ReleaseDebugFile*> ReleaseDebugFile::s_DebugFileMap;
		bool ReleaseDebugFile::FilesWritten = false;

		ReleaseDebugFile::ReleaseDebugFile()
		{
			m_threadID = GetThreadID();
			m_parent = s_DebugFileMap[m_threadID];
			s_DebugFileMap[m_threadID] = this;
		}

		ReleaseDebugFile::~ReleaseDebugFile()
		{
			// Put the map back in place
			s_DebugFileMap[m_threadID] = m_parent;
		}

		void ReleaseDebugFile::OutputToFile(FILE* file)
		{
			if (m_parent)
				m_parent->OutputToFile(file);
			fprintf(file, "%s", MESSAGE.c_str());
		}

		void ReleaseDebugFile::OutputDebugFile(const char* titlePrefix)
		{
			int threadID = GetThreadID();
			ReleaseDebugFile* curr = s_DebugFileMap[threadID];
			if (curr)
			{
				std::string fileName = BuildString("%s_%i_%i.txt", titlePrefix, threadID, OutputFileNumber++);
				FILE* file = fopen(fileName.c_str(), "w");
				curr->OutputToFile(file);
				fclose(file);
				printf("Wrote Debug Output to %s\n", fileName.c_str());
				FilesWritten = true;
			}
			else
				printf("No Data to write for %s\n", titlePrefix);
		}

		ReleaseDebugFile* ReleaseDebugFile::GetCurrent()
		{
			int threadID = GetThreadID();
			ReleaseDebugFile* curr = s_DebugFileMap[threadID];
			return curr;
		}
	}
}