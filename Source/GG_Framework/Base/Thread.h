// Thread.h
#pragma once
#include <OpenThreads/Mutex>
#include <OpenThreads/Thread>
#include <OpenThreads/Atomic>
#include <osg/Timer>

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

		//I've added both techniques... one using OS calls and one which does not.  It appears the one that does not is working fine
		//but there is no evidence of a performance gain yet... However it is the simpler case from a coding standpoint, so lets keep it
		#if 0
		// Note that this simple atomic class should work well when only one thread is writing and 
		// more than one thread is potentially reading.  It does a pointer swap.  Note that this
		// might be atomic anyway without the use of OpenThreads::AtomicPtr.
		template<class T>
		class AtomicT
		{
		private:
			T m_inner[2];
			OpenThreads::AtomicPtr m_atomicRead, m_atomicWrite;

		public:
			AtomicT() : m_atomicRead(&m_inner[0]), m_atomicWrite(&m_inner[1])
			{
			}
			AtomicT(T val) : m_atomicRead(&m_inner[0]), m_atomicWrite(&m_inner[1])
			{
				m_inner[0] = m_inner[1] = val;
			}

			// For reading, just cast to type T
			operator T() const 
			{
				T* ret = (T*)(m_atomicRead.get());
				return *ret;
			}

			// For writing, use =
			void operator=(const T& rhs)
			{
				T* write = (T*)(m_atomicWrite.get());
				*write = rhs;

				//exchange the pointers
				m_atomicWrite.assign(m_atomicRead.get(),m_atomicWrite.get());
				m_atomicRead.assign(write,m_atomicRead.get());
			}
		};
		#else
		template<class T>
		class AtomicT
		{
		private:
			T m_inner[2];
			T *m_atomicRead, *m_atomicWrite;

		public:
			AtomicT() : m_atomicRead(&m_inner[0]), m_atomicWrite(&m_inner[1])
			{
			}
			AtomicT(T val) : m_atomicRead(&m_inner[0]), m_atomicWrite(&m_inner[1])
			{
				m_inner[0] = m_inner[1] = val;
			}

			// For reading, just cast to type T
			operator T() const 
			{
				T *ret = m_atomicRead;
				return *ret;
			}

			// For writing, use =
			void operator=(const T& rhs)
			{
				T *write = m_atomicWrite;
				*write = rhs;

				//exchange the pointers
				m_atomicWrite=m_atomicRead;
				m_atomicRead=write;
			}
		};

		#endif

		class FRAMEWORK_BASE_API ThrottleFrame
		{
		public:
			void SetThrottleFPS(double FPS);
			double GetThrottleFPS() const {return m_throttleFPS;}
			void SetMinSleep(int MinSleep);
			ThrottleFrame()
			{
				Init();
			}
			ThrottleFrame(double FPS,int MinSleep=0)
			{
				Init(FPS,MinSleep);
			}

			double operator()();
		private:
			void Init(double FPS=0.0,int MinSleep=0);
			osg::Timer m_ThrottleTimer;
			double m_throttleFPS;
			int m_MinSleep;
		};

	}
}