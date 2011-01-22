#include "StdAfx.h"

#define AddAssetS( Name, Value )														\
	{	AssetElement* AE = NULL;														\
		pFileProperties->AddAsset( AE, Name, "String", true, true );					\
		if (AE)																			\
		{	AssetString *AES = GetInterface<AssetString>( AE->GetProperty() );			\
			AES->SetText( Value );														\
		}																				\
	}

#define AddAssetI( Name, IValue )														\
	{	char Value[128];																\
		sprintf( Value, "%d", IValue );													\
		AddAssetS( Name, Value );														\
	}

#define AddAssetF( Name, FValue )														\
	{	char Value[128];																\
		sprintf( Value, "%1.2f", FValue );												\
		AddAssetS( Name, Value );														\
	}

#define AddAssetTC( Name, IValue )														\
	{	char Value[128];																\
		ConvertTimecodeToString( Value, FrameRate, IValue );							\
		AddAssetS( Name, Value );														\
	}

bool WriteVideoFileAssets( const char *pFileName, const WMLib_FileInfoEx *pFormat )
{	// Write the format information
	FileProperties *pFileProperties=FileProperties::GetHandle( pFileName );
	if (!pFileProperties) return false;
	return WriteVideoFileAssets( pFileProperties, pFormat );
}

bool WriteVideoFileAssets( FileProperties *pFileProperties, const WMLib_FileInfoEx *_pFormat )
{	// Write the format information
	if (!pFileProperties) return false;
 
	WMLib_FileInfoEx Format = *_pFormat, *pFormat = &Format;

	// Exit quickly if it already looks like the assets exist
	if (pFileProperties->GetAsset( "Has Video" ))
		return true;

	// Timecode
	const float FrameRate = (float)pFormat->m_Video_FrameRate / (float)pFormat->m_Video_FrameScale;

	//If we have a video stream we can present the length in timecode
	if (pFormat->m_Video_Valid)
	{
		if (pFormat->m_Video_NoFrames>1)
		{ 
			AddAssetTC( "Length", ((WMLib_FileInfoEx*)pFormat)->GetNoFrames( pFormat->m_Video_FrameRate, pFormat->m_Video_FrameScale ) ); 
		}
		else	
			AddAssetS( "Length", "Still" );
	}
	else //If we do not have a video stream we can present the length in seconds with hundredths precision
		if ( pFormat->m_Audio_Valid )
	{
		AddAssetF("Length",	((double)((WMLib_FileInfoEx*)pFormat)->m_Audio_TotalTime) / 10000000.0);
	}

	// Create the assets
	if (pFormat->m_Video_Valid)
	{	AddAssetS( "Has Video", "True" );
		AddAssetI( "X Resolution", pFormat->m_Video_XRes );
		AddAssetI( "Y Resolution", pFormat->m_Video_YRes );
		AddAssetI( "No Frames", pFormat->m_Video_NoFrames );
		AddAssetS( "Still", (pFormat->m_Video_NoFrames==1) ? "True" : "False" );
		AddAssetF( "Frame-rate", (float)(pFormat->m_Video_FrameRate)/(float)(pFormat->m_Video_FrameScale) );
		AddAssetS( "Fielded", (pFormat->m_Video_IsFielded) ? "True" : "False" );
		AddAssetS( "Even Field First", (pFormat->m_Video_EvenFieldFirst) ? "True" : "False" );
		AddAssetS( "Has Alpha", pFormat->m_Alpha_Valid ? "True" : "False" );		

		// I cannot call these functions because of circular depenancies :(
		/*const char *pAspect = NULL;
		float dbest_Aspect = FLT_MAX;
		for( size_t i=0; i<RenderTree_AspectRatiosChoices::GetCount(); i++ )
		{	float this_Aspect  = RenderTree_AspectRatiosChoices::GetValue( i );
			float dthis_Aspect = fabs( this_Aspect-pFormat->m_Video_AspectRatio );
			if ((dthis_Aspect<dbest_Aspect)&&(dbest_Aspect<0.01))
			{	// Store the aspect
				dbest_Aspect = dthis_Aspect;
				// Store the string
				pAspect = RenderTree_AspectRatiosChoices::GetShortDescription( i );
			}
		}

		char Temp[128];
		if (!pAspect)
		{	sprintf( Temp, "%1.2f:1", this_Aspect-pFormat->m_Video_AspectRatio );
			pAspect = Temp;
		}

		// Store the aspect ratio name
		AddAssetS( "Aspect Ratio", pAspect );		*/

		// Try to guess the aspect ratio
		if (!pFormat->m_Video_AspectRatio)
			pFormat->m_Video_AspectRatio = pFormat->GuessAspectRatio( pFormat->m_Video_XRes, pFormat->m_Video_YRes, -1.0f );

		if ( fabs( pFormat->m_Video_AspectRatio - 4.0f/3.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "4:3" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 16.0f/9.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "16:9" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 16.0f/10.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "16:10" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 3.0f/2.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "3:2" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 5.0f/3.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "5:3" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 4.0f/6.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "4x6" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 5.0f/7.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "5x7" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 8.0f/10.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "8x10" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 6.0f/4.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "6x4" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 7.0f/5.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "7x5" ); }
		else if ( fabs( pFormat->m_Video_AspectRatio - 10.0f/8.0f ) < 0.01f )
				{ AddAssetS( "Aspect Ratio", "10x8" ); }
		//I'm not sure if anyone is using -1 for unknown aspect, but 0 is being used
		//  [2/14/2007 James]
		else if (( fabs( pFormat->m_Video_AspectRatio - -1.0f ) < 0.01f ) ||
				 ( fabs( pFormat->m_Video_AspectRatio - 0.0f ) < 0.01f ) 
				 )
				{ AddAssetS( "Aspect Ratio", "Not Available" ); }
		else
		{	char Temp[128];
			sprintf( Temp, "%1.2f:1", pFormat->m_Video_AspectRatio );
			AddAssetS( "Aspect Ratio", Temp );
		}
	}
	else	
	{	AddAssetS( "Has Video", "False" );
		AddAssetS( "X Resolution", "n/a" );
		AddAssetS( "Y Resolution", "n/a" );
		AddAssetS( "No Frames", "n/a" );
		AddAssetS( "Still", "n/a" );
		AddAssetF( "Frame-rate", "n/a" );
		AddAssetS( "Fielded", "n/a" );
		AddAssetS( "Even Field First", "n/a" );
		AddAssetS( "Has Alpha", "n/a" );
		AddAssetS( "Aspect Ratio", "n/a" );
	}

	if (pFormat->m_Audio_Valid)
	{	AddAssetS( "Has Audio", "True" );
		AddAssetI( "Sample Rate", pFormat->m_Audio_SampleRate );
		AddAssetI( "Channels", pFormat->m_Audio_NoChannels );
		if (pFormat->m_Audio_BitDepth==8)
				{ AddAssetS( "Sample Depth", "8 bit" ); }
		else if (pFormat->m_Audio_BitDepth==16)
				{ AddAssetS( "Sample Depth", "16 bit" ); }
		else if (pFormat->m_Audio_BitDepth==24)
				{ AddAssetS( "Sample Depth", "24 bit" ); }
		else if (pFormat->m_Audio_BitDepth==32)
				{ AddAssetS( "Sample Depth", "32 bit" ); }
		else if (pFormat->m_Audio_BitDepth==-32)
				{ AddAssetS( "Sample Depth", "Float" ); }
		else	{ AddAssetS( "Sample Depth", "?" ); }
	}
	else
	{	AddAssetS( "Has Audio", "False" );
		AddAssetS( "Sample Rate", "n/a" );
		AddAssetS( "Channels", "n/a" );
	}

	// Release
	pFileProperties->WriteInfoFile();
	FileProperties::ReleaseHandle( pFileProperties );

	// Success
	return true;
}