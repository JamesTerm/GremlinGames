#pragma once
///OutputStream talks to this interface as a callback mechanism to process buffers and establish how frequent to process them.  The ideal
///object to implement this interface is the actual device which uses OutputStream to handle all the device details.  If the device has
///its own threading (e.g. DirectShow) then this should not be used but rather use OutStream_NotThreaded
class DEVICES_BASE_API OutStream_Interface
{
	public:
		virtual ~OutStream_Interface() {}
		///Fill the buffer with data... You can assume the format to render has already been verified to match the device client
		/// \return true if successful
		virtual bool FillBuffer(PBYTE DestBuffer,size_t BufferSizeInBytes,
			TimeStamp &DeviceTime ///< If your device has other devices upstream pass that time to the DeviceTime
			)=0;
		/// \return Simply return a meaningful name of this device (e.g. return L"MyTestDevice";)
		virtual const wchar_t *pGetDeviceName( void ) const=0;
		///If you need to pace your frequency, you can provide a waiting mechanism here so OutputStream thread can halt.
		///Otherwise, this can simply return (no delay is necessary)
		/// \see OutputStream::GetAbortThread()
		virtual void WaitForNextBuffer()=0;
		///Most likely you can ignore this. this will notify when a new buffer gets added *before* it actually gets added.
		///You can for example use this callback to initialize a devices output format (where it only does this on the first call)
		/// \return true if successful
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer ) 
			{	return true;
			}
		///If you are not working with raw devices then you can ignore this.  This is primarily only used for a raw device where the device client 
		///does not have a format to deduce the size. In this case you can specify a size for the client to use.
		virtual size_t GetBufferSize() 
			{	return 0;
			}
};

class InputStream;
class InStream_Interface
{
	public:
		///Notify parent that it is closing to take appropriate action
		virtual void InStreamClosing(void *ThisChild)=0;
		///This callback may be useful to obtain information like incoming sample rate (it is optional)
		///At this time... this is only called for successful buffers to add
		virtual void AddBufferToTBCList( Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage )=0;
		///Let the client know that we detected a format change found when a buffer attempts to be added to the TBC queue
		///This gives the client a chance to refresh variables set when first sensing the device
		virtual void In_FormatChangeDetected()=0;
		///e.g. return new BufferType(pInputStream);
		virtual BufferInterface *InStream_Interface_GetNewBuffer(InputStream *pInputStream)=0;
		///This ensures the client will delete in the same place where it has been created (avoiding heap allocation errors)
		///This also gives the client the responsibility to delete to the true type (since BufferInterface does not have a virtual destructor)
		///So you *must* dynamically cast this buffer passed in to the true type that was allocated!
		virtual void InStream_Interface_FreeBuffer(BufferInterface *buffer)=0;
};
