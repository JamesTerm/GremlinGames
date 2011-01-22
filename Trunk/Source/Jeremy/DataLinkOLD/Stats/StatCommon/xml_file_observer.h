#pragma once

#ifdef STATCOMMON_EXPORTS
#define STATCOMMON_API __declspec(dllexport)
#else
#define STATCOMMON_API __declspec(dllimport)
#endif

struct STATCOMMON_API xml_file_observer : public file_observer
{			// Constructor
			xml_file_observer( const wchar_t filename[], node observer[] );

			// Destructor
			~xml_file_observer( void );

private:	// The observer
			node	*m_p_node;

			// Overload this if you want to know when the file is changed
			virtual void file_changed( const __int64 time_changed );
};