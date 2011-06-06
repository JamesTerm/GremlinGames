#pragma once
#define _DS_ Devices::Streams

//******************************** Video
class VideoStreamBase
{
	public:
		bool DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer);
		bool DoFormatsMatch(const Video::FormatInterface *pFormat1,const Video::FormatInterface *pFormat2);
};

class DEVICES_BASE_API VideoStreamBaseDevice :	public VideoStreamBase,
												public Video::DeviceInterface
{
	public:
		//refer to parent to wrap device methods
		//And pass in the format to also be wrapped
		VideoStreamBaseDevice(Devices::DeviceInterface * const Parent,Video::FormatInterface * const Format) :
		  m_Parent(Parent),m_Format(Format) {}

	  bool DoFormatsMatch(const Devices::BufferInterface *Buffer);

		//wrap everything
		//From device interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer ) {return m_Parent->AddBufferToQueue(pBuffer);}
		virtual void FlushAllBuffers( void ) {m_Parent->FlushAllBuffers();}
		virtual const wchar_t *pGetDeviceName( void ) const {return m_Parent->pGetDeviceName();}

		//From format interface
		virtual size_t GetXResolution( void ) const {return m_Format->GetXResolution();}
		virtual size_t GetYResolution( void ) const {return m_Format->GetYResolution();}
		virtual Video::FormatInterface::eFrameFormat GetFrameFormat( void ) const {return m_Format->GetFrameFormat();}
		virtual float GetAspectRatio( void ) const {return m_Format->GetAspectRatio();}
		virtual const char *pGetFourCC( void ) const {return m_Format->pGetFourCC();}

	private:
		Devices::DeviceInterface * const m_Parent;
		Video::FormatInterface * const m_Format;
};



class DEVICES_BASE_API VideoOutputStream : public _DS_::OutputStream
{
	public:
		VideoOutputStream(_DS_::OutStream_Interface *pParent,Video::FormatInterface * const Format) : 
			_DS_::OutputStream(pParent),m_VideoStreamBase(this,Format) {}

		Video::DeviceInterface *GetVideoDevice()
			{	return &m_VideoStreamBase;
			}
	protected:
		virtual bool DoFormatsMatch(const Devices::BufferInterface *Buffer);
	private:
		VideoStreamBaseDevice m_VideoStreamBase;
};


class DEVICES_BASE_API VideoInputStream : public _DS_::InputStream
{
	public:
		VideoInputStream(_DS_::InStream_Interface *pParent,DeviceInterface *pDevice,size_t DefaultTBCQueueDepth=_DS_::c_DefaultTBCQueueDepth,const wchar_t *pDeviceClientName = L"VideoInputStream") : 
			_DS_::InputStream(pParent, pDevice,DefaultTBCQueueDepth,pDeviceClientName) {}
	protected:
		virtual bool DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer);
	private:
		VideoStreamBase m_VideoStreamBase;
};

#undef _DS_