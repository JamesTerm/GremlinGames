#pragma once

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

// Predecls
class CWaveFile;

class DEVICES_AUDIO_CAPTURE_API WAVOutput : public Devices::Audio::PeakLevels::PeakLevelComputer,	// I want me some peak level support in the hizzy
											public Devices::AudioVideo::DSCapture::ICaptureSession
{
	typedef Devices::Audio::PeakLevels::PeakLevelComputer _super;

	// Interface
public:

	// C'tor/D'tor
	WAVOutput(Devices::Audio::DeviceInterface *pUpstreamDevice, Devices::Audio::PeakLevels::IPeakLevelListener *pFanOfMan, 
				Devices::Audio::FormatInterface *pFormatSpec=NULL, bool bInline=false);
	virtual ~WAVOutput( void );

	void SetOutputFilepath(const char *szFilePath);	// CALL on same thread as StartCapture!!!

	// Devices::AudioVideo::DSCapture::ICaptureSession
	virtual bool StartCapture( void );
	virtual bool StopCapture( void );
	virtual double PollPosition(bool &bDone);

	// Implementation
protected:

	virtual void DoProcess(Buffer *pFilledBuffer);
	virtual void AnalizeAudioSamples(const void *pBuffer, size_t cbData);

	// Data
protected:

	FrameWork::Threads::critical_section m_FileLock;
	std::wstring m_strFilePath;
	CWaveFile *m_pFileWriter;

	// Illegal
private:

	//WAVOutput(const WAVOutput&) { assert(false); }
	void operator = (const WAVOutput&) { assert(false); }
};

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************
