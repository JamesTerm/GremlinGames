#include "Stdafx.h"
#include "RenderTree_SWOnly.h"
#include "AVOutputConfig.h"

//******************************************************************************************************************
//******************************************************************************************************************

#include <malloc.h>
#include <atlbase.h>
#include <KS.H>
#include <ksmedia.h>
#include <Dsound.h>
#pragma comment (lib, "dsound")

//******************************************************************************************************************
namespace RenderTree_SWOnly
{
	// DirectSound default playback device GUID {DEF00000-9C6D-47ED-AAF1-4DDA8F2B5C03}
	//DSDEVID_DefaultPlayback
	const GUID My_DSDEVID_DefaultPlayback = { 0xdef00000, 0x9c6d, 0x47ed, { 0xaa, 0xf1,  0x4d,  0xda,  0x8f,  0x2b,  0x5c,  0x03 } };
}

//******************************************************************************************************************

using namespace RenderTree_SWOnly;

//******************************************************************************************************************

// Turn this on to use QueryPerformanceCounter for wait in between sound card clock polls.  The provides for
// more consistent wait times.  However on application context switches there is a very long lag.  Turning this off reduces
// the frequency of waits that take toooooo long causing stuttering.
#define Use_QPF_CompileSwitch	1

// Do I want to control params e.g. volume
#define SoundCard_CtrlParams

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SoundCard : public ISoundCard,
#ifdef SoundCard_CtrlParams
					public ISoundCard_CtrlParams,
#endif
					protected WorkerThread
{
	// Interface
public:

	SoundCard(void);
	~SoundCard(void);

	// ISoundCard
	virtual bool StartPlaying(const size_t sampleRate, const size_t cChannels,	// Always 32 bit FP 
							const size_t frameRateNum, const size_t frameRateDen, 
							const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, 
							bool bTBC);
	virtual bool StopPlaying(void);
	virtual bool IsPlaying(void) const;
	virtual bool QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime);
	virtual bool AddCallback(IPlayedFrameSoundCallback *pCB);
	virtual bool RemoveCallback(IPlayedFrameSoundCallback *pCB);
	virtual void SignalStartFlush(void);
	virtual void Flush(void);
	virtual void SignalEndFlush(void);

	// ISoundCard_CtrlParams 
#ifdef SoundCard_CtrlParams
	virtual void SetVolume(LONG lVal);
	virtual LONG GetVolume(void) const;
#endif

	// Mine all mine...
	__forceinline bool ClearCallbacks(void);
	__forceinline bool SwapCallbacks(tList<IPlayedFrameSoundCallback*> &l_Callbacks);
	static __forceinline void SwapRightLeftFloatChannelsInplace(void *pBuff, size_t cAudioSamples);

	// Implementation
protected:

	struct AudioFrameBufferInfo
	{
		void *m_pBuffer;
		size_t m_cSamples;
		size_t m_cbBuff;
		size_t m_frameNo;

		// Dynamically filled in by worker thread...

		DWORD m_dwStartWritePosn;	// Offset into sound card buffer where writing this frame of audio started

		DWORD m_dwNextWriteRemainder;	// Bytes of this particular buffer left to write

		bool m_bPlayedFirstField;		// Only play field0 1x
	};

	// Facilitator funcs
protected:

	// WorkerThread
	virtual void ThreadProcessor_Start(void);
	virtual void ThreadProcessor(void);	// This is the main thread the fills the circular sound buffer
	virtual void ThreadProcessor_Write_InlinePlay(DWORD cbLockOffset, DWORD cbLockSize);
	virtual void ThreadProcessor_Write_TBCPlay(DWORD cbLockOffset, DWORD cbLockSize);

	__forceinline void FillBuffer(byte* &pWriteStart, DWORD &cbWrite, const DWORD dwWriteOffset, AudioFrameBufferInfo *pWriteInfo);
	__forceinline void FillBuffer(void *const pWriteStart, const DWORD cbWrite);

	__forceinline bool Plunge(void);

	__forceinline void CallAllCallbacks(const size_t frameNo, bool bAll=true);

	// Data
protected:

	const bool m_bForce_PCM16;	// Turn this on to force 16 bit PCM integer samples

	double m_msClockCycles;	// Clock cycles per millisecond
	
	bool m_bDupBuffs;
	bool m_bNotifySilencePlayed;
	bool m_bTBC;

	WAVEFORMATEX m_WaveFormat;

	CComPtr<IDirectSound8> m_pDS;
	
	CComPtr<IDirectSoundBuffer> m_pPrimaryBuffer;
	CComPtr<IDirectSoundBuffer> m_pBuffer2;
	size_t m_cbBuffer;
	
	size_t m_cbSamplesPerFrame;
	size_t m_cbSamplesPerField;
	double m_msQuarterFrameTime;
	DWORD m_msQuarterFrameTimeSleep;
	__int64 m_cClocksPerQuarterFrameTime;

	tList<AudioFrameBufferInfo, true> m_Queue;
	VariableBlock m_PreQueueLock;
	tList<AudioFrameBufferInfo, true> m_PreQueue;

	tList<IPlayedFrameSoundCallback*> m_Callbacks;		// Callbacks when using the sound card clock

	DWORD m_LastPlayCursor;
	DWORD m_cbWrittenSincePlay;

	/////////////////////////
	LONG m_lFlushFlag;

	LONG m_lCachedVolumeForFlush;		// I mute myself during a flush session
	bool m_bDoingFlushSession;			// When true, callbacks are turned off
	/////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SoundCard::SoundCard(void) :
m_bForce_PCM16(ConfigSettings::GetInstance().AudioOut_PCM16()),
m_bDupBuffs(true), m_bNotifySilencePlayed(false), m_bTBC(false), m_cbBuffer(0), m_cbSamplesPerFrame(0), m_cbSamplesPerField(0), m_msQuarterFrameTime(0.0), 
m_msQuarterFrameTimeSleep(0), m_cClocksPerQuarterFrameTime(0), m_cbWrittenSincePlay(0), m_LastPlayCursor(0),
m_lFlushFlag(0)
, m_lCachedVolumeForFlush(DSBVOLUME_MAX), m_bDoingFlushSession(false)
{
	// Get the clock cycles per ms
	__int64 clockFreq = { 0 };
	BOOL bResult = ::QueryPerformanceFrequency((LARGE_INTEGER*)&clockFreq);	// Variable that receives the current performance-counter frequency, in counts per second
	assert(bResult);
	m_msClockCycles = static_cast<double>(clockFreq);
	m_msClockCycles /= 1000.0;	// now counts per ms

	::memset(&m_WaveFormat, 0, sizeof(m_WaveFormat));

	// Create DirectSound device
	HRESULT hr = ::DirectSoundCreate8(&My_DSDEVID_DefaultPlayback, &m_pDS, NULL);
	assert(SUCCEEDED(hr));

	// Set priority cooperation level
	assert(::IsWindow(::NewTek_GetGlobalParentHWND()));
	hr = m_pDS->SetCooperativeLevel(::NewTek_GetGlobalParentHWND(), DSSCL_PRIORITY);
	assert(SUCCEEDED(hr));
}

SoundCard::~SoundCard(void)
{
	this->StopPlaying();
}

