#pragma once

#define _DS_ Devices::Streams

//TODO: These will be gone as soon as I change to use the operator casting

class DEVICES_AUDIO_BASE_API DeviceWrapper : public DeviceInterface
{
	public:
		DeviceWrapper(DeviceInterface *pAudioDevice) : m_pAudioDevice(pAudioDevice) {}
		void SetAudioDevice(DeviceInterface *AudioDevice) 
			{	m_pAudioDevice=AudioDevice;
			}

		//Wrap it all up
		//From Device Interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer )	
			{	return m_pAudioDevice->AddBufferToQueue(pBuffer); 
			}
		virtual void FlushAllBuffers( void )								
			{	m_pAudioDevice->FlushAllBuffers(); 
			}
		virtual const wchar_t *pGetDeviceName( void ) const					
			{	return m_pAudioDevice->pGetDeviceName(); 
			}
		//Audio format data
		virtual size_t GetSampleRate( void ) const 
			{	return m_pAudioDevice->GetSampleRate();
			}
		virtual size_t GetNoChannels( void ) const 
			{	return m_pAudioDevice->GetNoChannels();
			}

		virtual eSampleFormat GetSampleFormat( void ) const
			{	return m_pAudioDevice->GetSampleFormat();
			}

		//Return true If the buffer is interleaved
		virtual bool GetIsBufferInterleaved( void ) const
			{	return m_pAudioDevice->GetIsBufferInterleaved();
			}

	private:
		//note: this can't be const because it may not be possible to initialize in construtor; however it should be treated as such
		DeviceInterface *m_pAudioDevice;
};


class DEVICES_AUDIO_BASE_API AudioHostDeviceWrapper
{
	public:
		AudioHostDeviceWrapper(Host::AudioHostDevice *pAudioHostDevice) : m_pAudioHostDevice(pAudioHostDevice) {}
		void SetAudioHostDevice(Host::AudioHostDevice *HostDevice) 
			{	m_pAudioHostDevice=HostDevice;
			}

		//Wrap it all up
		Streams::AudioHostInputStream *AddAudioDevice(DeviceInterface *pDevice,size_t DefaultTBCQueueDepth=_DS_::c_DefaultTBCQueueDepth) 
			{	return m_pAudioHostDevice->AddAudioDevice(pDevice,DefaultTBCQueueDepth);
			}
		bool RemoveAudioDevice(DeviceInterface *pDeviceToRemove)		
			{	return m_pAudioHostDevice->RemoveAudioDevice(pDeviceToRemove);
			}

		float *GetVolumeMatrix(DeviceInterface *AudioDevice) const 
			{	return m_pAudioHostDevice->GetVolumeMatrix(AudioDevice);
			}
		size_t GetSourceNoChannels(DeviceInterface *AudioDevice) const 
			{	return m_pAudioHostDevice->GetSourceNoChannels(AudioDevice);
			}
		size_t GetDestNoChannels() const 
			{	return m_pAudioHostDevice->GetAudioDevice()->GetNoChannels();
			}

	private:
		//note: this can't be const because it may not be possible to initialize in construtor; however it should be treated as such
		Host::AudioHostDevice * m_pAudioHostDevice;
};

#undef _DS_
