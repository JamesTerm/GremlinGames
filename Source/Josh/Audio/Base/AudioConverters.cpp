#include "StdAfx.h"

// Include the main project
#include "Devices_Audio_Base.h"

// No 64 bit BLAS libs right now
/*#ifdef _M_IX86
#include "../../../Utilities/Blas/Utilities_BLAS.h"
#endif*/

using namespace FrameWork::Base;
using namespace std;

namespace Devices
{
	namespace Audio
	{
		namespace Base
		{
			namespace aud_convert
			{

const float INTEGER_MAX_FP=	((float) 0x7fffff * (float)(1 << 8));
const float INTEGER_MIN_FP=	((float)-0x7fffff * (float)(1 << 8));

const float SHORT_MAX_FP=	(float)(0x7fff);
const float SHORT_MIN_FP= (float)(-32768);

const float BYTE_MAX_FP=	(float)(0x7f);
const float BYTE_MIN_FP=  (float)(-128);

enum ScatterType
{
	eNo_Transform,
	eInterleaved_Transform,
	eDeinterleaved_Transform
};


//I no longer need to disable warning 4730... I did indeed have some mixed __m64 with float, but the mmx has been removed
//Thus these warning are no longer produced
//  [8/14/2007 James]

template<bool TransformResult>
void Float2Long(long *Dest,const float *Source,size_t NoSamples,float conversion_constant,
				size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				)
{
	const __m128 LONGps=_mm_set1_ps(conversion_constant);
	const __m128 LONG_MAXps=_mm_set1_ps( INTEGER_MAX_FP );
	const __m128 LONG_MINps=_mm_set1_ps( INTEGER_MIN_FP );

	size_t i,count=NoSamples>>2; //we'll be handling 4 samples at a time
	for (i=0;i<count;i++)
	{
		__m128 t;

		t=
		_mm_max_ps
		(
			_mm_min_ps
			(
				_mm_mul_ps
				(
					//Note: I can no longer work with aligned instructions
					//*(__m128 *)(Source + (i<<2)),
					_mm_loadu_ps((Source + (i<<2))),
					LONGps
				),
				LONG_MAXps
			),
			LONG_MINps
		);

		if (TransformResult)
		{
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*0)+Interleave_Offset]=(long)((float *)&t)[0];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*1)+Interleave_Offset]=(long)((float *)&t)[1];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*2)+Interleave_Offset]=(long)((float *)&t)[2];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*3)+Interleave_Offset]=(long)((float *)&t)[3];
		}
		else
		{
			//We are no longer using __m64 MMX, this way we need not call empty before X2FloatCommon1
			//*(__m64 *)(Dest + (i<<2)) = _mm_cvtps_pi32( t );
			//t=_mm_movehl_ps(t,t); //hmm is this the best instruction? well it works :P
			//*(__m64 *)(Dest + ((i<<2)+2)) = _mm_cvtps_pi32( t );

			float * Sample=(float *)&t;
			Dest[(i<<2)+0]=(short)Sample[0];
			Dest[(i<<2)+1]=(short)Sample[1];
			Dest[(i<<2)+2]=(short)Sample[2];
			Dest[(i<<2)+3]=(short)Sample[3];

		}
	}

	//TODO not tested
	//Note: chances are that buffer sizes are a multiple of 4
	size_t remainder=NoSamples&3;
	count=NoSamples-remainder;
	for (i=count;i<count+remainder;i++)
	{
		float Sample=Source[i];
		Sample*=conversion_constant;
		Sample=max(INTEGER_MIN_FP,min(Sample,INTEGER_MAX_FP));
		if (TransformResult)
			Dest[(i*Interleave_IntervalSkip)+Interleave_Offset]=(long)(Sample);
		else
			Dest[i]=(long)(Sample);
	}
}

