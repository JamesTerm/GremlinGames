#include "stdafx.h"
#include "ClockThread.h"
#include "exports.h"

//Ideally for Debug case we may want to filter out other message for maintenance purposes
// #ifdef _DEBUG
// #define __UpdateVerbose__ 2
// #else
// #define __UpdateVerbose__ 1
// #endif

ClockThread::ClockThread( void )
	:	m_thread_must_exit( false ),
		m_key_changed( ::CreateEvent( NULL, FALSE, FALSE, NULL ) )
{	// Create a critical section for the time value
	::InitializeCriticalSection( &m_local_time_lock );
	
	// Start the thread
	m_thread_handle = ::CreateThread( NULL, 64*1024, (LPTHREAD_START_ROUTINE)ClockThreadProc, (void*)this, 0, NULL);
}

ClockThread::~ClockThread()
{	// Signal the thread to exit
	m_thread_must_exit = true;
	
	// Wait for the thread to exit
	if ( m_thread_handle ) 
	{	::WaitForSingleObject( m_thread_handle, 2000 );
		::CloseHandle(m_thread_handle);
	}

	// CLose the handle
	::CloseHandle( m_key_changed );

	// Release the critical section
	::DeleteCriticalSection( &m_local_time_lock );
}

// The handle to use
HANDLE ClockThread::get_handle( void ) const
{	return m_key_changed;
}

// Has the key actually changed since the last time through
const wchar_t *ClockThread::p_get_changed( void ) const
{	// Return all changed values

// #if (__UpdateVerbose__>=1)
	if ( (bool)m_local_time )	return L"Time";
	if ( (bool)m_local_date )	return L"Date";
// #endif
// #if (__UpdateVerbose__>=2)
	if ( (bool)m_int_time[0] )	return L"Time (+1h)";
	if ( (bool)m_int_time[1] )	return L"Time (+2h)";
	if ( (bool)m_int_time[2] )	return L"Time (+3h)";
	if ( (bool)m_int_time[3] )	return L"Time (+4h)";
	if ( (bool)m_int_time[4] )	return L"Time (+5h)";
	if ( (bool)m_int_time[5] )	return L"Time (+6h)";
	if ( (bool)m_int_time[6] )	return L"Time (+7h)";
	if ( (bool)m_int_time[7] )	return L"Time (+8h)";
	if ( (bool)m_int_time[8] )	return L"Time (+9h)";
	if ( (bool)m_int_time[9] )	return L"Time (+10h)";
	if ( (bool)m_int_time[10] )	return L"Time (+11h)";
	if ( (bool)m_int_time[11] )	return L"Time (+12h)";
	if ( (bool)m_int_time[12] )	return L"Time (-1h)";
	if ( (bool)m_int_time[13] )	return L"Time (-2h)";
	if ( (bool)m_int_time[14] )	return L"Time (-3h)";
	if ( (bool)m_int_time[15] )	return L"Time (-4h)";
	if ( (bool)m_int_time[16] )	return L"Time (-5h)";
	if ( (bool)m_int_time[17] )	return L"Time (-6h)";
	if ( (bool)m_int_time[18] )	return L"Time (-7h)";
	if ( (bool)m_int_time[19] )	return L"Time (-8h)";
	if ( (bool)m_int_time[20] )	return L"Time (-9h)";
	if ( (bool)m_int_time[21] )	return L"Time (-10h)";
	if ( (bool)m_int_time[22] )	return L"Time (-11h)";
	if ( (bool)m_int_time[23] )	return L"Time (-12h)";
	if ( (bool)m_h )			return L"Time (h)";
	if ( (bool)m_hh )			return L"Time (hh)";
	if ( (bool)m_H )			return L"Time (H)";
	if ( (bool)m_HH )			return L"Time (HH)";
	if ( (bool)m_t_m )			return L"Time (M)";
	if ( (bool)m_t_mm )			return L"Time (MM)";
	if ( (bool)m_s )			return L"Time (S)";
	if ( (bool)m_ss )			return L"Time (SS)";
	if ( (bool)m_t )			return L"Time (T)";
	if ( (bool)m_tt )			return L"Time (TT)";	
	if ( (bool)m_d )			return L"Date (D)";
	if ( (bool)m_dd )			return L"Date (DD)";
	if ( (bool)m_ddd )			return L"Date (DDD)";
	if ( (bool)m_dddd )			return L"Date (DDDD)";
	if ( (bool)m_m )			return L"Date (M)";
	if ( (bool)m_mm )			return L"Date (MM)";
	if ( (bool)m_mmm )			return L"Date (MMM)";
	if ( (bool)m_mmmm )			return L"Date (MMMM)";
	if ( (bool)m_y )			return L"Date (Y)";
	if ( (bool)m_yy )			return L"Date (YY)";
	if ( (bool)m_yyyy )			return L"Date (YYYY)";
// #endif	__UpdateVerbose__
	// No other items changed
	return NULL;
}

