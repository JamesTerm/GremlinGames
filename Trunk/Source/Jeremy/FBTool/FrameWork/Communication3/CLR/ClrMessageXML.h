#pragma once

public ref class Xml
{
public:
	// Constructor
	Xml( const char		xml_data[] );
	Xml( const wchar_t	xml_data[] );
	Xml( System::String ^ xml_data );

	// Constructor allowing you to fill in messages in place.
	// Technically this should allow for more efficient creation
	// of messages since it avoids all memory copies. The down-side
	// is that you need to know the memory length ahead of time, although
	// if a message is to long then there is no harm.
	Xml( const size_t max_size );

	// Constructor with pointer to unmanagedXML
	Xml( const unmanaged_FC3_xml::message * ux );

	// Destructor
	~Xml( void );

	// Finalizer
	!Xml( void );

	// Send it to a destination. 
	//
	// Sending values aynchronously is slightly faster, however there is a very tiny
	// chance that the message would not be received. The chance in practice is less
	// than one in a billion in most realistic scenarios. The only case where a message
	// would fail to be sent is if this structure was deleted very soon after the send.
	// if it exists even for a milli-second beyond this then the message is guaranteed
	// to be sent to the destination.
	//
	// The return value is whether sending this message succeeded. This most likely would
	// indicate that the destination does not exist. A destination would always receive
	// a message.
	//
	bool Send( const System::String ^ dst, const bool async );

	// This will broadcast a single message to a large number of destinations
	// The returned value is the number of destinations that it was known sent too.
	// If the messages are sent asyncronously then the return value is zero.
	int Broadcast( const bool async );

	// Get access to the actual data of the message. You can use this to fill in data (or
	// get data that you filled in using the constructor.)
	char* GetDataA( void );
	wchar_t* GetDataW( void );
	System::String ^ GetDataString( void );

	// Get the maximum allocated size for this message
	const size_t Size( void );

	// Recover the number of outstanding destintions that have not yet processed this
	// message. The name is quite a mouthfull, but I cannot think of a shorter one that
	// is equally descriptive.
	// 
	// This is a completely atomic operation, and returns very fast.
	// 
	// One important thing to note is that if a host dies for some reason, then this
	// will not automatically reflect that.
	//
	int NoOutstandingPendingProcessings( void );

	// Parse this message
	bool Parse( FrameWork::xml::node *p_node );

	// Reference counting support
	long AddRef( void );
	long Release( void );
	
private:
	unmanaged_FC3_xml::message * m_pUnmanagedXml;
	bool m_ownsUnmanagedXml;
};