bool SoundCard::StartPlaying(const size_t sampleRate, const size_t cChannels, const size_t frameRateNum, const size_t frameRateDen,
							const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, bool bTBC)
{
	// Stop playing
	// Retreive primary and 2ndary buffers
	// Set both to preferred output format
	// Clear 2ndary buffer w/ silence
	// Play
	// Start my buffer filling thread
	////////////////////////////////////////////////////////////////////////////////////////////////////

	this->StopPlaying();

	if (m_bForce_PCM16)
		m_bDupBuffs = true;
	else
		m_bDupBuffs = bDupBuffs;

	m_bNotifySilencePlayed = bNotifySilencePlayed;
	m_bTBC = bTBC;

	WAVEFORMATEX format = { 0 };
	format.nChannels = static_cast<WORD>(cChannels);
	format.nSamplesPerSec = static_cast<DWORD>(sampleRate);
	if (m_bForce_PCM16)
		format.wBitsPerSample = 16;
	else
		format.wBitsPerSample = sizeof(float)*8;	// number of bits per sample of mono data

	format.cbSize = 22;	// of xtra info (for WAVEFORMATEXTENSIBLE the cbSize member must be at least 22)
	format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;//WAVE_FORMAT_PCM;

	//If wFormatTag is WAVE_FORMAT_PCM or WAVE_FORMAT_EXTENSIBLE, 
	// nBlockAlign must be equal to the product of nChannels and wBitsPerSample divided by 8 (bits per byte). 
	format.nBlockAlign = format.nChannels * format.wBitsPerSample/8;
	
	// If wFormatTag is WAVE_FORMAT_PCM, nAvgBytesPerSec should be equal to the product of nSamplesPerSec and nBlockAlign
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	WAVEFORMATEXTENSIBLE formatEx = { 0 };
	formatEx.Format = format;
	if (m_bForce_PCM16)
		formatEx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	else
		formatEx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

	formatEx.Samples.wValidBitsPerSample = format.wBitsPerSample;
	formatEx.dwChannelMask = SPEAKER_FRONT_LEFT | 
                             SPEAKER_FRONT_RIGHT | 
                             SPEAKER_BACK_LEFT |
                             SPEAKER_BACK_RIGHT;
                                // Quadraphonic = 0x00000033

	/////////////////////////////////////////////////////////////////////////////////////////////////////

	//Obtain primary buffer
	DSBUFFERDESC buffDesc = { 0 };
	buffDesc.dwSize = sizeof(DSBUFFERDESC);
	buffDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	HRESULT hr = m_pDS->CreateSoundBuffer(&buffDesc, &m_pPrimaryBuffer, NULL);
	hr = m_pPrimaryBuffer->SetFormat((LPWAVEFORMATEX)&formatEx);
	assert(SUCCEEDED(hr));
	// The method succeeds even if the hardware does not support the requested format; DirectSound sets the buffer to the closest supported format.

#ifdef _DEBUG
	{
		DWORD dwSize = 0;
		HRESULT hr = m_pPrimaryBuffer->GetFormat(NULL, 0, &dwSize);

		WAVEFORMATEX *pFormat = (WAVEFORMATEX*)::_alloca(dwSize);
		hr = m_pPrimaryBuffer->GetFormat(pFormat, dwSize, &dwSize);
		WAVEFORMATEXTENSIBLE *pFormatEx = (WAVEFORMATEXTENSIBLE*)(pFormat);

		int iDebug = 11;
	}
#endif

	////////////////////////////////////////////////////////////////////////////////////////////////////
	double secondsPerFrame = (double)frameRateDen/(double)frameRateNum;
	double cSamplesPerFrame = (double)sampleRate * secondsPerFrame;

	m_msQuarterFrameTime = secondsPerFrame * 250.0;
	m_cClocksPerQuarterFrameTime = static_cast<__int64>(m_msClockCycles*m_msQuarterFrameTime);
	m_msQuarterFrameTimeSleep = static_cast<DWORD>(m_msQuarterFrameTime);
	m_cbSamplesPerFrame = format.nBlockAlign * ((size_t)::ceil(cSamplesPerFrame));
	m_cbSamplesPerField = m_cbSamplesPerFrame >> 1;
	m_cbBuffer = cFrameQueueHint * m_cbSamplesPerFrame;

	buffDesc.dwFlags = 	DSBCAPS_STATIC |				// In the hardware if possible
						DSBCAPS_GLOBALFOCUS	 |	
						DSBCAPS_GETCURRENTPOSITION2 |	// We are a new product use the best accurate code
						DSBCAPS_CTRLPOSITIONNOTIFY;
#ifdef SoundCard_CtrlParams
	buffDesc.dwFlags |= DSBCAPS_CTRLVOLUME;
#endif
	buffDesc.dwBufferBytes = static_cast<DWORD>(m_cbBuffer);
	buffDesc.lpwfxFormat = (LPWAVEFORMATEX)&formatEx;
	buffDesc.guid3DAlgorithm = DS3DALG_DEFAULT;

	hr = m_pDS->CreateSoundBuffer(&buffDesc, &m_pBuffer2, NULL);
	assert(SUCCEEDED(hr));

	////////////////////////////////////////////////////////////////////////////////////////////////////

	if (SUCCEEDED(hr) && m_pBuffer2)	// One of the more likely scenarios on wimpy sound cards is that it can't do DSBCAPS_GETCURRENTPOSITION2
	{
		void *pBuff = NULL, *pTemp = NULL;
		DWORD dwTemp1 = 0, dwTemp2 = 0;
		hr = m_pBuffer2->Lock(0, buffDesc.dwBufferBytes, &pBuff, &dwTemp1, &pTemp, &dwTemp2, DSBLOCK_ENTIREBUFFER);
		assert(SUCCEEDED(hr) && (dwTemp1==buffDesc.dwBufferBytes));
		::memset(pBuff, 0, dwTemp1);
		hr = m_pBuffer2->Unlock(pBuff, dwTemp1, pTemp, dwTemp2);
		assert(SUCCEEDED(hr));

		m_LastPlayCursor = 0;
		m_cbWrittenSincePlay = 0;

		hr = m_pBuffer2->Play(0, 0, DSBPLAY_LOOPING);
		assert(SUCCEEDED(hr));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////

	if (SUCCEEDED(hr) && m_pBuffer2)	// One of the more likely scenarios on wimpy sound cards is that it can't do DSBCAPS_GETCURRENTPOSITION2
	{
		// Store the wave format
		m_WaveFormat = format;

		this->StartThread();
		if (bTBC)
			this->SetThreadPriority(THREAD_PRIORITY_HIGHEST);
		else
			this->SetThreadPriority(THREAD_PRIORITY_TIME_CRITICAL);
		return true;
	}
	else
	{
		this->StopPlaying();
		return false;
	}
}

bool SoundCard::StopPlaying(void)
{
	if (m_pBuffer2)
	{
		HRESULT hr = m_pBuffer2->Stop();
		assert(SUCCEEDED(hr));
	}
	this->StopThread();

	m_pPrimaryBuffer = NULL;	// Release primary buffer
	m_pBuffer2 = NULL;			// Release 2ndary buffer

	// Delete all the queued sound buffers (Worker thread stopped, I have exclusive access to m_Queue)
	m_PreQueueLock.Block();	// Since we have it
	if (m_bDupBuffs)
	{
		for (size_t i=0; i<m_Queue.NoItems(); i++)
			::NewTek_free(m_Queue[i].m_pBuffer);
		for (size_t j=0; j<m_PreQueue.NoItems(); j++)
			::NewTek_free(m_PreQueue[j].m_pBuffer);
	}
	m_Queue.DeleteAll();
	m_PreQueue.DeleteAll();
	m_PreQueueLock.UnBlock();

	// Empty out my wave format
	::memset(&m_WaveFormat, 0, sizeof(m_WaveFormat));

	return true;
}

bool SoundCard::IsPlaying(void) const
{
	// Just check for non-NULL value on one of my data members
	if (!m_pBuffer2)
		return false;
	else
		return true;
	/*
	DWORD dwStatus = 0;
	HRESULT hr = m_pBuffer2->GetStatus(&dwStatus);
	assert(SUCCEEDED(hr));
	return (DSBSTATUS_PLAYING & dwStatus);
	*/
}

// For reference
/*
struct AudioFrameBufferInfo
	{
		void *m_pBuffer;
		size_t m_cSamples;
		size_t m_cbBuff;
		size_t m_frameNo;

		// Dynamically filled in by worker thread...

		DWORD m_dwStartWritePosn;	// Offset into sound card buffer where writing this frame of audio started

		DWORD m_dwNextWriteRemainder;	// Bytes of this particular buffer left to write

		bool m_bPlayedFirstField;		// Only play field0 1x
	};
*/
bool SoundCard::QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime)
{
	// YUCK: For historical reasons we have the channels in this order: FR, FL, BR, BL
	SoundCard::SwapRightLeftFloatChannelsInplace(pBuff, cAudioSamples);
	// TODO: I could maybe even optimize more by doing this explicitly for Floating point out audio
	// and adding the shuffle to the loop that converts each sample to PCM audio.

	size_t cbBuff = m_WaveFormat.nBlockAlign * cAudioSamples;
	if (m_bDupBuffs)
	{
		void *pCopiedBuff = ::NewTek_malloc(cbBuff);
		if (m_bForce_PCM16)
			//All the global converter function prototypes defined here
			int iRetVal = ::newtek_aud_Convert_QuadFP_QuadShort_XMM((short*)pCopiedBuff, (float*)pBuff, (int)cAudioSamples);
		else
			::memcpy(pCopiedBuff, pBuff, cbBuff);

		pBuff = pCopiedBuff;
	}
	
	AudioFrameBufferInfo info = { pBuff, cAudioSamples, cbBuff, frameNo, 
									static_cast<DWORD>(-1)/*TEMP*/, static_cast<DWORD>(cbBuff)/*Haven't written anything yet*/, false/*Haven't played anything yet*/ };
	FunctionBlock threadSafe(m_PreQueueLock);
	m_PreQueue.Add(info);

	return true;
}
//static __forceinline 
void SoundCard::SwapRightLeftFloatChannelsInplace(void *pBuff, size_t cAudioSamples)
{
	/*
	Selects four specific single-precision, floating-point values from a and b, based on the mask i.
	__m128 _mm_shuffle_ps(__m128 a , __m128 b , int i );
	*/

	__m128 *pIter = (__m128*)(pBuff);
	const __m128* const pEnd = pIter + cAudioSamples;
	while (pIter < pEnd)
	{
		*pIter = _mm_shuffle_ps(*pIter, *pIter, _MM_SHUFFLE(2,3,0,1));
		pIter++;
	}
}

