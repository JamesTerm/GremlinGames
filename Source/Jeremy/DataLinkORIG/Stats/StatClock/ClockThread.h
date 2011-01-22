// ClockThread
// ---------------------------------------------
// A simple thread for updating the clock time using TriCaster™ DataLink™

struct ClockThread
{		// Constructor and destructor
		 ClockThread( void );
		~ClockThread( void );	

		// The handle to use
		HANDLE get_handle( void ) const;

		// Has the key actually changed since the last time throyugh
		const wchar_t *p_get_changed( void ) const;

		// Get the value
		const wchar_t *p_get_value( const wchar_t key[] ) const;

		// Release the value
		void release_value( void ) const;

private:
		// Thread Implementation.  SetEvent must be called
		// at some point in the thread for TriCaster™ to ask for Keys and Values.
		static void ClockThreadProc( void *p_this );
		void ClockThreadRunner( void );

		// Add an hour to a time
		static SYSTEMTIME add_time( const SYSTEMTIME& from, const int hours );
		
		// Do we want the thread to exit
		bool	m_thread_must_exit;	

		// Has the key been changed
		mutable bool m_val_changed;

		// The handles
		HANDLE	m_key_changed;
		HANDLE	m_thread_handle;

		struct	simple_entry
		{	wchar_t	m_str[ 64 ];
			mutable bool m_changed;

			simple_entry( void ) { m_str[ 0 ] = 0; m_changed = false; }
			const bool operator= ( const wchar_t *p_new ) { if ( 0 != ::wcscmp( p_new, m_str ) ) { ::wcscpy( m_str, p_new ); m_changed = true; return true; } return false; }
			operator const wchar_t* ( void ) const { m_changed = false; return m_str; }
			operator const bool ( void ) const { bool ret = m_changed; m_changed = false; return ret; }
		};

		simple_entry	m_local_time;

		simple_entry	m_int_time[ 24 ];

		simple_entry	m_local_date;
		simple_entry	m_d;	// Day of month as digits with no leading zero for single-digit days.
		simple_entry	m_dd;	// Day of month as digits with leading zero for single-digit days.
		simple_entry	m_ddd;	// Day of week as a three-letter abbreviation.
		simple_entry	m_dddd;	// Day of week as its full name.
		simple_entry	m_m;	// Month as digits with no leading zero for single-digit months.
		simple_entry	m_mm;	// Month as digits with leading zero for single-digit months.
		simple_entry	m_mmm;	// Month as a three-letter abbreviation. The function uses the LOCALE_SABBREVMONTHNAME value associated with the specified locale.
		simple_entry	m_mmmm;	// Month as its full name. The function uses the LOCALE_SMONTHNAME value associated with the specified locale.
		simple_entry	m_y;	// Year as last two digits, but with no leading zero for years less than 10.
		simple_entry	m_yy;	// Year as last two digits, but with leading zero for years less than 10.
		simple_entry	m_yyyy;	// Year represented by full four or five digits, depending on the calendar used. Thai Buddhist and Korean calendars both have five digit years. 

		simple_entry	m_h;	// Hours with no leading zero for single-digit hours; 12-hour clock.
		simple_entry	m_hh;	// Hours with leading zero for single-digit hours; 12-hour clock.
		simple_entry	m_H;	// Hours with no leading zero for single-digit hours; 24-hour clock.
		simple_entry	m_HH;	// Hours with leading zero for single-digit hours; 24-hour clock.
		simple_entry	m_t_m;	// Minutes with no leading zero for single-digit minutes.
		simple_entry	m_t_mm;	// Minutes with leading zero for single-digit minutes.
		simple_entry	m_s;	// Seconds with no leading zero for single-digit seconds.
		simple_entry	m_ss;	// Seconds with leading zero for single-digit seconds.
		simple_entry	m_t;	// One character time marker string, such as A or P.
		simple_entry	m_tt;	// Multicharacter time marker string, such as AM or PM.
		

		mutable CRITICAL_SECTION m_local_time_lock;	
};