#ifdef BOARDWHITEWAY_EXPORTS
#define BOARDWHITEWAY_API __declspec(dllexport)
#else
#define BOARDWHITEWAY_API __declspec(dllimport)
#endif

// Stats Interface
// -------------------------------------------------------------
// A Stats DLL must implement these externs (Export C Functions)


// Start by calling this. It will return a handle that is "triggered"
// when a change has occurred to the stats. For instance, you could do a
// WaitForSingleEvent( handle, 100 ); on this event to see when stats have
// changed.
extern "C" BOARDWHITEWAY_API HANDLE			live_updates_start( void );

// Get a value from a key.  When TriCaster™ DataLink™ is done with this wchar_t pointer,
// it will pass it back through live_updates_release_value.
extern "C" BOARDWHITEWAY_API const wchar_t	*live_updates_get_value( const wchar_t key[] );

// Because you are returned a pointer that you do not own from
// live_updates_get_value, you will need to release it with this fcn. 
// TriCaster™ DataLink™ will not call another live_updates_get_value before it 
// releases the previous one.
extern "C" BOARDWHITEWAY_API void			live_updates_release_value( const wchar_t value[] );

// Get the next key that has changed.  When TriCaster™ DataLink™ is done with this wchar_t pointer,
// it will pass it back through live_updates_release_value.
extern "C" BOARDWHITEWAY_API const wchar_t	*live_updates_get_changed_key( void );

// Same as above, TriCaster™ DataLink™ will release the pointers before it reads
// the next changed key.
extern "C" BOARDWHITEWAY_API void			live_updates_release_changed_key( const wchar_t key[] );

// All done with stats
extern "C" BOARDWHITEWAY_API void			live_updates_end( HANDLE hHandle );
