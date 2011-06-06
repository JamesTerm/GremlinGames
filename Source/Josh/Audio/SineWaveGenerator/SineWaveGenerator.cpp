#include "stdafx.h"
// TODO: reference additional headers your program requires here
#include "Devices_Audio_SineWaveGenerator.h"
#include "../../../FrameWork/Base/FrameWork_Base.h"

using namespace FrameWork::Base;
using namespace Devices;
using namespace Devices::Audio;
using namespace Devices::Audio::SineWave;
using namespace Devices::Audio::Base::Streams;
using namespace Devices::Audio::Base::aud_convert;


Generator_BufferFill::Generator_BufferFill (size_t SampleRate,Devices::Audio::FormatInterface::eSampleFormat SampleFormat) :
m_SampleRate(SampleRate),m_SampleFormat(SampleFormat)
{
	m_Freq_hz=1000.0;
	//m_Amplitude = (float) 1.41421356;  //sqrt(2) 
	m_Amplitude=1.0;
	m_rho = 0;
}

Generator_BufferFill::~Generator_BufferFill()
{
}


double Generator_BufferFill::GetFrequency() const
{	
	return m_Freq_hz;
}

void Generator_BufferFill::Setfrequency(double Frequency)
{
	m_Freq_hz=Frequency;
}
double Generator_BufferFill::GetAmplitude() const
{
	return m_Amplitude;
}
void Generator_BufferFill::SetAmplitude(double Amplitude)
{
	m_Amplitude=Amplitude;
}


void Generator_BufferFill::SineBufSizeFreq(byte *DestBuffer,size_t NoSamples)
{
	size_t HalfBuf=NoSamples>>1,QuartBuf=NoSamples>>2,temp=0;
	for (size_t j=QuartBuf;j<HalfBuf;j++)
	{
		double Sample=sin((((double)j/(double)(HalfBuf))*2)-1);
		*(((long *)(DestBuffer))+(temp++))=(long)(Sample*(double)0x7fffffff);
	}
	for (size_t j=HalfBuf;j>0;j--)
	{
		double Sample=sin((((double)j/(double)(HalfBuf))*2)-1);
		*(((long *)(DestBuffer))+(temp++))=(long)(Sample*(double)0x7fffffff);
	}

	for (size_t j=0;j<QuartBuf;j++)
	{
		double Sample=sin((((double)j/(double)(HalfBuf))*2)-1);
		*(((long *)(DestBuffer))+(temp++))=(long)(Sample*(double)0x7fffffff);
	}
}


void Generator_BufferFill::SWfreq(byte *DestBuffer,size_t NoSamples)
{
	double			 freq_hz=m_Freq_hz;
	double			 amplitude=m_Amplitude;
	double			 rho,theta, scale, pi2;
	size_t index=0; //array index of buffer

	rho = m_rho;

	pi2 = 3.1415926;
	//Compute the angle ratio unit we are going to use
	theta = freq_hz / m_SampleRate;
	pi2 *= 2.0;
	//Convert the angle ratio unit into radians
	theta *= pi2;

	//set our scale... this is also the size of the radius 
	scale = amplitude;

	{
		size_t				 siz=NoSamples;
		double				 Sample;


		while( siz-- )
		{
			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			Sample = sin( rho ) * scale;
			//increase our angular measurement
			rho += theta;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if ( rho > pi2 )
				rho -= pi2;

			//Now to populate the buffer with our sample
			switch (m_SampleFormat)
			{
				//Note the minus sign puts the right channel out of phase
				case Devices::Audio::FormatInterface::eSampleFormat_FP_32:
					*(((float *)(DestBuffer))+(index++))=(float)Sample;
					break;
				case Devices::Audio::FormatInterface::eSampleFormat_I_32:
					*(((long *)(DestBuffer))+(index++))=(long)(Sample*(double)0x7fffffff);
					break;
				case Devices::Audio::FormatInterface::eSampleFormat_I_16:
					*(((short *)(DestBuffer))+(index++))=(short)(Sample*(double)0x7fff);
					break;
				case Devices::Audio::FormatInterface::eSampleFormat_U_8:
					*(((byte *)(DestBuffer))+(index++))=(byte)((Sample*(double)0x7f)+0x80);
					break;
			}
		}
	}
	//save the sine wave state
	m_rho = rho;
}



  /***********************************************************************************************************************/
 /*													Generator_Streamer													*/