template<bool TransformResult>
void Float2Short(short *Dest,const float *Source,size_t NoSamples,float conversion_constant,
				 size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				 size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				 )
{
	const __m128 SHORTps=_mm_set1_ps(conversion_constant);
	const __m128 SHORT_MAXps=_mm_set1_ps( SHORT_MAX_FP );	/* ((1 << 31)-1) */
	const __m128 SHORT_MINps=_mm_set1_ps( SHORT_MIN_FP );

	size_t i,count;
	count=NoSamples>>2; //we'll be handling 4 samples at a time

	for ( i = 0; i < count; i ++ )
	{
		__m128		t;

		t =
			_mm_max_ps 
			(
				_mm_min_ps
				(
					_mm_mul_ps
					(
						//Note: I can no longer work with aligned instructions
						//*(__m128 *)(Source + (i<<2)),
						_mm_loadu_ps(Source + (i<<2)),
						SHORTps
					),
					SHORT_MAXps
				),
				SHORT_MINps
		    );
		if (TransformResult)
		{
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*0)+Interleave_Offset]=(short)((float *)&t)[0];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*1)+Interleave_Offset]=(short)((float *)&t)[1];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*2)+Interleave_Offset]=(short)((float *)&t)[2];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*3)+Interleave_Offset]=(short)((float *)&t)[3];
		}
		else
		{
			//We are no longer using __m64 MMX, this way we need not call empty before X2FloatCommon1
			//*((__m64 *)Dest + i) = _mm_cvtps_pi16(t);
			float * Sample=(float *)&t;
			Dest[(i<<2)+0]=(short)Sample[0];
			Dest[(i<<2)+1]=(short)Sample[1];
			Dest[(i<<2)+2]=(short)Sample[2];
			Dest[(i<<2)+3]=(short)Sample[3];
		}
	}
	//TODO not tested
	//Note: chances are that buffer sizes are a multiple of 4
	size_t remainder=NoSamples&3;
	count=NoSamples-remainder;
	for (i=count;i<count+remainder;i++)
	{
		float Sample=Source[i];
		Sample*=conversion_constant;
		Sample=max(SHORT_MIN_FP,min(Sample,SHORT_MAX_FP));
		if (TransformResult)
			Dest[(i*Interleave_IntervalSkip)+Interleave_Offset]=(short)(Sample);
		else
			Dest[i]=(short)(Sample);
	}
}

template<bool TransformResult>
void Float2Byte(byte *Dest,const float *Source,size_t NoSamples,float conversion_constant,
				size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				)
{
	__m128	Sample;
	float * answer=(float *)&Sample;
	const __m128 BYTEps=_mm_set1_ps(conversion_constant);
	const __m128 BYTE_MAXps=_mm_set1_ps( BYTE_MAX_FP );
	const __m128 BYTE_MINps=_mm_set1_ps( BYTE_MIN_FP );
	const __m128 ps127_5=_mm_set1_ps(127.5f);

	size_t i,count;
	count=NoSamples>>2; //we'll be handling 4 samples at a time

	for ( i = 0; i < count; i ++ )
	{

		Sample = 
			_mm_max_ps 
			(
				_mm_min_ps
					(
					_mm_mul_ps
						(
						//Note: I can no longer work with aligned instructions
						//*(__m128 *)(Source + (i<<2)),
						_mm_loadu_ps(Source + (i<<2)),
						BYTEps
						),
					BYTE_MAXps
					),
				BYTE_MINps
			);
		Sample=_mm_add_ps(ps127_5,Sample);
		if (TransformResult)
		{
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*0)+Interleave_Offset]=(byte)(answer[0]);
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*1)+Interleave_Offset]=(byte)(answer[1]);
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*2)+Interleave_Offset]=(byte)(answer[2]);
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*3)+Interleave_Offset]=(byte)(answer[3]);
		}
		else
		{
			Dest [(i<<2)+0] = (byte)(answer[0]);
			Dest [(i<<2)+1] = (byte)(answer[1]);
			Dest [(i<<2)+2] = (byte)(answer[2]);
			Dest [(i<<2)+3] = (byte)(answer[3]);
		}
	}

	//TODO not tested
	//Note: chances are that buffer sizes are a multiple of 4
	size_t remainder=NoSamples&3;
	count=NoSamples-remainder;
	for (i=count;i<count+remainder;i++)
	{
		float Sample=Source[i];
		Sample*=conversion_constant;
		Sample=max(BYTE_MIN_FP,min(Sample,BYTE_MAX_FP));
		if (TransformResult)
			Dest[(i*Interleave_IntervalSkip)+Interleave_Offset]=(byte)(Sample);
		else
			Dest[i]=(byte)(Sample);
	}
}