bool SoundCard::AddCallback(IPlayedFrameSoundCallback *pCB)
{	
	// For thread safety, my worker thread can't be accessing this list. Also Block thread from starting during this call.
	FunctionBlock threadSafe(static_cast<VariableBlock*>(this));
	if (this->GetRunning())
	{
		assert(false);
		return false;
	}

	assert(pCB);
	m_Callbacks.Add(pCB);

	return true;
}

bool SoundCard::RemoveCallback(IPlayedFrameSoundCallback *pCB)
{	
	// For thread safety, my worker thread can't be accessing this list. Also Block thread from starting during this call.
	FunctionBlock threadSafe(static_cast<VariableBlock*>(this));
	if (this->GetRunning())
	{
		assert(false);
		return false;
	}

	assert(pCB);
	return m_Callbacks.Delete(pCB);
}

bool SoundCard::ClearCallbacks(void)
{	
	// For thread safety, my worker thread can't be accessing this list. Also Block thread from starting during this call.
	FunctionBlock threadSafe(static_cast<VariableBlock*>(this));
	if (this->GetRunning())
	{
		assert(false);
		return false;
	}

	m_Callbacks.DeleteAll();
	return true;
}

bool SoundCard::SwapCallbacks(tList<IPlayedFrameSoundCallback*> &l_Callbacks)
{
	// For thread safety, my worker thread can't be accessing this list. Also Block thread from starting during this call.
	FunctionBlock threadSafe(static_cast<VariableBlock*>(this));
	if (this->GetRunning())
	{
		assert(false);
		return false;
	}

	m_Callbacks.SwapFrom(&l_Callbacks);
	return true;
}

void SoundCard::SignalStartFlush(void)
{
	m_lCachedVolumeForFlush = this->GetVolume();	// Cache the volume
	this->SetVolume(DSBVOLUME_MIN);					// Silence the sound
	m_bDoingFlushSession = true;					// Now I'm really doing a flush
}

void SoundCard::SignalEndFlush(void)
{
	this->SetVolume(m_lCachedVolumeForFlush);	// Reset the volume
	m_bDoingFlushSession = false;				// Not doing a flush session anymore
}

void SoundCard::Flush(void)
{
	// Wait for thread processor to finish current functional iteration and temporarily suspend it's next iteration 
	//		so that I have exclusive access to the queues and the circular sound buffer
	// Flush pre queue
	// Flush queue
	// Fill w/ silence

	if (this->IsPlaying())
	{
		::InterlockedExchange(&m_lFlushFlag, 1L);
		while (0 != ::InterlockedCompareExchange(&m_lFlushFlag, 0L, 0L))
			::Sleep(1);
	}
	else
	{
		assert(false);
	}
}

#ifdef SoundCard_CtrlParams
void SoundCard::SetVolume(LONG lVal)
{
	if (!m_pBuffer2)
		return;

	m_pBuffer2->SetVolume(lVal);
}
LONG SoundCard::GetVolume(void) const
{
	if (!m_pBuffer2)
		return DSBVOLUME_MAX;

	LONG retVal = DSBVOLUME_MAX;
	m_pBuffer2->GetVolume(&retVal);
	return retVal;
}
#endif

void SoundCard::ThreadProcessor_Start(void)
{
	DWORD dwPlayCursor=0, dwWriteCursor=0;
	HRESULT hr = m_pBuffer2->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
	assert(SUCCEEDED(hr));

	// Update sound buffer play cursor
	m_LastPlayCursor = dwPlayCursor;

	// Sleep 1/4 frame rate (Wait a tiny bit before polling GetCurrentPositions again)
	::Sleep(m_msQuarterFrameTimeSleep);
}

void SoundCard::ThreadProcessor(void)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// TODO: If no more frames to play and all buffers have been filled w/ silence, 
		// Wait for event that frame have been added to my queue
	
	// I have frames in my queue, GetCurrentPositions
	// Fill buffer from last play position to new play position
	// After finished filling buffer w/ 1 entire frame of audio (silence if nothing in queue), play a previous frame of audio
	// Sleep 1/4 frame time

	DWORD dwPlayCursor=0, dwWriteCursor=0;
	HRESULT hr = m_pBuffer2->GetCurrentPosition(&dwPlayCursor, &dwWriteCursor);
	assert(SUCCEEDED(hr));

