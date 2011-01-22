// NOTE: USED ONLY WHEN BUILDING OUTSIDE OF VT_SOLUTION, SEE stdafx.h

class VariableBlock
{	private:
		CRITICAL_SECTION myBlock;
		long m_Locks;
		//DWORD m_ThreadID;

	public:
		//uncomment this for debugging lockups (see Threads.cpp)
		//DWORD m_ThreadID;

		void Block(void);
		bool TryBlock(void);
		void UnBlock(void);

		VariableBlock(void);
		~VariableBlock(void);
};

class FunctionBlock
{	private:	VariableBlock	*Ptr;

	public:		FunctionBlock(VariableBlock *Fcn);
				FunctionBlock(VariableBlock &Fcn);
				~FunctionBlock(void);
};

class Thread
{	protected:
		HANDLE			MyHandle;
		unsigned long	MyHandleID;		

		static void FireStarter(void *ptr);

	public:
		//! overload this function with your thread entry point
		virtual void ThreadEntryPoint(void)=0;//		{}

		//! Start The thread (This function can only be called once from its creation)
		void StartThread( const int StackSize = 128*1024 );	// Increasing this uses memory !

		//! Set the thread priority
		void SetThreadPriority(int nPriority);

		//! build uup and tear down
		Thread(void);
		virtual ~Thread(void);
};

class WorkerThread : public Thread, public VariableBlock
{	
	private:	//**********************************************************************************************************
				//! Thread stuff
				bool IsThreadRunning;
				bool ThreadMustExit;

				//**********************************************************************************************************
	protected:	
				bool GetRunning(void);
				void SetRunning(bool Flag);

				//**********************************************************************************************************
				bool GetThreadMustExit(void);
				void SetThreadMustExit(bool Flag=true);

	public:		//**********************************************************************************************************
				//! The thread entry point, do not overload this
				virtual void ThreadEntryPoint(void);

				//! This is the one to overload !
				virtual void ThreadProcessor(void) { Sleep(1); }	//***********
				virtual void ThreadProcessor_Start(void);			//***********
				virtual void ThreadProcessor_End(void);				//***********

				//! How to start and stop the thread
				//! If you overload these, cast them backwards to WorkerThread::

				//! StartThread() can only be called once from its creation
				virtual void StartThread(void);

				//! You want to call this explicitly from your subclass destructor 
				//! (esp. if you override ThreadProcessor_End)
				virtual void StopThread( const int TimeOut=1500 );

				//! Init data members
				WorkerThread(void);

				//! Destructor checks thread has stopped
				~WorkerThread(void);
};