template<ScatterType TransformResult,typename L>
__forceinline void X2FloatCommon1(size_t i,__m128 &t,const __m128 &coef,const __m128 &offset,float *Dest,const L *Source,size_t Interleave_IntervalSkip,size_t Interleave_Offset)
{
	if (TransformResult==eInterleaved_Transform)
		{
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*0)+Interleave_Offset]=((float *)&t)[0];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*1)+Interleave_Offset]=((float *)&t)[1];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*2)+Interleave_Offset]=((float *)&t)[2];
			Dest[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*3)+Interleave_Offset]=((float *)&t)[3];
		}
		else if (TransformResult==eDeinterleaved_Transform)
		{
			((float *)&t)[0]=(float)(Source[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*0)+Interleave_Offset]);
			((float *)&t)[1]=(float)(Source[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*1)+Interleave_Offset]);
			((float *)&t)[2]=(float)(Source[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*2)+Interleave_Offset]);
			((float *)&t)[3]=(float)(Source[((i<<2)*Interleave_IntervalSkip)+(Interleave_IntervalSkip*3)+Interleave_Offset]);

			//Note: I can no longer work with aligned instructions
			//_mm_store_ps
			_mm_storeu_ps
				(
					Dest+(i<<2),
					_mm_mul_ps
					(
						_mm_sub_ps(t,offset),
						coef
					)
				);
		}
		else if (TransformResult==eNo_Transform)
		{
			_mm_storeu_ps(Dest+(i<<2),t);
		}
}


//Note: chances are that buffer sizes are a multiple of 4								
template<ScatterType TransformResult,typename L>
__forceinline void X2FloatCommon2(float *Dest,const L *Source,size_t Interleave_IntervalSkip,size_t Interleave_Offset,size_t NoSamples,float conversion_constant)
{
	size_t remainder=NoSamples&3;
	size_t i,count=NoSamples-remainder;
	for (i=count;i<count+remainder;i++)
	{
		float Sample;
		if ((TransformResult==eNo_Transform)||(TransformResult==eInterleaved_Transform))
		{
			Sample=(float)Source[i];
			Sample*=conversion_constant;
		}
		if (TransformResult==eInterleaved_Transform)
			Dest[(i*Interleave_IntervalSkip)+Interleave_Offset]=(float)(Sample);
		else if (TransformResult==eDeinterleaved_Transform)
		{
			Sample=(float)Source[(i*Interleave_IntervalSkip)+Interleave_Offset];
			Dest[i] = (float)(Sample*conversion_constant);
		}
		else if (TransformResult==eNo_Transform)
			Dest[i]=(float)(Sample);
	}
}

template<ScatterType TransformResult>
void Float2Float(float *Dest,const float *Source,size_t NoSamples,float conversion_constant,
				size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				)
{
	const __m128 FLOATps=_mm_set1_ps(conversion_constant);

	size_t i,count=NoSamples>>2; //we'll be handling 4 samples at a time
	for (i=0;i<count;i++)
	{
		__m128 t;

		t=
				_mm_mul_ps
				(
					//Note: I can no longer work with aligned instructions
					//*(__m128 *)(Source + (i<<2)),
					_mm_loadu_ps(Source + (i<<2)),

					FLOATps
				);
		X2FloatCommon1<TransformResult,float>(i,t,FLOATps,_mm_setzero_ps(),Dest,Source,Interleave_IntervalSkip,Interleave_Offset);
	}
	X2FloatCommon2<TransformResult,float>(Dest,Source,Interleave_IntervalSkip,Interleave_Offset,NoSamples,conversion_constant);
}