#if Use_QPF_CompileSwitch
	// I want to limit the frequency of GetCurrentPosition calls, 
	// but still give myself enough head room to fill the circular sound buffer on time
	__int64 clockCyclesStartTime = { 0 };
	BOOL bResult = ::QueryPerformanceCounter((LARGE_INTEGER*)&clockCyclesStartTime);
	assert(bResult);
	__int64 clockCyclesEndTime = clockCyclesStartTime + m_cClocksPerQuarterFrameTime;
#endif

	// I'm going to recycle buffer portion that was played since last check
	DWORD cbLockOffset=0, cbLockSize=0;
	if (m_LastPlayCursor < dwPlayCursor)
	{
		cbLockOffset = m_LastPlayCursor;
		cbLockSize = dwPlayCursor - m_LastPlayCursor;
	}
	else if (dwPlayCursor < m_LastPlayCursor)
	{
		// I always lock @ the play cursor
		cbLockOffset = m_LastPlayCursor;
		cbLockSize = static_cast<DWORD>(m_cbBuffer) - (m_LastPlayCursor-dwPlayCursor);
	}
	//************************
	// Diagnostics
#ifdef _DEBUG
	double msFreq = (static_cast<double>(cbLockSize)) / (static_cast<double>(m_WaveFormat.nBlockAlign * 48));	// Because 48000 hz
	static char g_szDbgMsg[2048] = { 0 };
	/*
	::sprintf(g_szDbgMsg, "\n mwatkins - GetCurrentPosition frequency = %f", msFreq);
	::OutputDebugString(g_szDbgMsg);
	*/
	if (msFreq > 33.0)	// Approximately time of 1 NTSC frame in ms
	{
		//::sprintf(g_szDbgMsg, "\n mwatkins - Too much time has gone by between sound card play cursor polls!!! %f \n", msFreq);
		//::OutputDebugString(g_szDbgMsg);
	}
#endif
	//************************

	if (cbLockSize)
	{
		// Copy pre queue to queue
		m_PreQueueLock.Block();
		m_Queue.Add(m_PreQueue.NoItems(), m_PreQueue.GetPtr());
		m_PreQueue.DeleteAll();
		m_PreQueueLock.UnBlock();
#ifdef _DEBUG
		// TODO: Could potentially be unbounded memory usage up in here!!!
		/*
		char szDbgMsg[256] = { 0 };
		::sprintf(szDbgMsg, "\n mwatkins - Size of audio queue = %d \n", m_Queue.NoItems());
		::OutputDebugString(szDbgMsg);
		*/
#endif
		
		if (m_bTBC)
			this->ThreadProcessor_Write_TBCPlay(cbLockOffset, cbLockSize);
		else
			this->ThreadProcessor_Write_InlinePlay(cbLockOffset, cbLockSize);

		// Update sound buffer play cursor
		m_LastPlayCursor = dwPlayCursor;
	}

	//************************
	// Diagnostics
#ifdef _DEBUG
	/*
	::sprintf(g_szDbgMsg, "\n mwatkins - Audio queue depth = %d \n", m_Queue.NoItems());
	::OutputDebugString(g_szDbgMsg);
	*/
#endif
	//************************

#if Use_QPF_CompileSwitch
	// How many clock cycles in 2 millisecond
	__int64 clocksPer2MS = static_cast<__int64>(m_msClockCycles*2.0);
	__int64 clockCyclesEndTimeDelta = clockCyclesEndTime - clocksPer2MS;

	// Sleep 1/4 frame rate (Wait a tiny bit before polling GetCurrentPositions again)
	__int64 clockCyclesCurrentTime = { 0 };
	do
	{
		//************************
		// Plunge if needed which flushes for waiting thread
		if (0 != ::InterlockedCompareExchange(&m_lFlushFlag, 1L, 1L))
		{
			this->Plunge();
			::InterlockedExchange(&m_lFlushFlag, 0L);
		}
		//************************

		bool bResult = ::QueryPerformanceCounter((LARGE_INTEGER*)&clockCyclesCurrentTime);
		assert(bResult);

		if (clockCyclesCurrentTime < clockCyclesEndTimeDelta)
			::Sleep(1);	// OK to sleep 1 here
		else
			::Sleep(0);	// Can't sleep...it'll fuck up my timing
	} 
	while (clockCyclesCurrentTime <= clockCyclesEndTime);
#else
	// Old way of doing things up in here
	::Sleep(m_msQuarterFrameTimeSleep);
#endif
}

