// Thread.h
#pragma once
#include <OpenThreads/Mutex>
#include <OpenThreads/Thread>

namespace GG_Framework
{
	namespace Base
	{
		FRAMEWORK_BASE_API void ThreadSleep(unsigned sleepMS);

		class FRAMEWORK_BASE_API RefMutexWrapper
		{
		public:
			RefMutexWrapper(OpenThreads::Mutex& mutex) : m_mutex(mutex) {m_mutex.lock();}
			~RefMutexWrapper() {m_mutex.unlock();}

		private:
			OpenThreads::Mutex& m_mutex;
		};

		class FRAMEWORK_BASE_API MutexWrapper : RefMutexWrapper
		{
		public:
			MutexWrapper() : RefMutexWrapper(m_ownMutex) {}
		private:
			OpenThreads::Mutex m_ownMutex;
		};

		class FRAMEWORK_BASE_API ThreadedClass : public OpenThreads::Thread
		{
		public:
			virtual void run();
			static bool ERROR_STATE;

		protected:
			virtual void tryRun() = 0;
		};
	}
}