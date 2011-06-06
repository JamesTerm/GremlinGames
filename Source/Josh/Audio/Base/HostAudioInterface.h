#pragma once

class AudioHostCallback_Interface
{
	public:
		//The buffer format should be interleaved and defined by the AudioHost (as it is handled in DirectSound)
		//ASIO host will need to handle the de-interleaving internally
		//return true if successful
		virtual bool FillBuffer(
			byte *Buffer,		//The buffer to fill
			size_t NoSamples,	//The number of samples to fill
			void *AudioInputStream_ptr=NULL //If this is not NULL no mixing will occur and it will fill this buffer with only one stream
			//Otherwise if this is NULL... the buffer will have all the streams mixed into it.
			)=0;
};

class AudioHost_Interface : public Devices::Audio::FormatInterface
{
	public:
		//This will be used to link the Audio Host to the desired callback interface (e.g. Audio Host Mixer)
		virtual void SetAudioHostCallback_Interface(AudioHostCallback_Interface *ahi)=0;

		//---------------------------------------------------AudioHost internal mixing methods-------------------------
		//You will need to keep track of this handle so that you can tell the mixer which stream you want
		//This will help the AudioHostMixer to find the stream immediately... Note: this is a pointer to void because at this time there is no
		//reason why the AudioHost needs to access any of this information... we can make an interface if this should change
		virtual void AudioHost_CreateNewStream(void *AudioInputStream_ptr)=0;
		//Here you can safely close all your stream's resources (return true if successful e.g. found in list)
		virtual void AudioHost_RemoveStream(void *AudioInputStream_ToRemove)=0;

		//Notify parent that it is closing to take appropriate action
		virtual void AudioInStreamClosing(void *ThisChild)=0;

		//When providing this value, you must be able to successfully have the information available... you may wish to use an assert
		//to ensure all the environment needed to get this value has been set up by the time this gets called.
		//Note: this gets called when the StartTBCQueue instantiates the first AudioBuffer 
		virtual size_t GetBufferSampleSize()=0;

};