void SoundCard::ThreadProcessor_Write_InlinePlay(DWORD cbLockOffset, DWORD cbLockSize)
{
	// I'm writing m_Queue[1]
	size_t cQueueItems = m_Queue.NoItems();	// This way I minimize lock of queue
	AudioFrameBufferInfo *pPlayingInfo = cQueueItems>0 ? &(m_Queue[0]) : NULL;
	AudioFrameBufferInfo *pWritingInfo = cQueueItems>1 ? &(m_Queue[1]) : NULL;
	AudioFrameBufferInfo *pNextWritingInfo = cQueueItems>2 ? &(m_Queue[2]) : NULL;

	// Lock the sound card buffer (get access to it)
	const byte* pDestBufferBegin = NULL;

	void *pWriteCursor1 = NULL, *pWriteCursor2 = NULL;
	DWORD dwWrite1 = 0, dwWrite2 = 0;
	HRESULT hr = m_pBuffer2->Lock(cbLockOffset, cbLockSize, &pWriteCursor1, &dwWrite1, &pWriteCursor2, &dwWrite2, 0);
	assert(SUCCEEDED(hr) && cbLockSize==(dwWrite1+dwWrite2));

	// TODO: Don't need to figure this every f'ing time!!!
	pDestBufferBegin = pWriteCursor2 ? reinterpret_cast<byte*>(pWriteCursor2) : reinterpret_cast<byte*>(pWriteCursor1)-cbLockOffset;

	byte* pDestBuffArr[] = { reinterpret_cast<byte*>(pWriteCursor1), reinterpret_cast<byte*>(pWriteCursor2) };	// Gets incremented by FillBuffer below
	DWORD cbWriteArr[] = { dwWrite1, dwWrite2 };	// Gets decremented by FillBuffer below
	AudioFrameBufferInfo* pSrcArr[] = { pWritingInfo, pNextWritingInfo };

	// Important Sanity check
#ifdef _DEBUG
	/*
	if (pWritingInfo && pNextWritingInfo)
	{
		assert(cbLockSize <= (pWritingInfo->m_cbBuff + pNextWritingInfo->m_cbBuff));
	}
	*/
#endif

	// Fill both buffers...
	// First fill 1st chuck of buffer w/ combo of (pWritingInfo, pNextWritingInfo, and/or silence)
	// Then fill 2nd chuck of buffer w/ combo remainder of (pWritingInfo, pNextWritingInfo, and/or silence)

	// Fill w/ writing info
	// Fill w/ next writing info
	for (size_t i=0; i<2; i++)
	{
		for (size_t j=0; j<2; j++)
			this->FillBuffer(pDestBuffArr[i], cbWriteArr[i], pDestBuffArr[i]-pDestBufferBegin, pSrcArr[j]);
	}

	// Fill both buffers w/ silence
	for (size_t k=0; k<2; k++)
		this->FillBuffer(pDestBuffArr[k], cbWriteArr[k]);

	// Unlock the sound card buffer (we don't need it anymore.  It has been filled)
	hr = m_pBuffer2->Unlock(pWriteCursor1, dwWrite1, pWriteCursor2, dwWrite2);
	assert(SUCCEEDED(hr));

	// Increment the bytes written to the sound card buffer
	m_cbWrittenSincePlay += cbLockSize;

	// If finished filling a frame, play previous frame
	if (pPlayingInfo || m_bNotifySilencePlayed)
	{	static bool bPlayedFirstField = false; 

		// New support for interlaced frames (fielded)
		byte finishedWritingField = 0;
		if (pWritingInfo)
		{
			if (0 == pWritingInfo->m_dwNextWriteRemainder)
				finishedWritingField = 2;	// I've written a frame
			else if (m_cbSamplesPerField >= pWritingInfo->m_dwNextWriteRemainder)
				finishedWritingField = 1;	// I've written a field
		}
		else 
		{
			if (m_cbWrittenSincePlay >= m_cbSamplesPerFrame)	// Have written 1 frame of silence since done writing frames?
			{
				/*
				#ifdef _DEBUG
				::sprintf(g_szDbgMsg, "\n mwatkins - Samples written delta = %d", m_cbWrittenSincePlay-m_cbSamplesPerFrame);
				::OutputDebugString(g_szDbgMsg);
				#endif
				*/

				finishedWritingField = 2;	// I've written a frame
			}
			else if (m_cbWrittenSincePlay >= m_cbSamplesPerField)	// Have written 1 field of silence since done writing frames?
			{
				finishedWritingField = 1;	// I've written a field
			}
		}

		if (2 == finishedWritingField)
		{	
			if (pPlayingInfo)
			{	/*assert(pPlayingInfo->m_bPlayedFirstField);*/	bPlayedFirstField = false;	// I've written a frame of audio. Next silence should be first field (Field0).
				this->CallAllCallbacks(pPlayingInfo->m_frameNo, true);	// Display both fields

				if (m_bDupBuffs)
					::NewTek_free(m_Queue[0].m_pBuffer);
				m_Queue.DeleteEntryInOrder(0);

				// Reset bytes written to sound card buffer
				m_cbWrittenSincePlay = 0;

				//::OutputDebugString("\nmwatkins-Played frame sound Field1\n");
			}
			else
			{	/*assert(bPlayedFirstField);*/	bPlayedFirstField = false;	// I've written a frame of silence.  Next silence should be first field (Field0).

				this->CallAllCallbacks(static_cast<size_t>(-1), true);	// Display both fields

				// Reset bytes written to sound card buffer
				m_cbWrittenSincePlay = 0;
			}
		}
		else if (1 == finishedWritingField)
		{
			if (pPlayingInfo)
			{	//::OutputDebugString("\nmwatkins-Checking for frame sound Field0 playing\n");
				if (!pPlayingInfo->m_bPlayedFirstField)
				{
					pPlayingInfo->m_bPlayedFirstField = true;	bPlayedFirstField = true;	// I've written a frame field.  Next frame AND silence should be 2nd field (Field1)

					this->CallAllCallbacks(pPlayingInfo->m_frameNo, false);	// Display 1st field

					//::OutputDebugString("\nmwatkins-Played frame sound Field0\n");
				}
			}
			else if (!bPlayedFirstField)
			{	bPlayedFirstField = true;	// I've written a field of silence.

				this->CallAllCallbacks(static_cast<size_t>(-1), false);	// Display 1st field
			}
		}

		// This only works for progressive frames (But it works well!!!)
		/*
		bool bFinishedWritingFrame = false;

		if (pWritingInfo)
		{
			if (0 == pWritingInfo->m_dwNextWriteRemainder)
				bFinishedWritingFrame = true;
		}
		else if (m_cbWrittenSincePlay >= m_cbSamplesPerFrame)	// Have written 1 frame of silence since done writing frames?
		{
			bFinishedWritingFrame = true;
		}

		if (bFinishedWritingFrame)
		{
			this->CallAllCallbacks(pPlayingInfo->m_frameNo);

			if (m_bDupBuffs)
				::NewTek_free(m_Queue[0].m_pBuffer);
			m_Queue.DeleteEntryInOrder(0);

			// Reset bytes written to sound card buffer
			m_cbWrittenSincePlay = 0;
		}
		*/
	}
}

// TODO:
// This function needs to return a next write cursor that is aligned to the end point of the previously written field.
// This data needs to be maintained separately from the last play cursor data.
void SoundCard::ThreadProcessor_Write_TBCPlay(DWORD cbLockOffset, DWORD cbLockSize)
{	static bool bPlayedFirstField = false;

	// Lock the sound card buffer (get access to it)
	const byte* pDestBufferBegin = NULL;

	void *pWriteCursor1 = NULL, *pWriteCursor2 = NULL;
	DWORD dwWrite1 = 0, dwWrite2 = 0;
	HRESULT hr = m_pBuffer2->Lock(cbLockOffset, cbLockSize, &pWriteCursor1, &dwWrite1, &pWriteCursor2, &dwWrite2, 0);
	assert(SUCCEEDED(hr) && cbLockSize==(dwWrite1+dwWrite2));

	// TODO: Don't need to figure this every f'ing time!!!
	pDestBufferBegin = pWriteCursor2 ? reinterpret_cast<byte*>(pWriteCursor2) : reinterpret_cast<byte*>(pWriteCursor1)-cbLockOffset;

	byte* pDestBuffArr[] = { reinterpret_cast<byte*>(pWriteCursor1), reinterpret_cast<byte*>(pWriteCursor2) };	// Gets incremented by FillBuffer below
	DWORD cbWriteArr[] = { dwWrite1, dwWrite2 };	// Gets decremented by FillBuffer below

	// Fill both buffers...
	// First fill 1st chuck of buffer w/ combo of (pWritingInfo, pNextWritingInfo, and/or silence)
	// Then fill 2nd chuck of buffer w/ combo remainder of (pWritingInfo, pNextWritingInfo, and/or silence)
	for (size_t i=0; i<2; i++)
	{
		size_t j=0;
		while (cbWriteArr[i] && j<m_Queue.NoItems())
		{
			AudioFrameBufferInfo *pWritingInfo = &(m_Queue[j++]);
			this->FillBuffer(pDestBuffArr[i], cbWriteArr[i], pDestBuffArr[i]-pDestBufferBegin, pWritingInfo);
		}
	}
	
	// Fill both buffers w/ silence
	for (size_t k=0; k<2; k++)
		this->FillBuffer(pDestBuffArr[k], cbWriteArr[k]);

	// Unlock the sound card buffer (we don't need it anymore.  It has been filled)
	hr = m_pBuffer2->Unlock(pWriteCursor1, dwWrite1, pWriteCursor2, dwWrite2);
	assert(SUCCEEDED(hr));

	//*************************************************************************************
	// Somefing new

	m_cbWrittenSincePlay += cbLockSize;
	
	//DWORD &dwLeftToPlay = m_cbWrittenSincePlay;
	long dwLeftToPlay = static_cast<long>(m_cbWrittenSincePlay);
	long l_cbSamplesPerField = static_cast<long>(m_cbSamplesPerField);

	while (m_Queue.NoItems() && dwLeftToPlay>l_cbSamplesPerField)
	{
		if (!m_Queue[0].m_bPlayedFirstField)	// I have a field left to play and I haven't played field 0
		{
			if (m_cbSamplesPerField >= m_Queue[0].m_dwNextWriteRemainder)						// To handle zero sized audio buffers
			//if (m_Queue[0].m_cbBuff-m_Queue[0].m_dwNextWriteRemainder > m_cbSamplesPerField)	// I can't play what I haven't written
			{
				this->CallAllCallbacks(m_Queue[0].m_frameNo, false);	// Display 1st field
				m_Queue[0].m_bPlayedFirstField = bPlayedFirstField = true;

				dwLeftToPlay -= l_cbSamplesPerField;
			}
			else
			{
				break;
			}
		}
		else if (!m_Queue[0].m_dwNextWriteRemainder)
		{
			this->CallAllCallbacks(m_Queue[0].m_frameNo, true);	// Display 2st field
			bPlayedFirstField = false;

			dwLeftToPlay += l_cbSamplesPerField;
			dwLeftToPlay -= static_cast<long>(m_Queue[0].m_cbBuff);

			if (m_bDupBuffs)
				::NewTek_free(m_Queue[0].m_pBuffer);
			m_Queue.DeleteEntryInOrder(0);
		}
		else
		{
			break;
		}
	}
	
	if (m_bNotifySilencePlayed)
	{
		while (dwLeftToPlay > l_cbSamplesPerField)
		{
			this->CallAllCallbacks(static_cast<size_t>(-1), !bPlayedFirstField);
			bPlayedFirstField = !bPlayedFirstField;
			dwLeftToPlay -= l_cbSamplesPerField;
		}
	}

	m_cbWrittenSincePlay = dwLeftToPlay>0 ? static_cast<DWORD>(dwLeftToPlay) : 0;

	//*************************************************************************************
}