bool ConvertFromFloat(const float *Source,
							size_t NoSamples,
							byte *Destination,
							Devices::Audio::FormatInterface::eSampleFormat Dst_SampleFormat,
							size_t No_Channels,
							float Dst_ScalingFactor,
							bool IsSourceBufferInterleaved,
							bool IsDestBufferInterleaved
							)
{
	ScatterType TransposeResult;
	//Now to figure out how to transform
	if (IsSourceBufferInterleaved==IsDestBufferInterleaved)
		TransposeResult=eNo_Transform;
	else
		TransposeResult=IsSourceBufferInterleaved?eDeinterleaved_Transform:eInterleaved_Transform;

	bool Success=true;
	switch(Dst_SampleFormat)
	{
		case Devices::Audio::FormatInterface::eSampleFormat_FP_32:
			switch (TransposeResult)
			{
				case eNo_Transform:
					if (Dst_ScalingFactor==1.0)
					{
						if (Source!=(float *)Destination)
							memcpy(Destination,Source,(NoSamples<<2)*No_Channels);
					}
					else
					{
						assert(Source!=(float *)Destination); //we are not going to support this, unless there is a good reason
						for(size_t i=0;i<No_Channels;i++)
						{
							Float2Float<eNo_Transform>(((float *)Destination)+(NoSamples*i),Source+(NoSamples*i),NoSamples,Dst_ScalingFactor);
						}
					}
					break;
				case eInterleaved_Transform:
					assert(Source!=(float *)Destination); //we are not going to support this, unless there is a good reason
					for(size_t i=0;i<No_Channels;i++)
						Float2Float<eInterleaved_Transform>(((float *)Destination),Source+(NoSamples*i),NoSamples,Dst_ScalingFactor,No_Channels,i);
					break;
				case eDeinterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Float2Float<eDeinterleaved_Transform>(((float *)Destination)+(NoSamples*i),Source,NoSamples,Dst_ScalingFactor,No_Channels,i);
					break;
			}
			break;
		case Devices::Audio::FormatInterface::eSampleFormat_I_32:
			if (TransposeResult==eNo_Transform)
			{
				for(size_t i=0;i<No_Channels;i++)
					Float2Long<false>(((long *)Destination)+(NoSamples*i),Source+(NoSamples*i),NoSamples,INTEGER_MAX_FP*Dst_ScalingFactor);
			}
			else
			{
				for(size_t i=0;i<No_Channels;i++)
				{
					Float2Long<true>(((long *)Destination),Source+(NoSamples*i),NoSamples,INTEGER_MAX_FP*Dst_ScalingFactor,No_Channels,i);
				}
			}
			break;
		case Devices::Audio::FormatInterface::eSampleFormat_I_16:
			if (TransposeResult==eNo_Transform)
			{
				for(size_t i=0;i<No_Channels;i++)
					Float2Short<false>((short *)Destination+(NoSamples*i),Source+(NoSamples*i),NoSamples,SHORT_MAX_FP*Dst_ScalingFactor);
			}
			else
			{
				for(size_t i=0;i<No_Channels;i++)
				{
					Float2Short<true>(((short *)Destination),Source+(NoSamples*i),NoSamples,SHORT_MAX_FP*Dst_ScalingFactor,No_Channels,i);
				}
			}

			break;
		case Devices::Audio::FormatInterface::eSampleFormat_U_8:
			if (TransposeResult==eNo_Transform)
			{
				for(size_t i=0;i<No_Channels;i++)
					Float2Byte<false>(Destination+(NoSamples*i),Source+(NoSamples*i),NoSamples,BYTE_MAX_FP*Dst_ScalingFactor);
			}
			else
			{
				for(size_t i=0;i<No_Channels;i++)
				{
					Float2Byte<true>(Destination,Source+(NoSamples*i),NoSamples,BYTE_MAX_FP*Dst_ScalingFactor,No_Channels,i);
				}
			}
			break;
		default:
			Success=false;
	}

	return Success;
}