void ClockThread::ClockThreadProc( void *p_this )
{	// Get the pointer
	ClockThread* ctp=(ClockThread*)p_this;
	
	// Call It
	ctp->ClockThreadRunner();
}

// Get the value
const wchar_t *ClockThread::p_get_value( const wchar_t key[] ) const
{	// Lock
	::EnterCriticalSection( &m_local_time_lock );

	if ( !::wcscmp( key, L"Time" ) )				return (const wchar_t*)m_local_time;
	if ( !::wcscmp( key, L"Date" ) )				return (const wchar_t*)m_local_date;
	if ( !::wcscmp( key, L"Time (+1h)" ) )			return (const wchar_t*)m_int_time[0];
	if ( !::wcscmp( key, L"Time (+2h)" ) )			return (const wchar_t*)m_int_time[1];
	if ( !::wcscmp( key, L"Time (+3h)" ) )			return (const wchar_t*)m_int_time[2];
	if ( !::wcscmp( key, L"Time (+4h)" ) )			return (const wchar_t*)m_int_time[3];
	if ( !::wcscmp( key, L"Time (+5h)" ) )			return (const wchar_t*)m_int_time[4];
	if ( !::wcscmp( key, L"Time (+6h)" ) )			return (const wchar_t*)m_int_time[5];
	if ( !::wcscmp( key, L"Time (+7h)" ) )			return (const wchar_t*)m_int_time[6];
	if ( !::wcscmp( key, L"Time (+8h)" ) )			return (const wchar_t*)m_int_time[7];
	if ( !::wcscmp( key, L"Time (+9h)" ) )			return (const wchar_t*)m_int_time[8];
	if ( !::wcscmp( key, L"Time (+10h)" ) )			return (const wchar_t*)m_int_time[9];
	if ( !::wcscmp( key, L"Time (+11h)" ) )			return (const wchar_t*)m_int_time[10];
	if ( !::wcscmp( key, L"Time (+12h)" ) )			return (const wchar_t*)m_int_time[11];
	if ( !::wcscmp( key, L"Time (-1h)" ) )			return (const wchar_t*)m_int_time[12];
	if ( !::wcscmp( key, L"Time (-2h)" ) )			return (const wchar_t*)m_int_time[13];
	if ( !::wcscmp( key, L"Time (-3h)" ) )			return (const wchar_t*)m_int_time[14];
	if ( !::wcscmp( key, L"Time (-4h)" ) )			return (const wchar_t*)m_int_time[15];
	if ( !::wcscmp( key, L"Time (-5h)" ) )			return (const wchar_t*)m_int_time[16];
	if ( !::wcscmp( key, L"Time (-6h)" ) )			return (const wchar_t*)m_int_time[17];
	if ( !::wcscmp( key, L"Time (-7h)" ) )			return (const wchar_t*)m_int_time[18];
	if ( !::wcscmp( key, L"Time (-8h)" ) )			return (const wchar_t*)m_int_time[19];
	if ( !::wcscmp( key, L"Time (-9h)" ) )			return (const wchar_t*)m_int_time[20];
	if ( !::wcscmp( key, L"Time (-10h)" ) )			return (const wchar_t*)m_int_time[21];
	if ( !::wcscmp( key, L"Time (-11h)" ) )			return (const wchar_t*)m_int_time[22];
	if ( !::wcscmp( key, L"Time (-12h)" ) )			return (const wchar_t*)m_int_time[23];
	if ( !::wcscmp( key, L"Time (h)" ) )			return (const wchar_t*)m_h;
	if ( !::wcscmp( key, L"Time (hh)" ) )			return (const wchar_t*)m_hh;
	if ( !::wcscmp( key, L"Time (H)" ) )			return (const wchar_t*)m_H;
	if ( !::wcscmp( key, L"Time (HH)" ) )			return (const wchar_t*)m_HH;
	if ( !::wcscmp( key, L"Time (M)" ) )			return (const wchar_t*)m_t_m;
	if ( !::wcscmp( key, L"Time (MM)" ) )			return (const wchar_t*)m_t_mm;
	if ( !::wcscmp( key, L"Time (S)" ) )			return (const wchar_t*)m_s;
	if ( !::wcscmp( key, L"Time (SS)" ) )			return (const wchar_t*)m_ss;
	if ( !::wcscmp( key, L"Time (T)" ) )			return (const wchar_t*)m_t;
	if ( !::wcscmp( key, L"Time (TT)" ) )			return (const wchar_t*)m_tt;
	if ( !::wcscmp( key, L"Date (D)" ) )			return (const wchar_t*)m_d;
	if ( !::wcscmp( key, L"Date (DD)" ) )			return (const wchar_t*)m_dd;
	if ( !::wcscmp( key, L"Date (DDD)" ) )			return (const wchar_t*)m_ddd;
	if ( !::wcscmp( key, L"Date (DDDD)" ) )			return (const wchar_t*)m_dddd;
	if ( !::wcscmp( key, L"Date (M)" ) )			return (const wchar_t*)m_m;
	if ( !::wcscmp( key, L"Date (MM)" ) )			return (const wchar_t*)m_mm;
	if ( !::wcscmp( key, L"Date (MMM)" ) )			return (const wchar_t*)m_mmm;
	if ( !::wcscmp( key, L"Date (MMMM)" ) )			return (const wchar_t*)m_mmmm;
	if ( !::wcscmp( key, L"Date (Y)" ) )			return (const wchar_t*)m_y;
	if ( !::wcscmp( key, L"Date (YY)" ) )			return (const wchar_t*)m_yy;
	if ( !::wcscmp( key, L"Date (YYYY)" ) )			return (const wchar_t*)m_yyyy;

	// No matching key
	::LeaveCriticalSection( &m_local_time_lock );
	return NULL;
}

