#pragma once

// Utility class that can be instantiated in stand alone managed EXEs that will force terminate with the top level launcher is terminated.
public ref class lifetime_control 
{	
public:
	lifetime_control(void);
	~lifetime_control(void);

private:
	void thread_proc(void);

private:
	__int64	m_dead_at;	// This is the time that we consider to be dead at

	// The thread
	System::Threading::Thread^ m_thread;
	bool m_b_shutdown;
};