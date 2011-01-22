#pragma once

struct profile
{			// Constructor
			profile( void );

			// Stop timing and return the number of seconds elapsed
			// stop automatically does a start, so you can poll the 
			// times.
			double operator() ( const bool reset = true );

private:	// The internal timers
			static double	m_freq;
			LARGE_INTEGER	m_count;
};