void ClockThread::release_value( void ) const
{	// Lock the value
	::LeaveCriticalSection( &m_local_time_lock );
}

// Add an hour to a time
SYSTEMTIME ClockThread::add_time( const SYSTEMTIME& from, const int hours )
{	// Convert to a file-time
	FILETIME	ft;
	::SystemTimeToFileTime( &from, &ft );

	// Offset by the hours
	*(__int64*)&ft += (__int64)hours * 36000000000;

	// Convert back to a system time
	SYSTEMTIME	st;
	::FileTimeToSystemTime( &ft, &st );

	// Return the result
	return st;
}

void ClockThread::ClockThreadRunner( void )
{	// While we still need to run
	while( !m_thread_must_exit )
	{	// Never miss a second
		::Sleep( 500 );

		// Get the local time
		SYSTEMTIME systemTime;
		::GetLocalTime( &systemTime );

		// Get all fo the values
		wchar_t local_time[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, NULL, local_time, sizeof(local_time) );

		// International times
		wchar_t international_time[ 24 ][ 64 ];
		for( int i=0; i<24; i++ )
		{	const int hr = ( i < 12 ) ? ( i+1 ) : ( -i+11 );
			::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &add_time( systemTime, hr ), NULL, international_time[i], sizeof(international_time[0]) );
		}

		// Hours with no leading zero for single-digit hours; 12-hour clock.
		wchar_t local_h[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"h", local_h, sizeof(local_h) );
		
		// Hours with leading zero for single-digit hours; 12-hour clock.
		wchar_t local_hh[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"hh", local_hh, sizeof(local_hh) );
		
		// Hours with no leading zero for single-digit hours; 24-hour clock.
		wchar_t local_H[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"H", local_H, sizeof(local_H) );
		
		// Hours with leading zero for single-digit hours; 24-hour clock.
		wchar_t local_HH[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"HH", local_HH, sizeof(local_HH) );
		
		// Minutes with no leading zero for single-digit minutes.
		wchar_t local_t_m[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"m", local_t_m, sizeof(local_t_m) );
		
		// Minutes with leading zero for single-digit minutes.
		wchar_t local_t_mm[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"mm", local_t_mm, sizeof(local_t_mm) );
		
		// Seconds with no leading zero for single-digit seconds.
		wchar_t local_s[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"s", local_s, sizeof(local_s) );
		
		// Seconds with leading zero for single-digit seconds.
		wchar_t local_ss[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"ss", local_ss, sizeof(local_ss) );
		
		// One character time marker string, such as A or P.
		wchar_t local_t[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"t", local_t, sizeof(local_t) );
		
		// Multicharacter time marker string, such as AM or PM.
		wchar_t local_tt[ 64 ];
		::GetTimeFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"tt", local_tt, sizeof(local_tt) );

		// Get the day
		wchar_t	local_date[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, NULL, local_date, sizeof(local_date) );

		// Day of month as digits with no leading zero for single-digit days.
		wchar_t local_d[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"d", local_d, sizeof(local_d) ); 

		// Day of month as digits with leading zero for single-digit days.
		wchar_t local_dd[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"dd", local_dd, sizeof(local_dd) ); 

		// Day of week as a three-letter abbreviation.
		wchar_t local_ddd[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"ddd", local_ddd, sizeof(local_ddd) ); 

		// Day of week as its full name.
		wchar_t local_dddd[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"dddd", local_dddd, sizeof(local_dddd) ); 

		// Month as digits with no leading zero for single-digit months.
		wchar_t local_m[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"M", local_m, sizeof(local_m) ); 

		// Month as digits with leading zero for single-digit months.
		wchar_t local_mm[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"MM", local_mm, sizeof(local_mm) ); 

		// Month as a three-letter abbreviation. The function uses the LOCALE_SABBREVMONTHNAME value associated with the specified locale.
		wchar_t local_mmm[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"MMM", local_mmm, sizeof(local_mmm) ); 

		// Month as its full name. The function uses the LOCALE_SMONTHNAME value associated with the specified locale.
		wchar_t local_mmmm[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"MMMM", local_mmmm, sizeof(local_mmmm) ); 

		// Year as last two digits, but with no leading zero for years less than 10.
		wchar_t local_y[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"y", local_y, sizeof(local_y) ); 

		// Year as last two digits, but with leading zero for years less than 10.
		wchar_t local_yy[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"yy", local_yy, sizeof(local_yy) ); 

		// Year represented by full four or five digits, depending on the calendar used. Thai Buddhist and Korean calendars both have five digit years. 
		wchar_t local_yyyy[ 64 ];
		::GetDateFormatW( LOCALE_USER_DEFAULT, 0, &systemTime, L"yyyy", local_yyyy, sizeof(local_yyyy) ); 

		// Lock the value
		::EnterCriticalSection( &m_local_time_lock );

		// Check for a change
		bool val_changed  = false;
			 val_changed |= ( m_local_time = local_time );		
			 val_changed |= ( m_local_date = local_date );
			 val_changed |= ( m_d = local_d );	
			 val_changed |= ( m_dd = local_dd );	
			 val_changed |= ( m_ddd = local_ddd );	
			 val_changed |= ( m_dddd = local_dddd );	
			 val_changed |= ( m_m = local_m );	
			 val_changed |= ( m_mm = local_mm );	
			 val_changed |= ( m_mmm = local_mmm );	
			 val_changed |= ( m_mmmm = local_mmmm );	
			 val_changed |= ( m_y = local_y );	
			 val_changed |= ( m_yy = local_yy );	
			 val_changed |= ( m_yyyy = local_yyyy );
			 val_changed |= ( m_h = local_h );
			 val_changed |= ( m_hh = local_hh );
			 val_changed |= ( m_H = local_H );
			 val_changed |= ( m_HH = local_HH );
			 val_changed |= ( m_t_m = local_t_m );
			 val_changed |= ( m_t_mm = local_t_mm );
			 val_changed |= ( m_s = local_s );
			 val_changed |= ( m_ss = local_ss );
			 val_changed |= ( m_t = local_t );
			 val_changed |= ( m_tt = local_tt );

		 for( int i=0; i<24; i++ )
			 val_changed |= ( m_int_time[ i ] = international_time[ i ] );

		// Release the critical section
		::LeaveCriticalSection( &m_local_time_lock );

		// Trigger the changed message
		if ( val_changed ) 
			::SetEvent( m_key_changed );
	}
}