template<ScatterType TransformResult>
void Long2Float(float *Dest,const long *Source,size_t NoSamples,float conversion_constant,
				size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				)
{
	const __m128 LONGps=_mm_set1_ps(conversion_constant);

	size_t i,count=NoSamples>>2; //we'll be handling 4 samples at a time
	for (i=0;i<count;i++)
	{
		__m128 t=_mm_setzero_ps();

		if ((TransformResult==eNo_Transform)||(TransformResult==eInterleaved_Transform))
		{
			/*
			__m128 t=_mm_setzero_ps();
			t=_mm_cvtpi32_ps(t,*(__m64 *)(Source + 2 + (i<<2)));
			t=_mm_movelh_ps(t,t);
			t=_mm_cvtpi32_ps(t,*(__m64 *)(Source + (i<<2)));
			t=_mm_mul_ps(t,LONGps);
			*/

			//_mm_cvtpi16_ps(*(__m64 *)(Source + (i<<2)))
			//We are no longer using __m64 MMX, this way we need not call empty before X2FloatCommon1
			float Sample[4];
			Sample[0]=(float)Source[(i<<2)+0];
			Sample[1]=(float)Source[(i<<2)+1];
			Sample[2]=(float)Source[(i<<2)+2];
			Sample[3]=(float)Source[(i<<2)+3];

			t=_mm_mul_ps(_mm_loadu_ps(Sample),LONGps);
		}
		X2FloatCommon1<TransformResult,long>(i,t,LONGps,_mm_setzero_ps(),Dest,Source,Interleave_IntervalSkip,Interleave_Offset);
	}
	X2FloatCommon2<TransformResult,long>(Dest,Source,Interleave_IntervalSkip,Interleave_Offset,NoSamples,conversion_constant);
}

template<ScatterType TransformResult>
void Short2Float(float *Dest,const short *Source,size_t NoSamples,float conversion_constant,
				 size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				 size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				 )
{
	const __m128 SHORTps=_mm_set1_ps(conversion_constant);

	size_t i,count=NoSamples>>2; //we'll be handling 4 samples at a time
	for (i=0;i<count;i++)
	{

		/*
		__m128 t;
		if ((TransformResult==eNo_Transform)||(TransformResult==eInterleaved_Transform))
		{
			t=_mm_setzero_ps();
			t=_mm_cvtpi16_ps(*(__m64 *)(Source + (i<<2)));
			t=_mm_mul_ps(t,SHORTps);
		}
		*/

		/**/
		__m128 t=_mm_setzero_ps();

		if ((TransformResult==eNo_Transform)||(TransformResult==eInterleaved_Transform))
		{

			//We are no longer using __m64 MMX, this way we need not call empty before X2FloatCommon1
			//_mm_cvtpi16_ps(*(__m64 *)(Source + (i<<2)))
			float Sample[4];
			Sample[0]=(float)Source[(i<<2)+0];
			Sample[1]=(float)Source[(i<<2)+1];
			Sample[2]=(float)Source[(i<<2)+2];
			Sample[3]=(float)Source[(i<<2)+3];

			t=_mm_mul_ps(_mm_loadu_ps(Sample),SHORTps);
		}
		X2FloatCommon1<TransformResult,short>(i,t,SHORTps,_mm_setzero_ps(),Dest,Source,Interleave_IntervalSkip,Interleave_Offset);
	}
	X2FloatCommon2<TransformResult,short>(Dest,Source,Interleave_IntervalSkip,Interleave_Offset,NoSamples,conversion_constant);
}

template<ScatterType TransformResult>
void Byte2Float(float *Dest,const byte *Source,size_t NoSamples,float conversion_constant,
				size_t Interleave_IntervalSkip=1,	//used to transpose to interleaved (This is NoChannels)
				size_t Interleave_Offset=0			//used to transpose to interleaved (This is the current channel)
				)
{
	const __m128 BYTEps=_mm_set1_ps(conversion_constant);
	const __m128 ps127_5=_mm_set1_ps(127.5f);

	size_t i,count=NoSamples>>2; //we'll be handling 4 samples at a time
	for (i=0;i<count;i++)
	{
		__m128 t=_mm_setzero_ps();

		if ((TransformResult==eNo_Transform)||(TransformResult==eInterleaved_Transform))
		{
			/*
			__m128 t=_mm_setzero_ps();
			t=_mm_cvtpu8_ps(*(__m64 *)(Source + (i<<2)));
			t=_mm_sub_ps(t,ps127_5);
			t=_mm_mul_ps(t,BYTEps);
			*/

			//We are no longer using __m64 MMX, this way we need not call empty before X2FloatCommon1
			float Sample[4];
			Sample[0]=(float)Source[(i<<2)+0];
			Sample[1]=(float)Source[(i<<2)+1];
			Sample[2]=(float)Source[(i<<2)+2];
			Sample[3]=(float)Source[(i<<2)+3];

			t=_mm_mul_ps
				(
				_mm_sub_ps(_mm_loadu_ps(Sample),ps127_5),
				BYTEps
				);
		}
		X2FloatCommon1<TransformResult,byte>(i,t,BYTEps,ps127_5,Dest,Source,Interleave_IntervalSkip,Interleave_Offset);
	}
	X2FloatCommon2<TransformResult,byte>(Dest,Source,Interleave_IntervalSkip,Interleave_Offset,NoSamples,conversion_constant);
}

