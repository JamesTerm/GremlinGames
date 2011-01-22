#include "StdAfx.h"
#include "FrameWork_Communication3_CLR.h"

#include <set>
#include "..\..\..\FrameWork\threads\FrameWork.Threads.h"
#include "..\..\..\FrameWork\Versions\FrameWork.Versions.h"
#include "../../../Application/Launcher/Application.Launcher.h"

namespace FrameWork { namespace Communication3 { namespace CLR {

lifetime_control::lifetime_control(void) 
: m_thread(nullptr), m_b_shutdown(false)
{	
	// Get the dead time
	__int64 l_dead_at;
	::QueryPerformanceFrequency((LARGE_INTEGER*)&l_dead_at); 
	m_dead_at = 3 * l_dead_at / 2;

	// Start the thread
	System::Threading::ThreadStart ^thread_delegate = gcnew System::Threading::ThreadStart(this, &lifetime_control::thread_proc);
	m_thread = gcnew System::Threading::Thread(thread_delegate);
	m_thread->Start();
}

lifetime_control::~lifetime_control(void)
{	
	m_b_shutdown = true;
	m_thread->Join();
	m_thread = nullptr;
}

//static
void lifetime_control::thread_proc(void)
{	
	while (!m_b_shutdown)
	{
		// Sleep
		::Sleep(1000);

		// Get the launcher heard-bear
			  __int64 current_time; ::QueryPerformanceCounter((LARGE_INTEGER*)&current_time);
		const __int64 launcher_time = FC3::utilities::heart_beat(p_main_launcher_name);

		// Have we died
		if (current_time-launcher_time > m_dead_at)
		{	
			// Terminate this process
			::TerminateProcess(::GetCurrentProcess(), 0);

			// Incase the above failed
			::exit(0);
		}	
	}
}

}}} // closing namespace