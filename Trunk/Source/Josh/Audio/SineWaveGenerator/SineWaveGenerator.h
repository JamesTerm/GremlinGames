#pragma once

///This is the heart of the sine wave generator.  It works by first establishing the angle unit to use (theta [in radians]) and then increments the 
///angle on each iteration (rho) finally a sample is obtained by finding Y given the angle (rho) and the hypotenuse 
///(i.e. radius, amplitude or scaling factor).  This will initially start the sine wave at 0 peak to positive then peak to negative and end with 0
///for one cycle, the angle measurement (rho) is preserved for subsequent calls to create a seamless waveform.
class SINEWAVEGENERATOR_API Generator_BufferFill
{
	public:
		Generator_BufferFill (size_t SampleRate=44100,
			Devices::Audio::FormatInterface::eSampleFormat SampleFormat=
			Devices::Audio::FormatInterface::eSampleFormat_I_32);
		virtual ~Generator_BufferFill();

		//This one is a simple one that completes one cycle over the span of the buffer size.
		void SineBufSizeFreq(byte *DestBuffer,size_t NoSamples);
		//This one works with a fixed rate and keeps track of where the wave leaves off from a previous buffer packet.
		void SWfreq(byte *DestBuffer,size_t NoSamples);

		//returns m_Freq_hz;
		double GetFrequency() const;
		//sets m_Freq_hz
		void Setfrequency(double Frequency);
		//returns m_Amplitude;
		double GetAmplitude() const;
		//sets m_Amplitude
		void SetAmplitude(double Amplitude);
	protected:
		size_t m_SampleRate;
		Devices::Audio::FormatInterface::eSampleFormat m_SampleFormat;

	private:
		///How many revolutions per second
		double m_Freq_hz;
		///The scaling factor to apply default is 1.0
		double m_Amplitude;
		///keep track of the last angle measurement to create seamless waves
		double m_rho;
};

class SINEWAVEGENERATOR_API Generator_Streamer	 :	public Devices::Streams::OutStream_Interface,
													public Devices::Audio::FormatInterface
{
	public:
		Generator_Streamer(const wchar_t *pDeviceName = L"SineWave",size_t SampleRate=44100,
			Devices::Audio::FormatInterface::eSampleFormat SampleFormat=
			Devices::Audio::FormatInterface::eSampleFormat_I_32,
			size_t NoChannels=2);

		virtual ~Generator_Streamer();

		//returns the buffer fill list's get frequency
		double GetFrequency(size_t channel) const;
		//sets the buffer fill list's set frequency		
		void Setfrequency(size_t channel,double Frequency);
		//returns the buffer fill list's get amplitude
		double GetAmplitude(size_t channel) const;
		//sets the buffer fill list's set amplitude
		void SetAmplitude(size_t channel,double Amplitude);

		//returns m_OutputStream
		DeviceInterface *GetAudioDevice();

		//returns m_SampleFormat;
		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
		//returns the buffer fill list's size
		virtual size_t GetNoChannels( void ) const;
		//Return true If the buffer is interleaved
		virtual bool GetIsBufferInterleaved( void ) const;

	protected:
		//Each channel will have its own instance
		#pragma warning (disable : 4251)
		std::vector<Generator_BufferFill *> m_BufferFillList;
		#pragma warning (default : 4251)

	private:  //From OutStream_Interface
		virtual bool FillBuffer(PBYTE DestBuffer,size_t BufferSizeInBytes,TimeStamp &DeviceTime);
		virtual void WaitForNextBuffer();

		//From Device Interface

		//nothing to do for this... just returns true
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer ); 
		//returns m_pDeviceName
		virtual const wchar_t *pGetDeviceName( void ) const;

	private:
		//Audio format data
		virtual size_t GetSampleRate( void ) const;		//returns m_SampleRate

		//For now only set this up in the constructor
		void SetNoChannels(size_t NoChannels);

		// My device name
		const wchar_t * const m_pDeviceName;
		Devices::Audio::Base::Streams::AudioOutputStream m_OutputStream;

		bool m_bIsInterleaved;
		size_t m_SampleRate;
		Devices::Audio::FormatInterface::eSampleFormat m_SampleFormat;

		__int64 m_Timer; //used to stream to high precision
		size_t m_LastSamplePacketSize;
		double m_OverageTime; //used to trim the wait time for sleep error
};

using namespace Devices::Audio::Base;
class SINEWAVEGENERATOR_API Generator : public DeviceWrapper
{
	public:
		Generator(const wchar_t *pDeviceName = L"SineWave",size_t SampleRate=44100,size_t NoChannels=2);

		//Note all of these are just wrapped to use the generator streamer
		double GetFrequency(size_t channel) const;
		void Setfrequency(size_t channel,double Frequency);
		double GetAmplitude(size_t channel) const;
		void SetAmplitude(size_t channel,double Amplitude);

	private:
		Generator_Streamer m_Generator;
};
