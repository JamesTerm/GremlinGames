#pragma once

///This is a base implementation of a device where it wraps up all the details of handling buffers that get added from the device client.
///The client provides an OutStream_Interface which lets this know where and how frequent to submit new buffers to be processed
/// \see VideoOutputStream and AudioOutputStream for devices which use these kind of streams
class DEVICES_BASE_API OutputStream : public DeviceInterface
{
	public:
		OutputStream (OutStream_Interface *pParent);
		virtual ~OutputStream();

		///This has been exposed mainly for OutStream_Interface::WaitForNextBuffer().  If the OutStream_Interface is using a type of waiting
		///mechanism it should also be monitoring the abort thread status.
		const bool GetAbortThread( void ) const; //returns m_AbortThread

	protected:
		/// \note derived classes should implement this if they contain format information
		virtual bool DoFormatsMatch(const BufferInterface *Buffer)
			{ return true;
			}

	private: //From Device Interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer );
		virtual void FlushAllBuffers( void );
		///returns parents device name
		virtual const wchar_t *pGetDeviceName( void ) const;

	private:
		/// This is the thread callback
		friend FrameWork::Threads::thread<OutputStream>;
		void operator() ( const void* );

		FrameWork::Threads::thread<OutputStream>	*m_pThread;

		/// My parent
		OutStream_Interface * const m_pParent;

		/// This is a critical section only used for locking buffers during flushing
		FrameWork::Threads::critical_section	m_FlushQueue_Lock;

		/// The list of buffers that currently need to be filled in
		FrameWork::Threads::critical_section						m_ListOfBuffers_Lock;

		std::queue<BufferInterface*>							m_ListOfBuffers;

		///Will set an event when the Queue has added a buffer
		FrameWork::Threads::event m_QueueActivitySignal;

		///This safely unwinds the thread (used mainly for destructor)
		bool m_AbortThread;
};

///This is a base implementation of a device where it wraps up all the details of device negotiations to transfer the buffers.
///In this non-threaded form, you manage your own threading and waiting mechanism.
class DEVICES_BASE_API OutStream_NotThreaded : public DeviceInterface
												
{
	public:
		OutStream_NotThreaded(const wchar_t *pDeviceName = L"OutStream_NotThreaded");
		virtual ~OutStream_NotThreaded();

		///This will transfer the buffer data to any buffers in queue
		///If this is called when no buffers are available the data will be dropped
		void TransferBuffer(const byte *Buffer,size_t BufferSizeInBytes,
			TimeStamp Time=(__int64)-1 ///Note: Default implementation will use query performance count
			);

		///Use this to determine the amount of queue depth
		///This has been exposed so that you can have control of how to wait 
		///(e.g. threading 2 streams the parent can wait only when both streams are empty)
		size_t GetDepth() const;

		///This provides notification when buffers have been added to the device queue
		FrameWork::Threads::event &GetEventHandle() const;
	protected:
		///Derived classes should implement this if they contain format information
		virtual bool DoFormatsMatch(const BufferInterface *Buffer)
			{ return true;
			}

		//OutStream type members----------------------------------------
		/// This is a critical section only used for locking buffers during flushing
		FrameWork::Threads::critical_section	m_FlushQueue_Lock;

		/// The list of buffers that currently need to be filled in
		mutable FrameWork::Threads::critical_section				m_ListOfBuffers_Lock;
		std::queue<Devices::BufferInterface*>			m_ListOfBuffers;

		///Override this if there is some specific way to copy the data (e.g. video using pitch)
		virtual void *OutStream_MemCopy(void *Dest,const void *Source,size_t SizeInBytes,Devices::BufferInterface *pBufferToFill)
			{ return memcpy(Dest,Source,SizeInBytes);
			}

		///A derived class may wish to call these directly to fill the buffers and avoid a memcpy
		Devices::BufferInterface *GetBufferToFill();
		///This will perform a memcpy if the source and destination buffers are different; in either case, it will then
		///release the buffer thereby letting the device client know that the memory has been filled and ready to process
		void TransferBuffer(const byte *SourceBuffer,
			Devices::BufferInterface *pBufferToFill,
			size_t BufferSizeInBytes,
			TimeStamp Time=(__int64)-1 ///Note: Default implementation will use query performance count
			);

		///This safely unwinds the thread (used mainly for destructor)
		bool m_AbortThread;
	private:	//From Device Interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer );
		virtual void FlushAllBuffers( void );
		virtual const wchar_t *pGetDeviceName( void ) const {return m_pDeviceName;}

		const wchar_t * const m_pDeviceName; //we can change this easily to be dynamically changed if necessary
		mutable FrameWork::Threads::event m_EventHandle;
};


const size_t c_DefaultTBCQueueDepth=8; //note the default is somewhat lean, you may wish to compute a higher value

///This is a base implementation of a device client where it handles all the negotiations with a device, and manages buffer allocations.  This will
///provide buffers that have been filled and ready to process.
class DEVICES_BASE_API InputStream
{
	public:
		
		InputStream(
			InStream_Interface *pParent, ///< The InStream_Interface which usually is the owner of this class
			DeviceInterface *pDevice,	///< The name of the device for which this client will send its buffers to (this is const)
			 /// \param DefaultTBCQueueDepth for a InputStream which works with a passive Device (e.g. the AudioTBC) use a low value like 2
			size_t DefaultTBCQueueDepth=c_DefaultTBCQueueDepth,
			const wchar_t *pDeviceClientName = L"InputStream" ///< The name of this device client (mainly for debug purposes)
			);
		virtual ~InputStream();

		void AdvanceTBC( void );

		///The owning class will need to call this (this way it can pick the best time to start the streaming)
		/// Start the queue... will define the buffer size, and start adding new buffers to the device queue
		bool StartTBCQueue(
			size_t NoBuffersToUse=-1,		///<Specify queue size; -1 will use the default value set in the constructor
			size_t NoBuffersToStart=-1  ///< Will add x number to device queue... -1 will revert to the value in the first parameter
			);

		/// Finish the queue
		void FinishTBCQueue( void );

		InStream_Interface *GetParent() const;
		DeviceInterface *GetDevice() const;
		FrameWork::Threads::critical_section &GetTBCQueue_Lock(); 
		std::vector<BufferInterface *>	&GetTBCQueue();
		bool GetFlushingQueue() const;

		/// This should only be called by any derived implementation of BufferInterface::ReleaseBufferData() when they are received
		void AddBufferToTBCList( BufferInterface *pBuffer , const wchar_t *pErrorMessage );
	protected:
		///Derived classes should implement this if they contain format information
		virtual bool DoFormatsMatch(const DeviceInterface *Device,const BufferInterface *Buffer)
			{ return true;
			}

		InStream_Interface * const m_pParent;

	private:
		/// This will put a frame "in flight" onto the device. If the device
		/// format does not match, it is replaced with a buffer that does match.
		/// if you specify "NULL" here, then a totally new frame is created
		bool AddBufferToDeviceQueue( BufferInterface *pBufferToAdd );

		DeviceInterface * const m_pDevice;
		const size_t m_DefaultTBCQueueDepth;		

		size_t m_MaxTBCQueueDepth; ///<for video this was around 8 frames... I'll make mine twice the value of the lead time
		size_t m_BufferCount; ///<used to manage how many buffers to add in an insert situation
		//**** TBC Variables
		FrameWork::Threads::critical_section m_TBCQueue_Lock;
		std::vector<BufferInterface *>	m_TBCQueue; /// \note this is a vector because some clients may need to access the next element before advancing the queue
		const std::wstring m_DeviceClientName;
		bool m_FlushingQueue;
};