void SoundCard::FillBuffer(byte* &pWriteStart, DWORD &cbWrite, const DWORD dwWriteOffset, AudioFrameBufferInfo *pWriteInfo)
{
	if (!pWriteStart || 0==cbWrite || !pWriteInfo || 0==pWriteInfo->m_dwNextWriteRemainder)
		return;

	if (static_cast<DWORD>(-1) == pWriteInfo->m_dwStartWritePosn)	// First time writing this frame of audio???
		pWriteInfo->m_dwStartWritePosn = dwWriteOffset;

	if (cbWrite > pWriteInfo->m_dwNextWriteRemainder)	// More to write than I have left
	{
		byte *pSrc = (reinterpret_cast<byte*>(pWriteInfo->m_pBuffer)) + (pWriteInfo->m_cbBuff - pWriteInfo->m_dwNextWriteRemainder);
		::memcpy(pWriteStart, pSrc, pWriteInfo->m_dwNextWriteRemainder);	// Write all I've got
		pWriteStart += pWriteInfo->m_dwNextWriteRemainder;	// Increment the dest pointer # of bytes I just wrote
		cbWrite -= pWriteInfo->m_dwNextWriteRemainder;		// Decrement the # of bytes left to write
		pWriteInfo->m_dwNextWriteRemainder = 0;				// Nullify the bytes left in this frame of audio
		// This is where I would NULL-out a src write from ptr
	}
	else
	{
		byte *pSrc = (reinterpret_cast<byte*>(pWriteInfo->m_pBuffer)) + (pWriteInfo->m_cbBuff - pWriteInfo->m_dwNextWriteRemainder);
		::memcpy(pWriteStart, pSrc, cbWrite);					// Write what I can
		pWriteInfo->m_dwNextWriteRemainder -= cbWrite;			// Decrement # of bytes left in this frame of audio
		// This is where I would increment a src write from ptr
		cbWrite = 0;											// No mass to write in this buffer
		pWriteStart = NULL;	// Paranoid.  This buffer is filled.  It shouldn't be written to anymore
	}
}

void SoundCard::FillBuffer(void *const pWriteStart, const DWORD cbWrite)
{
	if (!pWriteStart || 0==cbWrite)
		return;

#ifdef _DEBUG
	//::OutputDebugString("\n mwatkins - Had to write marklar!!! \n");
#endif
	::memset(pWriteStart, 0, cbWrite);
}

bool SoundCard::Plunge(void)
{
	// Fill w/ as much silence silence and as soon as possible
	// Flush pre queue
	// Flush queue

	// Just stop playing and 0 out the buffer (simpler, but less elegant)
	{
		HRESULT hr = m_pBuffer2->Stop();
		assert(SUCCEEDED(hr));

		void *pBuff = NULL, *pTemp = NULL;
		DWORD dwTemp1 = 0, dwTemp2 = 0;
		hr = m_pBuffer2->Lock(0, static_cast<DWORD>(m_cbBuffer), &pBuff, &dwTemp1, &pTemp, &dwTemp2, DSBLOCK_ENTIREBUFFER);
		assert(SUCCEEDED(hr) && (dwTemp1==m_cbBuffer));
		::memset(pBuff, 0, dwTemp1);
		hr = m_pBuffer2->Unlock(pBuff, dwTemp1, pTemp, dwTemp2);
		assert(SUCCEEDED(hr));

		m_LastPlayCursor = 0;
		m_cbWrittenSincePlay = 0;

		hr = m_pBuffer2->Play(0, 0, DSBPLAY_LOOPING);
		assert(SUCCEEDED(hr));
	}

	// Delete all the queued sound buffers (Worker thread stopped, I have exclusive access to m_Queue)
	m_PreQueueLock.Block();	// Since we have it
	if (m_bDupBuffs)
	{
		for (size_t i=0; i<m_Queue.NoItems(); i++)
			::NewTek_free(m_Queue[i].m_pBuffer);
		for (size_t j=0; j<m_PreQueue.NoItems(); j++)
			::NewTek_free(m_PreQueue[j].m_pBuffer);
	}
	m_Queue.DeleteAll();
	m_PreQueue.DeleteAll();
	m_PreQueueLock.UnBlock();

	return true;
}

void SoundCard::CallAllCallbacks(const size_t frameNo, bool bAll)
{	
	if (m_bDoingFlushSession)
		return;	// No callbacks during a flush, I muted myself anyways!!!
	for (size_t i=0; i<m_Callbacks.NoItems(); i++)
		m_Callbacks[i]->PlayingFrameSound(frameNo, bAll);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace RenderTree_SWOnly
{

BOOL __stdcall DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
	GUID *pGUID = reinterpret_cast<GUID*>(lpContext);

	if (NULL != ::strstr(lpcstrDescription, "Video Toaster"))
	{
		if (pGUID)
			*pGUID = *lpGuid;
		return FALSE;
	}
	return TRUE;
}

GUID GetToasterSoundCardGUID(void)
{
	// This is the GUID on my machine for the 1st device using the emulated driver
	//{C2AD1800-B243-11CE-A8A4-00AA006C4500}
	//const GUID ToasterCardAudioGUID = { 0xC2AD1800, 0xB243, 0x11CE, { 0xA8, 0xA4,  0x00,  0xAA,  0x00,  0x6C,  0x45,  0x00 } };

	GUID retVal = GUID_NULL;
	::DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumCallback, &retVal);

	return retVal;
}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define Str_RequiredHardware_SoundCard_NoToaster		TL_GetString("SpeedEdit::ErrorMsg::NoToaterSoundCard", "Please go to Control Panel and choose a default audio playback device other than the Video Toaster card.")
#define Str_RequiredHardware_SoundCard_NotPresentMsg	TL_GetString("SpeedEdit::ErrorMsg::NoSoundCard", "Running NewTek SpeedEdit requires a sound card.")
#define Str_RequiredHardware_SoundCard_NotCompatible	TL_GetString("SpeedEdit::ErrorMsg::BadSoundCard", "SpeedEdit is not supported on your sound card.")
#define Str_RequiredHardware_SoundCard_Emulated			TL_GetString("SpeedEdit::ErrorMsg::Emulated", "SpeedEdit can not run using DirectSound emulation.  Please verify that hardware sound acceleration is enabled.")