/***********************************************************************************************************************/


#pragma warning (disable : 4355)
Generator_Streamer::Generator_Streamer(const wchar_t *pDeviceName,size_t SampleRate,Devices::Audio::FormatInterface::eSampleFormat SampleFormat,size_t NoChannels) :
	m_pDeviceName(pDeviceName),m_SampleRate(SampleRate),m_SampleFormat(SampleFormat),
	m_OutputStream(this,this),m_Timer(0),m_LastSamplePacketSize(0),m_OverageTime(0.0),m_bIsInterleaved(false)
{
	SetNoChannels(NoChannels);
}
#pragma warning (default : 4355)

Generator_Streamer::~Generator_Streamer()
{
	m_LastSamplePacketSize=0; //a safety check
	//A clever way to free all the instances >:)
	SetNoChannels(0);
}

double Generator_Streamer::GetFrequency(size_t channel) const
{	
	return m_BufferFillList[channel]->GetFrequency();
}
void Generator_Streamer::Setfrequency(size_t channel,double Frequency)		
{	
	m_BufferFillList[channel]->Setfrequency(Frequency);
}
double Generator_Streamer::GetAmplitude(size_t channel) const
{	
	return m_BufferFillList[channel]->GetAmplitude();
}
void Generator_Streamer::SetAmplitude(size_t channel,double Amplitude)		
{	
	m_BufferFillList[channel]->SetAmplitude(Amplitude);
}
bool Generator_Streamer::AddBufferToQueue( Devices::BufferInterface *pBuffer ) 
{
	return true;
}
const wchar_t *Generator_Streamer::pGetDeviceName( void ) const 
{
	return m_pDeviceName;
}
size_t Generator_Streamer::GetSampleRate( void ) const 
{	
	return m_SampleRate;
}
Devices::Audio::FormatInterface::eSampleFormat Generator_Streamer::GetSampleFormat( void ) const
{	
	return m_SampleFormat;
}
bool Generator_Streamer::GetIsBufferInterleaved( void ) const
{
	return m_bIsInterleaved;
}
size_t Generator_Streamer::GetNoChannels( void ) const 
{	
	return m_BufferFillList.size();
}

Devices::Audio::DeviceInterface *Generator_Streamer::GetAudioDevice() 
{
	return m_OutputStream.GetAudioDevice();
}


void Generator_Streamer::SetNoChannels(size_t NoChannels)
{
	//Ensure that the fill buffer has not gone into effect yet
	assert(m_LastSamplePacketSize==0);

	size_t Current=m_BufferFillList.size();
	if (Current!=NoChannels)
	{
		//figure out how many to add and remove
		if (Current>NoChannels)
		{
			for (size_t i=0;i<(Current-NoChannels);i++)
			{
				Generator_BufferFill *RemoveThis=m_BufferFillList[0];
				m_BufferFillList.erase(m_BufferFillList.begin());
				delete RemoveThis;
			}
		}
		else
		{
			for (size_t i=0;i<(NoChannels-Current);i++)
			{
				Generator_BufferFill *AddThis=new Generator_BufferFill(m_SampleRate,m_SampleFormat);
				m_BufferFillList.insert(m_BufferFillList.end(),AddThis);
			}
		}
	}
}