bool  ConvertToFloat (const byte *Source,
							Devices::Audio::FormatInterface::eSampleFormat Source_SampleFormat,
							size_t NoSamples,
							float *Destination,
							size_t No_Channels,
							float Source_ScalingFactor,
							bool IsSourceBufferInterleaved,
							bool IsDestBufferInterleaved
							)
{
	bool Success=true;

	ScatterType TransposeResult;
	//Now to figure out how to transform
	if (IsSourceBufferInterleaved==IsDestBufferInterleaved)
		TransposeResult=eNo_Transform;
	else
		TransposeResult=IsSourceBufferInterleaved?eDeinterleaved_Transform:eInterleaved_Transform;

	switch(Source_SampleFormat)
	{
		case Devices::Audio::FormatInterface::eSampleFormat_FP_32:
			switch (TransposeResult)
			{
				case eNo_Transform:
					if (Source_ScalingFactor==1.0)
					{
						if (Source!=(byte *)Destination)
							memcpy(Destination,Source,(NoSamples<<2)*No_Channels);
					}
					else
					{
						assert(Source!=(byte *)Destination); //we are not going to support this, unless there is a good reason
						for(size_t i=0;i<No_Channels;i++)
						{
							Float2Float<eNo_Transform>((Destination)+(NoSamples*i),(float *)Source+(NoSamples*i),NoSamples,Source_ScalingFactor);
						}
					}
					break;
				case eInterleaved_Transform:
					assert(Source!=(byte *)Destination); //we are not going to support this, unless there is a good reason
					for(size_t i=0;i<No_Channels;i++)
						Float2Float<eInterleaved_Transform>(Destination,(float *)Source+(NoSamples*i),NoSamples,Source_ScalingFactor,No_Channels,i);
					break;
				case eDeinterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Float2Float<eDeinterleaved_Transform>(Destination+(NoSamples*i),(float *)Source,NoSamples,Source_ScalingFactor,No_Channels,i);
					break;
			}
			break;
		case Devices::Audio::FormatInterface::eSampleFormat_I_32:
			switch (TransposeResult)
			{
				case eNo_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Long2Float<eNo_Transform>((Destination)+(NoSamples*i),(long *)Source+(NoSamples*i),NoSamples,1/(INTEGER_MAX_FP*Source_ScalingFactor));
					break;
				case eInterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Long2Float<eInterleaved_Transform>(Destination,(long *)Source+(NoSamples*i),NoSamples,1/(INTEGER_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
				case eDeinterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Long2Float<eDeinterleaved_Transform>(Destination+(NoSamples*i),(long *)Source,NoSamples,1/(INTEGER_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
			}
			break;
		case Devices::Audio::FormatInterface::eSampleFormat_I_16:
			switch (TransposeResult)
			{
				case eNo_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Short2Float<eNo_Transform>(Destination+(NoSamples*i),(short *)Source+(NoSamples*i),NoSamples,1/(SHORT_MAX_FP*Source_ScalingFactor));
					break;
				case eInterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Short2Float<eInterleaved_Transform>(Destination,(short *)Source+(NoSamples*i),NoSamples,1/(SHORT_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
				case eDeinterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Short2Float<eDeinterleaved_Transform>(Destination+(NoSamples*i),(short *)Source,NoSamples,1/(SHORT_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
			}
			break;
		case Devices::Audio::FormatInterface::eSampleFormat_U_8:
			switch (TransposeResult)
			{
				case eNo_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Byte2Float<eNo_Transform>(Destination+(NoSamples*i),Source+(NoSamples*i),NoSamples,1/(BYTE_MAX_FP*Source_ScalingFactor));
					break;
				case eInterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Byte2Float<eInterleaved_Transform>(Destination,Source+(NoSamples*i),NoSamples,1/(BYTE_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
				case eDeinterleaved_Transform:
					for(size_t i=0;i<No_Channels;i++)
						Byte2Float<eDeinterleaved_Transform>(Destination+(NoSamples*i),Source,NoSamples,1/(BYTE_MAX_FP*Source_ScalingFactor),No_Channels,i);
					break;
			}
			break;
		default:
			Success=false;
	}

	return Success;

}


bool Convert(BufferInterface *Source,BufferInterface *Dest,float Source_ScalingFactor,float Dst_ScalingFactor)
{
	size_t MemorySize;
	byte *SourceBuffer=(byte *)Source->pGetBufferData(&MemorySize);
	//Note: the source and dest must have the same number of channels... any channel mixing (truncation) should be handled elsewhere
	size_t No_Channels=Source->GetNoChannels();
	assert(No_Channels==Dest->GetNoChannels());
	//Now to figure out how many samples there are
	size_t BytesPerSample=GetFormatBitDepth(Source->GetSampleFormat()) * No_Channels;  //we can assume float now
	//size_t BytesPerSample=sizeof(float) * GetNoChannels();
	size_t No_Samples = (MemorySize / BytesPerSample);

	//Note: I probably should be checking this
	byte *DestBuffer=(byte *)Dest->pGetBufferData(NULL);
	//sanity check... this is not supported
	if (SourceBuffer==DestBuffer)
	{
		assert(false);
		return false;
	}

	bool IsSourceBufferInterleaved=Source->GetIsBufferInterleaved();
	bool IsDestBufferInterleaved=Dest->GetIsBufferInterleaved();

	size_t ChannelSizeInBytes=No_Samples*sizeof(float);
	size_t BufferSizeInBytes=ChannelSizeInBytes*No_Channels;

	byte *TempBuf=(byte *)_alloca(BufferSizeInBytes + 16);
	float *AlignedTempBuf=(float *)(((size_t)TempBuf+0x0f)&~0x0f); //We need this aligned to a 16 byte boundary

	if (
		ConvertToFloat (SourceBuffer,Source->GetSampleFormat(),No_Samples,AlignedTempBuf,No_Channels,Source_ScalingFactor,
		IsSourceBufferInterleaved,IsDestBufferInterleaved))
	{
		return ConvertFromFloat (AlignedTempBuf,No_Samples,DestBuffer,Dest->GetSampleFormat(),No_Channels,Dst_ScalingFactor,
			IsSourceBufferInterleaved,IsDestBufferInterleaved);
	}
	return false;
}


  /*******************************************************************************************************/
 /*													ChannelMixer										*/
/*******************************************************************************************************/


ChannelMixer::ChannelMixer(const ChannelMixer_Interface *pChannelMixer_Interface) : 
	m_pChannelMixer_Interface(pChannelMixer_Interface)
{
	m_MasterVolume=1.0f;
}

void ChannelMixer::SetMasterVolume(float Vol) 
{
	m_MasterVolume=Vol;
}

float ChannelMixer::GetMasterVolume() const
{
	return m_MasterVolume;
}


// No 64 bit BLAS libs right now
/*#ifdef _M_IX86
using namespace Utilities::BLAS;
#endif*/

void ChannelMixer::Mix(const float *Source,float *Dest,size_t NoSamples,bool mix,void *UserData)
{
	int SourceNoChannels=(int)m_pChannelMixer_Interface->ChannelMixer_GetSourceNoChannels(UserData);
	int DestNoChannels=(int)m_pChannelMixer_Interface->ChannelMixer_GetDestNoChannels(UserData);
	if (!mix)
		memset(Dest,0,NoSamples*DestNoChannels*sizeof(float));

// No 64 bit BLAS libs right now
/*#ifdef _M_IX86
	//BLAS makes life easy for me ;)  ----  c = alpha * op_a * op_b + beta * c
	cblas_sgemm(
		CblasColMajor,		//Working with de-interleaved so the columns are on the large end
		CblasNoTrans,		//No intention to transpose this matrix
		CblasNoTrans,		//No intention to transpose this matrix (for deinterleaved it must be source across, dest down)
		(int)NoSamples,		//M -- The number of "rows" (where rows= which ever one is major so actually its columns)
		DestNoChannels,		//N -- The number of "columns" (or the "Minor" dimension)
		SourceNoChannels,	//K -- The number of columns for operation A
		m_MasterVolume,		//alpha -- will scale the entire matrix before the mix down
		Source,				//A -- The A Matrix
		(int)NoSamples,		//lda -- provides the missing dimension for A matrix which needs to be max(1,m) for non-transpose
		m_pChannelMixer_Interface->ChannelMixer_GetVolumeMatrix(UserData), //B -- the B Matrix
		SourceNoChannels,	//ldb -- provides the missing dimension for B matrix which needs to be max(1,k) for non-transpose
		1.0f,				//beta -- not used for this type of effect so it is always 1.0
		Dest,				//C -- The C Matrix
		(int)NoSamples		//ldc -- provides the missing dimension for C matrix which needs to be max(1,m) for non-transpose
		);
#else
	// TODO
#endif*/

	float *TempSourceBuf=(float *)malloc(NoSamples*SourceNoChannels*sizeof(float));
	float *TempDestBuf=(float *)malloc(NoSamples*DestNoChannels*sizeof(float));
	if (!mix)
		memset(TempDestBuf,0,NoSamples*DestNoChannels*sizeof(float));

	ConvertFromFloat(Source,NoSamples,(byte *)TempSourceBuf,FormatInterface::eSampleFormat_FP_32,SourceNoChannels,1.0,false,true);

	FrameWork::Audio::mix(
		NoSamples,															//(dimension M)
		DestNoChannels,														//(dimension N)
		SourceNoChannels,													//(dimension K)
		m_MasterVolume,														//scalar: alpha
		TempSourceBuf,														//Matrix A (MxK)
		m_pChannelMixer_Interface->ChannelMixer_GetVolumeMatrix(UserData),  //Matrix B (NxK)
		TempDestBuf);														//Matrix C (NxM)

	ConvertToFloat((byte *)TempDestBuf,FormatInterface::eSampleFormat_FP_32,NoSamples,Dest,DestNoChannels,1.0,true,false);
	assert(TempSourceBuf);
	free(TempSourceBuf);
	assert(TempDestBuf);
	free(TempDestBuf);
}

//This is the ideal thing to do for mono; however we may want to disable for test purposes to the native simple mapping
#define __SupportMonoToStereoMix__

float *ChannelMixer::CreateDefaultVolumeMatrix(size_t SourceNoChannels,size_t DestNoChannels)
{

	size_t MatrixSizeInBytes=sizeof(float)*DestNoChannels*SourceNoChannels;
	float *pVolumeMatrix=(float *)malloc(MatrixSizeInBytes);
	memset(pVolumeMatrix,0,MatrixSizeInBytes);

	{
		//Now to make some intelligent default guess for the volume mix.. for now I have chosen a simple diagonal 1.0 across the channels
		//This works nice for a 2 : 2 and 4 : 2 mix down
		size_t x=0;
		if (DestNoChannels>SourceNoChannels)
		{
			#ifdef	__SupportMonoToStereoMix__
			if ((SourceNoChannels==1)&&(DestNoChannels>1))
			{
				//we mix down the mono to both channels
				pVolumeMatrix[0]=0.5f;
				pVolumeMatrix[1]=0.5f;
			}
			else
			#endif
			//Most likely in a mix up we'll want to only place the channels in a first come first serve basis, and keep the 
			//remaining channels in silence
			{
				for (size_t i=0;i<SourceNoChannels;i++)
				{
					pVolumeMatrix[(i*SourceNoChannels)+(x++)]=1.0f;
					if (x>=SourceNoChannels) x=0;
				}

			}
		}
		else
		{
			for (size_t i=0;i<SourceNoChannels;i++)
			{
				pVolumeMatrix[((x++)*SourceNoChannels)+i]=1.0f;
				if (x>=DestNoChannels) x=0;
			}
		}
		//TODO I may want to set up a default 5.1 and 6.1 mix down scenario... easy to do just need to find a standard we'll be using
	}
	return pVolumeMatrix;
}

void ChannelMixer::DestroyDefaultVolumeMatrix(float *VolumeMatrix)
{
	assert (VolumeMatrix);
	free(VolumeMatrix);
}

			} // namespace aud_convert
		} // namespace Base
	} // namespace Audio
}  //Namespace Devices