//static 
const char* TheSoundCard::VerifyRequiredHardware(void)
{
	// At least check that the default playback device isn't currently the toaster card
	{
		// Get the toaster sound card GUID.  Returns GUID_NULL if not found.
		GUID toasterGUID = GetToasterSoundCardGUID();

		// Get the GUID of the default playback device being used
		GUID currentGUID = GUID_NULL;
		HRESULT hr = ::GetDeviceID(&My_DSDEVID_DefaultPlayback, &currentGUID);
		if (FAILED(hr) || ::IsEqualGUID(GUID_NULL, currentGUID))	// Is there a default playback device???
		{
			assert(false);
			return Str_RequiredHardware_SoundCard_NotPresentMsg;
		}

		// If I found a toaster card audio device and it's set as the default playback device...
		if (::IsEqualGUID(toasterGUID, currentGUID))
			return Str_RequiredHardware_SoundCard_NoToaster;
	}

	// At least check that there is a sound card please!!!
	{
		CComPtr<IDirectSound8> pDS;
		if (FAILED(::DirectSoundCreate8(&My_DSDEVID_DefaultPlayback, &pDS, NULL)))
			return Str_RequiredHardware_SoundCard_NotPresentMsg;

		// Get the caps of the device
		DSCAPS soundCardCaps = { 0 };
		soundCardCaps.dwSize = sizeof(DSCAPS);
		if (FAILED(pDS->GetCaps(&soundCardCaps)))
			return Str_RequiredHardware_SoundCard_NotPresentMsg;
		if (DSCAPS_EMULDRIVER & soundCardCaps.dwFlags)
			return Str_RequiredHardware_SoundCard_Emulated;
	}

	// Don't have a valid parent hwnd yet!!!
	/*
	// Try to start playing for a minute second
	{
		const char *szRetVal = NULL;

		try
		{
			TheSoundCard::Get().StartPlaying(48000, 4, 30000, 1001, 8, false, true, ConfigSettings::GetInstance().AudioOut_TBC());
		}
		catch(...)
		{
			szRetVal = Str_RequiredHardware_SoundCard_NotCompatible;
		}

		TheSoundCard::Get().StopPlaying();

		return szRetVal;
	}
	*/

	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PlaybackScheduler : public ISoundCard,
							public ISoundCardClocking
{
	// Interface
public:

	// C'tor/D'tor
	PlaybackScheduler(void);
	virtual ~PlaybackScheduler(void);

	// ISoundCard
	virtual bool StartPlaying(const size_t sampleRate, const size_t cChannels,	// Always 32 bit FP 
							const size_t frameRateNum, const size_t frameRateDen, 
							const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, 
							bool bTBC);
	virtual bool StopPlaying(void);
	virtual bool IsPlaying(void) const;
	virtual bool QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime);
	virtual bool AddCallback(IPlayedFrameSoundCallback *pCB);
	virtual bool RemoveCallback(IPlayedFrameSoundCallback *pCB);
	virtual void SignalStartFlush(void) { }
	virtual void Flush(void);
	virtual void SignalEndFlush(void) { }

	// ISoundCardClocking
	virtual void InitPluginHost(StandardOutput::PluginHost *pPluginHost);
	virtual void ReleasePluginHost(StandardOutput::PluginHost *pPluginHost);
	virtual void RefClock_PlayFrame(const size_t frameNo, ePlayInst playInst);
	virtual void EnablePluginClock(bool bEnable) { }	// Nothing todo...plug-in clock must be enabled for this impl

	// Implementation
protected:

	// Data
protected:

	StandardOutput::PluginHost *m_pPluginClockHost;
	bool m_bPlaying;
	tList<IPlayedFrameSoundCallback*> m_Callbacks;		// Callbacks when using the sound card clock
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Make this class more robust (e.g. Thread safety around callback list etc.)

PlaybackScheduler::PlaybackScheduler(void)
: m_pPluginClockHost(NULL), m_bPlaying(false)
{
}

PlaybackScheduler::~PlaybackScheduler(void)
{
}

bool PlaybackScheduler::StartPlaying(const size_t sampleRate, const size_t cChannels,
									const size_t frameRateNum, const size_t frameRateDen, 
									const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, 
									bool bTBC)
{
	m_bPlaying = true;
	return true;
}

bool PlaybackScheduler::StopPlaying(void)
{
	m_bPlaying = false;
	return true;
}

bool PlaybackScheduler::IsPlaying(void) const
{
	return m_bPlaying && m_pPluginClockHost;
}

bool PlaybackScheduler::QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime)
{	assert(m_pPluginClockHost);	// Precondition
	
	m_pPluginClockHost->FrameReadyForOutput(startTime, frameNo);
	return true;
}

bool PlaybackScheduler::AddCallback(IPlayedFrameSoundCallback *pCB)
{
	// Add the callback to my list
	assert(pCB);
	m_Callbacks.Add(pCB);
	return true;
}

bool PlaybackScheduler::RemoveCallback(IPlayedFrameSoundCallback *pCB)
{
	// Remove the callback from my list
	assert(pCB);
	return m_Callbacks.Delete(pCB);
}

void PlaybackScheduler::Flush(void)
{
	if (m_pPluginClockHost)
		m_pPluginClockHost->FlushOutputQueue();
	else
		assert(false);
}

void PlaybackScheduler::InitPluginHost(StandardOutput::PluginHost *pPluginHost)
{	
	// TODO: Must be called before I'm started playing
	assert(!m_pPluginClockHost && pPluginHost);	// Init function, only called 1x
	m_pPluginClockHost = pPluginHost;
}

void PlaybackScheduler::ReleasePluginHost(StandardOutput::PluginHost *pPluginHost)
{	
	// TODO: Must be called after playing is stopped
	assert(pPluginHost);
	if (pPluginHost == m_pPluginClockHost)
		m_pPluginClockHost = NULL;
}

