#include "StdAfx.h"
#include "FrameWork_Threads.h"

double	profile::m_freq = 0.0;

// Constructor
profile::profile( void )
{	// Get the frequency if needed
	if (!m_freq)
	{	LARGE_INTEGER freq;
		::QueryPerformanceFrequency( &freq );
		m_freq = 1.0 / (double)freq.QuadPart;
	}

	// Start timing
	::QueryPerformanceCounter( &m_count );
}

// Stop timing and return the number of seconds elapsed
double profile::operator() ( const bool reset )
{	LARGE_INTEGER	new_count, old_count = m_count;
	::QueryPerformanceCounter( &new_count );
	if (reset) m_count = new_count;
	const __int64 d_count = new_count.QuadPart - old_count.QuadPart;
	return (double)d_count * m_freq;
}