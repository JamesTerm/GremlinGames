#pragma once

#ifdef STATFILEOBSERVER_EXPORTS
#define STATFILEOBSERVER_API __declspec(dllexport)
#else
#define STATFILEOBSERVER_API __declspec(dllimport)
#endif

// Stats Interface designed by Andrew

// Start by calling this. It will return a handle that is "triggered"
// when a change has occured to the stats. For instance, you could do a
// WaitForSingleEvent( handle, 100 ); on this event to see when stats have
// changed.
extern "C" STATFILEOBSERVER_API HANDLE live_updates_start( void );

// All done with stats.
extern "C" STATFILEOBSERVER_API void live_updates_end( HANDLE handle );

// Get the next key that has changed.
extern "C" STATFILEOBSERVER_API const wchar_t* live_updates_get_changed_key( void );

// Because you are returned a pointer that you do not own from live_updates_get_key, you
// will need to release it with this function. Do not call another live_updates_get_key
// until the key returned by the previous call has been released.
extern "C" STATFILEOBSERVER_API void live_updates_release_changed_key( const wchar_t *key );

// Get a value from a key.
extern "C" STATFILEOBSERVER_API const wchar_t* live_updates_get_value( const wchar_t *key );

// Same as above, you need to release the pointer before you can read the next changed value.
extern "C" STATFILEOBSERVER_API void live_updates_release_value( const wchar_t *value );