void PlaybackScheduler::RefClock_PlayFrame(const size_t frameNo, ePlayInst playInst)
{
	if (ePlayInst_EntireFrame == playInst)
	{
		for (size_t i=0; i<m_Callbacks.NoItems(); i++)
		{
			// Display both fields
			m_Callbacks[i]->PlayingFrameSound(frameNo, false);
			m_Callbacks[i]->PlayingFrameSound(frameNo, true);
		}
	}
	else
	{
		bool bField1 = (ePlayInst_Field1==playInst);
		for (size_t i=0; i<m_Callbacks.NoItems(); i++)
			m_Callbacks[i]->PlayingFrameSound(frameNo, bField1);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class PlaybackScheduler_DirectSound : public PlaybackScheduler,
#ifdef SoundCard_CtrlParams
										public ISoundCard_CtrlParams
#endif
{
	// Interface
public:

	PlaybackScheduler_DirectSound(void);
	virtual ~PlaybackScheduler_DirectSound(void);

	// ISoundCard
	virtual bool StartPlaying(const size_t sampleRate, const size_t cChannels,	// Always 32 bit FP 
							const size_t frameRateNum, const size_t frameRateDen, 
							const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, 
							bool bTBC);
	virtual bool StopPlaying(void);
	virtual bool IsPlaying(void) const;
	virtual bool QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime);
	virtual bool AddCallback(IPlayedFrameSoundCallback *pCB);
	virtual bool RemoveCallback(IPlayedFrameSoundCallback *pCB);
	virtual void SignalStartFlush(void);
	virtual void Flush(void);
	virtual void SignalEndFlush(void);

	// ISoundCard_CtrlParams 
#ifdef SoundCard_CtrlParams
	virtual void SetVolume(LONG lVal);
	virtual LONG GetVolume(void) const;
#endif

	// ISoundCardClocking
	virtual void InitPluginHost(StandardOutput::PluginHost *pPluginHost);
	virtual void ReleasePluginHost(StandardOutput::PluginHost *pPluginHost);
	virtual void RefClock_PlayFrame(const size_t frameNo, ePlayInst playInst);
	virtual void EnablePluginClock(bool bEnable);

	// Implementation
protected:

	// Data
protected:

	SoundCard m_sndCard;

	VariableBlock m_EnablePluginClockLock;
	bool m_bPluginClockEnabled;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Make this class more robust (e.g. Thread safety around callback list etc.)

PlaybackScheduler_DirectSound::PlaybackScheduler_DirectSound(void) : m_bPluginClockEnabled(false)
{
}

PlaybackScheduler_DirectSound::~PlaybackScheduler_DirectSound(void)
{
	this->StopPlaying();
}

bool PlaybackScheduler_DirectSound::StartPlaying(const size_t sampleRate, const size_t cChannels,
									const size_t frameRateNum, const size_t frameRateDen, 
									const size_t cFrameQueueHint, bool bDupBuffs, bool bNotifySilencePlayed, 
									bool bTBC)
{
	m_bPlaying = m_sndCard.StartPlaying(sampleRate, cChannels, frameRateNum, frameRateDen, cFrameQueueHint, bDupBuffs, bNotifySilencePlayed, bTBC);
	return m_bPlaying;
}

bool PlaybackScheduler_DirectSound::StopPlaying(void)
{
	m_bPlaying = false;
	return m_sndCard.StopPlaying();
}

bool PlaybackScheduler_DirectSound::IsPlaying(void) const
{
	if (!m_pPluginClockHost)
		return false;
	return m_sndCard.IsPlaying();
}

bool PlaybackScheduler_DirectSound::QueueFrameAudio(const size_t frameNo, const size_t cAudioSamples, void *pBuff, double startTime)
{	
	bool bSuccess = __super::QueueFrameAudio(frameNo, cAudioSamples, pBuff, startTime);	assert(bSuccess);
	return m_sndCard.QueueFrameAudio(frameNo, cAudioSamples, pBuff, startTime);
}

bool PlaybackScheduler_DirectSound::AddCallback(IPlayedFrameSoundCallback *pCB)
{
	//////////////////////////////////////////////
	// Block EnablePluginClock
	// If plug-in clock is enabled
		// Call inherited method
	// Else
		// Call m_sndCard method
	//////////////////////////////////////////////
	
	FunctionBlock threadSafe(m_EnablePluginClockLock);
	if (m_bPluginClockEnabled)
		return __super::AddCallback(pCB);
	else
		return m_sndCard.AddCallback(pCB);
}

bool PlaybackScheduler_DirectSound::RemoveCallback(IPlayedFrameSoundCallback *pCB)
{
	//////////////////////////////////////////////
	// Block EnablePluginClock
	// If plug-in clock is enabled
		// Call inherited method
	// Else
		// Call m_sndCard method
	//////////////////////////////////////////////
	
	FunctionBlock threadSafe(m_EnablePluginClockLock);
	if (m_bPluginClockEnabled)
		return __super::RemoveCallback(pCB);
	else
		return m_sndCard.RemoveCallback(pCB);
}

void PlaybackScheduler_DirectSound::SignalStartFlush(void)
{
	m_sndCard.SignalStartFlush();
	__super::SignalStartFlush();
}

void PlaybackScheduler_DirectSound::SignalEndFlush(void)
{	
	m_sndCard.SignalEndFlush();
	__super::SignalEndFlush();
}

void PlaybackScheduler_DirectSound::Flush(void)
{
	m_sndCard.Flush();	// Must flush sound card first
	__super::Flush();	// Call inherited method
}

#ifdef SoundCard_CtrlParams
void PlaybackScheduler_DirectSound::SetVolume(LONG lVal)
{
	m_sndCard.SetVolume(lVal);
}
LONG PlaybackScheduler_DirectSound::GetVolume(void) const
{
	return m_sndCard.GetVolume();
}
#endif

void PlaybackScheduler_DirectSound::InitPluginHost(StandardOutput::PluginHost *pPluginHost)
{	
	__super::InitPluginHost(pPluginHost);
}

void PlaybackScheduler_DirectSound::ReleasePluginHost(StandardOutput::PluginHost *pPluginHost)
{	
	__super::ReleasePluginHost(pPluginHost);
}

void PlaybackScheduler_DirectSound::RefClock_PlayFrame(const size_t frameNo, ePlayInst playInst)
{	// TODO: Sanity checks???
	
	__super::RefClock_PlayFrame(frameNo, playInst);
}

void PlaybackScheduler_DirectSound::EnablePluginClock(bool bEnable)
{
	//////////////////////////////////////////////
	// Block AddCallback and RemoveCallback
	// Swap m_sndCard::m_Callbacks and __super::m_Callbacks
	//////////////////////////////////////////////
	
	FunctionBlock threadSafe(m_EnablePluginClockLock);
	m_sndCard.SwapCallbacks(m_Callbacks);
	m_bPluginClockEnabled = bEnable;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//static
ISoundCard& TheSoundCard::Get(void)
{
	static PlaybackScheduler_DirectSound g_SoundCard;
	return g_SoundCard;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//static 
ISoundCard* TheSoundCard::NewInst(bool bUseDirectSound)
{
	if (bUseDirectSound)
		return new PlaybackScheduler_DirectSound;
	else
		return new PlaybackScheduler;
}

//static 
void TheSoundCard::DeleteInst(ISoundCard* pInst)
{
	if (!pInst)
		return;

	PlaybackScheduler *pScheduler = dynamic_cast<PlaybackScheduler*>(pInst);
	if (pScheduler)
	{
		delete pScheduler;
		return;
	}

	SoundCard *pSndCard = dynamic_cast<SoundCard*>(pInst);
	if (pSndCard)
	{
		delete pSndCard;
		return;
	}

	assert(false);	// If I get here, what the f was I passed???
}

//******************************************************************************************************************
//******************************************************************************************************************