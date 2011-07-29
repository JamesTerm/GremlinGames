#include "stdafx.h"
#include "Module.Hardware.DirectSound.Output.h"

using namespace Modules::Hardware::DirectSound;
using namespace FrameWork::Communication3;
using namespace FrameWork::Threads;
using namespace std;

extern "C" __declspec(dllexport) bool newtek_launch( const wchar_t *p_parameter_xml )
{	
	bool ret = false;

	// Start HW
	DirectSound_Initializer	ds_init;

	//name this thread
	FrameWork::Work::set_thread_name( "DirectSound.Output n_l thread" );

	// Launch and wait
	ret = OutputModule( p_parameter_xml, ds_init.GetDirectSoundInitStatus() ).wait();

	return ret;
}



  /*******************************************************************************************************/
 /*												OutputModule											*/
/*******************************************************************************************************/

OutputModule::OutputModule( const wchar_t *p_parameter_xml, bool DS_Supported ) : 
	m_exit_event( false ),m_DS_Supported(DS_Supported),m_DefaultConfig_Parsed(false)
{	
	// No frames yet
	// Parse the input parameters
	FXML::tree		data( p_parameter_xml );

	// Look for the names
	for( int i=0; i<data.no_children(); i++ )
	{	
		// Get the child node
		const FXML::tree &item = data.child( i );
		deliver_xml( &xml::message( item ) );

		// Get the type
		const wchar_t *p_type = item.type();
		if ( !p_type ) continue;

		// Get the category
		const wchar_t *p_categ = item.parameter( L"type", L"" );

		// Get the name
		const wchar_t *p_name = item.parameter( L"name", L"" );

		// Look at what this is
		if ( ! ::wcscmp( p_type, L"input" ) )
		{	     
			if ( ! ::wcscmp( p_categ, L"command" ) )	m_input_command_name = p_name;
			// ADJC was here
			else if ( !::wcscmp( p_categ, L"frame" ) )
			{	// Get the number of inputs
				const int no_inputs = ::_wtoi( item.parameter( L"num", L"1" ) );

				// Cycle across all inputs
				for( int i=0; i<no_inputs; i++ )
				{	// Create a new row
					row NewRow;
					NewRow.p_output=NULL;

					// Build the names						
					wchar_t	audio_name[ 128 ];
					::swprintf( audio_name, L"%s|%d:frame", p_name, i );

					// Assign the names
					NewRow.audio_frame_server_name = audio_name;

					// Add it to the list
					m_Outputs.push_back( NewRow );
				}
			}
		}
		
		else if ( ! ::wcscmp( p_type, L"output" ) )   // Is this an output node ?
		{	     
			if ( ! ::wcscmp( p_categ, L"response" ) )	
				m_output_response_name = p_name;

		}
	}

	// Start the server
	assert( m_input_command_name[0] );
	m_p_command_server = new FC3::xml::receive( m_input_command_name.c_str(), this );
}


OutputModule::~OutputModule( void )
{
	if ( m_p_command_server )
	{
		delete m_p_command_server;
		m_p_command_server=NULL;
	}

	//stop output
	for( size_t i=0; i < m_Outputs.size(); i++ )
	{
		delete m_Outputs[i].p_output;
		m_Outputs[i].p_output=NULL;
	}
	m_Outputs.clear();
}

// Wait for it to exit
bool OutputModule::wait( const DWORD timeout )
{	// Wait for the time to elapse
	return m_exit_event.wait( timeout );
}

const wchar_t * const cwsz_SDI_Audio_Out=L"SDI_Audio_Out";

void OutputModule::ParseFormatChanges(const FXML::node2 &xml_tree)
{
	auto_lock block( m_block_deliver_xml );

	for( int i=0; i< xml_tree.no_children(); i++)
	{
		const FXML::node2 &input_child = xml_tree.child(i);

		//TODO determine what we want to do here
		#if 0
		if( !::wcscmp( input_child.type(), L"audio" ) )
		{
			size_t row_index = ::_wtoi( input_child.parameter( L"number" ) );
			if (row_index>=m_Outputs.size())
			{
				assert(false);  //output number should not exceed the number of outputs specified
				return;
			}

			StreamInformation &AudInfo=m_Outputs[row_index].audio_streaminfo;
			AudioFormat &audformat=AudInfo.Format.FormatSelection.AudioValue;

			//frequency, 48000, 96000 or 192000
			switch( ::_wtol(input_child.parameter( L"sample_rate", L"96000" ) ) )
			{
			case 48000:		audformat.SampleRate = AudioFormat::e48000;		break;
			case 96000:		audformat.SampleRate = AudioFormat::e96000;		break;
			case 192000:	audformat.SampleRate = AudioFormat::e192000;	break;
			}
		}
		#endif
	}	

	//if (m_DS_Supported)
	//	UpdateHardware();
}

void OutputModule::deliver_xml( const xml::message *p_msg )
{
	// Build an xml tree from the message
	const FXML::tree2 xml_tree( (const wchar_t*)*p_msg );

	if( !xml_tree.type() || m_exit_event )
		return;

	// Is this an exit message
	if ( !::wcscmp( xml_tree.type(), L"exit" ) )
	{	// This was an exit message
		m_exit_event = true;
	}
	else if((wcscmp( xml_tree.type(), L"output_config" )==0) || (wcscmp(xml_tree.type(),L"outputs_video")==0) )
	{
		//async_debug_output(p_debug_category,L"OutputModule::deliver_xml:\n%s",(const wchar_t*)*p_msg);
		ParseFormatChanges(xml_tree);
	}
	else if (m_DefaultConfig.Parse(xml_tree))
		m_DefaultConfig_Parsed=true;
}
