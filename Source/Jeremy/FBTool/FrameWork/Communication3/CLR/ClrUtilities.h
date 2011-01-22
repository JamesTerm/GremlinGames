#pragma once

public ref class Utilities
{
public:
	// Determine how many messages a destination has pending to be processed.
	// A negative value indicates that an error occured, for instance that the
	// destination no longer exists.
	static int QueueDepth( System::String ^ name );

	// This will recover the heart-beat time for a particular server.
	// This represents the last ::QueryPeformanceCounter time that the server in
	// question was considered alive.
	static __int64 HeartBeat( System::String ^ name );


};