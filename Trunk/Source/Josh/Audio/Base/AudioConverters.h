#pragma once

//A Quick function which can convert the native float into any given format
//TODO: eDeinterleaved_Transform has not yet been implemented
//That is where the source is interleaved and the destination is de-interleaved
bool DEVICES_AUDIO_BASE_API ConvertFromFloat (const float *Source,
	size_t NoSamples,
	byte *Destination,
	Devices::Audio::FormatInterface::eSampleFormat Dst_SampleFormat,
	//Note: the source and dest must have the same number of channels... any channel mixing (truncation) should be handled elsewhere
	size_t No_Channels,
	//This is used to manipulate the conversion constant that is passed to the converters
	float Dst_ScalingFactor=1.0,
	//specify what kind of buffers you have and what you want it to convert to
	bool IsSourceBufferInterleaved=false,
	bool IsDestBufferInterleaved=false
	);

//A Quick function which can convert any given format to the native float
bool DEVICES_AUDIO_BASE_API ConvertToFloat (const byte *Source,
	Devices::Audio::FormatInterface::eSampleFormat Source_SampleFormat,
	size_t NoSamples,
	float *Destination,
	//Note: the source and dest must have the same number of channels... any channel mixing (truncation) should be handled elsewhere
	size_t No_Channels,
	//This is used to manipulate the conversion constant that is passed to the converters
	float Source_ScalingFactor=1.0,
	//specify what kind of buffers you have and what you want it to convert to
	bool IsSourceBufferInterleaved=false,
	bool IsDestBufferInterleaved=false
	);


//THIS HAS NOT YET BEEN TESTED - James
//Note: the source and dest must have the same number of channels
bool DEVICES_AUDIO_BASE_API Convert(BufferInterface *Source,BufferInterface *Dest,float Source_ScalingFactor=1.0,float Dst_ScalingFactor=1.0);


class ChannelMixer_Interface
{
	public:
		virtual size_t ChannelMixer_GetSourceNoChannels(void *UserData) const=0;
		virtual size_t ChannelMixer_GetDestNoChannels(void *UserData) const=0;
		//Provide an array of float which will be used in the BLAS for the "B" matrix
		//The dimensions are SourceNoChannels x DestNoChannels... so for example if we have 4 channels and need to 
		//mix down to 2 channels it will be as follows:
		//  Row1 = destination left sample =   source left-front 0, right-front 1, left-rear 2, right rear 3
		//  Row2 = destination right sample =  source  "    "    ",  "     "    ",  "     "  ",  "      "  "
		//See AudioHostInputStream::AudioHostInputStream() for example default matrix pattern
		// for the 4:2 it will make the following:
		//1 0 1 0
		//0 1 0 1
		//Where the left front and left rear are mixed and the right front and right rear are mixed
		virtual float *ChannelMixer_GetVolumeMatrix(void *UserData) const=0;
};

class DEVICES_AUDIO_BASE_API ChannelMixer
{
	public:
		ChannelMixer(const ChannelMixer_Interface *pChannelMixer_Interface);

		//Source buffer needs to be deinterleaved and Dest buffer will be also
		void Mix(const float *Source,float *Dest,size_t NoSamples,
			//If this is true the result will be added to the memory already set in Dest
			//false will "overwrite" the result on Dest (ie. zero the dest memory)
			bool mix,
			//This pointer will pass thru to all the interface methods called... this will allow you to customize extra parameters you may
			//want for this function (e.g. stream reference)
			void *UserData=NULL
			);

		//Sets m_MasterVolume
		void SetMasterVolume(float Vol);
		//returns m_MasterVolume
		float GetMasterVolume() const;
		//This will create a good default matrix to use 
		//as an intelligent default guess for the volume mix.. its a simple diagonal 1.0 across the channels
		//This works nice for a 2 : 2 and 4 : 2 mix down
		//Be sure to call DestroyDefaultVolumeMatrix when you are finished with it!
		static float *CreateDefaultVolumeMatrix(size_t SourceNoChannels,size_t DestNoChannels);
		//You must call this when finished (there is a check to ensure the heap where free is called is local to this project)
		static void DestroyDefaultVolumeMatrix(float *VolumeMatrix);
	private:
		const ChannelMixer_Interface * const m_pChannelMixer_Interface;
		float m_MasterVolume;
};