//virtual bool FillBuffer(byte *DestBuffer,size_t NoSamples,FormatInterface *Format,TimeStamp &DeviceTime);
bool Generator_Streamer::FillBuffer(PBYTE DestBuffer,size_t BufferSizeInBytes,TimeStamp &DeviceTime)
{
	size_t NoSamples;
	{
		size_t BytesPerSample=aud_convert::GetFormatBitDepth(GetSampleFormat()) * GetNoChannels();  //we can assume float now
		NoSamples = (BufferSizeInBytes/BytesPerSample);
	}
	//SineBufSizeFreq(DestBuffer,NoSamples);
	m_LastSamplePacketSize=NoSamples;
	size_t NoChannels=GetNoChannels();
	size_t ChannelSizeInBytes=NoSamples*sizeof(float); //Note the OutStream no longer supports other formats
	for (size_t i=0;i<NoChannels;i++)
	{
		m_BufferFillList[i]->SWfreq(DestBuffer+(ChannelSizeInBytes*i),NoSamples);
	}
	return true;
}

void Generator_Streamer::WaitForNextBuffer()
{

	//Testing a bad time
	//Sleep(11);
	//return;

	__int64 freq,time;
	//Find out the current time
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	QueryPerformanceCounter((LARGE_INTEGER *)&time);

	//The first time this is called we can return immediately
	if (m_LastSamplePacketSize==0)
	{
		m_Timer=time;
		return;
	}

	double TimeElapsed=((double)(time-m_Timer)/(double)freq);
	double SampleRate=(double)GetSampleRate();
	//SampleRate+=1250; //add some unexplained error in the amount of sleep (This doesn't matter much for this device)
	double PacketDelay=(double)m_LastSamplePacketSize / SampleRate;
	double TimeLeft=(PacketDelay-TimeElapsed) * 1000.0;
	TimeLeft+=m_OverageTime; //trim down any overage

	//if (TimeElapsed>PacketDelay)
	//	printf("Time Elapsed %f\n",TimeElapsed *1000);
	//	DebugOutput(L"Time Elapsed %f\n",TimeElapsed *1000);

	if (TimeLeft>0.0)
	{
		//DebugOutput(L"PD %f,TE %f,TL %f\n",PacketDelay,TimeElapsed,TimeLeft);
		//DebugOutput(L"Sleep %f\n",TimeLeft);
		//DebugOutput(L"Sleep(%d),\t",(DWORD)TimeLeft);
		//printf("Sleep %f\n",TimeLeft);
		Sleep((DWORD)TimeLeft);
	}

	m_Timer=time;
	QueryPerformanceCounter((LARGE_INTEGER *)&time);

	//This will show the actual time in between each buffer interval
	//printf("Time Elapsed %f\n",(TimeElapsed+((double)(time-m_Timer)/(double)freq)) *1000);
	//DebugOutput(L"Time Elapsed %f\n",(TimeElapsed+((double)(time-m_Timer)/(double)freq)) *1000);

	TimeElapsed=((double)(time-m_Timer)/(double)freq) *1000;
	m_Timer=time;

	m_OverageTime=TimeLeft-TimeElapsed; //No overage time this time

	//DebugOutput(L"TimeElapsed=%f,TimeLeft=%f,overage=%f\n",TimeElapsed,TimeLeft,m_OverageTime);
}


  /***********************************************************************************************************/
 /*													Generator												*/
/***********************************************************************************************************/

Generator::Generator(const wchar_t *pDeviceName,size_t SampleRate,size_t NoChannels) :
m_Generator(pDeviceName,SampleRate,Devices::Audio::FormatInterface::eSampleFormat_FP_32,NoChannels),
DeviceWrapper(m_Generator.GetAudioDevice())  
{
}

double Generator::GetFrequency(size_t channel) const
{	
	return m_Generator.GetFrequency(channel);
}

void Generator::Setfrequency(size_t channel,double Frequency)		
{	
	m_Generator.Setfrequency(channel,Frequency);
}

double Generator::GetAmplitude(size_t channel) const
{	
	return m_Generator.GetAmplitude(channel);
}
void Generator::SetAmplitude(size_t channel,double Amplitude)		
{	
	m_Generator.SetAmplitude(channel,Amplitude);
}
