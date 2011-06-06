#pragma once

//********************************************************************************************************************************************
//********************************************************************************************************************************************

struct IPeakLevelListener;	// Predecl

// I compute, but I'm not really a true "computer".  I run on a computer though.
class DEVICES_AUDIO_PEAKLEVELS_API PeakLevelComputer	: public Devices::Audio::FormatInterface	// I store some audio format shiznizzle
{
	// Interface
public:

	// C'tor/D'tor
		// NOTE: Only class clients should use this C'tor.  Derived classes should use the protected C'tor.
	PeakLevelComputer(Devices::Audio::DeviceInterface *pUpstreamDevice, IPeakLevelListener *pFanOfMan, 
						Devices::Audio::FormatInterface *pFormatSpec=NULL, bool bInline=true, bool blow_priority=true);	// If inline doesn't do processing on sep thread
	virtual ~PeakLevelComputer( void );

	// Devices::Audio::FormatInterface
	virtual size_t GetSampleRate( void ) const;
	virtual size_t GetNoChannels( void ) const;
	virtual eSampleFormat GetSampleFormat( void ) const;
	virtual bool GetIsBufferInterleaved( void ) const;

	// Mine all mine...
	virtual void operator() ( const void* );	// My worker thread function that does something useful w/ a buffer after it's been filled
	void ReleaseBufferData_CB(Devices::Audio::BufferInterface *pBuffer, const wchar_t *pErrorMessage);

	// Implementation
protected:

	// C'tor for derived class use.  Clever bit of trickery to rearrange order of params in C'tor signature for differentiation.
	PeakLevelComputer(IPeakLevelListener *pFanOfMan, Devices::Audio::DeviceInterface *pUpstreamDevice, Devices::Audio::FormatInterface *pFormatSpec=NULL, bool bInline=true);
	// These dudes are not thread safe.  Must be called on same thread!!!
	// Not that big a deal since typically called in most derived class C'tor and D'tor respectively.
	virtual void Startup(bool blow_priority=true);	// Should be called in most derived class C'tor
	virtual void Shutdown( void );					// Should be called in most derived class D'tor

	class Buffer : public Devices::Audio::BufferInterface
	{
	public:
		Buffer(PeakLevelComputer *pParent);
		virtual ~Buffer( void );
		// Devices::Audio::BufferInterface
		virtual size_t GetSampleRate( void ) const;
		virtual size_t GetNoChannels( void ) const;
		virtual eSampleFormat GetSampleFormat( void ) const;
		virtual bool GetIsBufferInterleaved( void ) const;
		virtual void *pGetBufferData(size_t *pMemorySize);
		virtual void ReleaseBufferData(const TimeStamp *pFrameTime, const wchar_t *pErrorMessage);
		virtual const TimeStamp& GetTimeStamp( void ) const;
	private:
		PeakLevelComputer *m_pOwner;
		void *m_pBuff;
		size_t m_cbBuff;
		size_t m_cbData;
		Devices::TimeStamp m_Time;
	};
	
	// If not doing inline processing, called on my worker thread!!!
	// Else called on upstream device thread the releases buffer!!!
	virtual void DoProcess(Buffer *pFilledBuffer);
	virtual void AnalizeAudioSamples(const void *pBuffer, size_t cbData);	// Just called by DoProcess

	// Data
protected:

	// Config and state
	const bool m_bInline;	// Am I doing threaded processing or inline processing??? Once set properly, I'm set for good!!!
	bool m_bRunning;		// Am I between a Startup and Shutdown???

	Devices::Audio::DeviceInterface* m_pUpstreamDevice;
	IPeakLevelListener* const m_pPLListener;

	const size_t m_SampleRate;
	const size_t m_cChannels;
	const eSampleFormat m_SampleFormat;
	const bool m_bInterleaved;

	float *m_pPeaksAndValleys;

	std::vector<Buffer*> m_ListOfAllBuffers;
	typedef std::vector<Buffer*>::iterator AllBuffersIter;
	// My worker thread that does something useful w/ a buffer after it's been filled
	FrameWork::Threads::thread<PeakLevelComputer> *m_pThread;
	// My queue of filled buffers (I add to this once ReleaseBufferData is called by upstream device)
	FrameWork::Threads::critical_section m_QueueLock;
	std::queue<Buffer*> m_Queue;

	// Illegal
private:

	//PeakLevelComputer(const PeakLevelComputer&) { asssert(false); }
	void operator = (const PeakLevelComputer&) { assert(false); }
};


//********************************************************************************************************************************************
//********************************************************************************************************************************************