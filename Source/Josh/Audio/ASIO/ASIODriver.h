#pragma once

const int CLSID_STRING_LEN=39;

enum ASIODriverState
{
	ASIO_DRIVER_STOPPED,
	ASIO_DRIVER_STARTED
};

class ASIOMgr;
class ASIOHost_DLL ASIODriver
{
friend ASIOMgr;
public:
	ASIODriver();
	virtual ~ASIODriver();

	IASIO * OpenDriver(HWND hwnd);
	bool Stop();
	bool Start();
	const char * getPath();
	const char* getName();

	//returns m_AudioHostInterface
	Devices::Audio::Base::Host::AudioHost_Interface *GetAudioHost_Interface();

	ASIOSampleRate getSampleRate();

//Note: This group of public members are needed for the internal global binding only
public:
	static void ClearCallbackTables();

	void OnBufferSwitch(long doubleBufferIndex, ASIOBool directProcess);
	void OnSampleRateDidChange(ASIOSampleRate sRate);
	long OnAsioMessage(long selector, long value, void* message, double* opt);
	ASIOTime* OnBufferSwitchTimeInfo(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);

private:
	void setPath(char *path);
	void setCLSID(char *clsid);
	void setCLSID(CLSID clsid);
	int getNbOutput();
	int getNbInput();
	ASIOBufferInfo  *GetBufferInfo(size_t channel);
	ASIOChannelInfo *GetChannelInfo(size_t channel);
	int getBufferSize();
	const char * getCLSID();
	int GetState();
	IASIO * GetIASIOInterface();
	void CloseDriver();
	bool SetId(int id);


private:	
	char					m_sclsid[CLSID_STRING_LEN];
	CLSID					m_clsid;
	char					m_dllpath[MAXPATHLEN];
	char					m_drvname[MAXDRVNAMELEN];
	int						m_state;
	double					m_CurrentSamplePosition;
	int						m_id;
	IASIO *					m_iasio;
	bool CreateBuffer();
	ASIOBufferInfo			*m_bufferInfos;
	ASIOChannelInfo			*m_channelInfos;
	ASIOCallbacks			m_asioCallbacks;
	long					m_nbInputChannels;
	long					m_nbOutputChannels;
	long					m_bufferSize;
	ASIOSampleRate			m_samplerate;

	//Note: I would have preferred to inherit from the ASIODriver... but due the callback mechanism... aggregation will prevent the need for
	//dynamic casting.
	class ASIOHost_DLL ASIOAudioHost : public Devices::Audio::Base::Host::AudioHost_Interface
	{
	public:
		ASIOAudioHost(ASIODriver *pParent);
		~ASIOAudioHost();
		virtual void SetAudioHostCallback_Interface(Devices::Audio::Base::Host::AudioHostCallback_Interface *ahi) 
		{	m_pAudioHostCallback=ahi;
		}
		//AudioHostMixer *GetAudioMixer() {return &m_AudioMixer;}
		Devices::Audio::Base::Host::AudioHostCallback_Interface *GetAudioHostCallback() {return m_pAudioHostCallback;}
		virtual void AudioInStreamClosing(void *ThisChild) 
		{	SetAudioHostCallback_Interface(NULL); //no longer pointing to this interface
		}

		//Implementation of the audio format interface
		virtual size_t GetSampleRate( void ) const;
		virtual eSampleFormat GetSampleFormat( void ) const;
		virtual bool GetIsBufferInterleaved( void ) const {return false;}

		//TODO find out how to find the channels
		virtual size_t GetNoChannels( void ) const;

		//Implementation from Audio Host Interface
		virtual size_t GetBufferSampleSize() {return (size_t)m_pParent->getBufferSize();}

		void ASIODriver::OnBufferSwitch(long doubleBufferIndex, ASIOBool directProcess);

		//I'm not managing the mixing internally
		virtual void AudioHost_CreateNewStream(void *AudioInputStream_ptr) {}
		virtual void AudioHost_RemoveStream(void *AudioInputStream_ToRemove) {}
	private:
		ASIODriver * const m_pParent;
		//Note: we want to be able to dynamically change this
		Devices::Audio::Base::Host::AudioHostCallback_Interface * m_pAudioHostCallback;
	} m_AudioHostInterface;
};
