#pragma once

class OutputModule :	private FrameWork::Communication3::xml::receive::client
{	
	public:
		OutputModule( const wchar_t *p_xml_data, bool DS_Supported  );
		~OutputModule( void );

		bool wait( const DWORD timeout = INFINITE );
	protected:		
		// both runtime and launch scripts will be parsed here
		virtual void deliver_xml( const FrameWork::Communication3::xml::message *p_msg );

	private:
		//This will implicitly call to update the hardware
		void ParseFormatChanges(const FXML::node2 &xml_tree);

		HD::Base::ConfigParser m_DefaultConfig;
		// These are the object names
		std::wstring m_input_command_name;
		std::wstring m_input_frame_name;
		std::wstring m_output_response_name;
		// the xml receive server and ptr to it's allocated string
		FC3::xml::receive *m_p_command_server;

		struct row
		{
			std::wstring audio_frame_server_name;
			DS_Output *p_output;

			//StreamInformation audio_streaminfo;
		};

		std::vector<row> m_Outputs;

		// The exit event
		FrameWork::Threads::event m_exit_event;
		FrameWork::Threads::critical_section m_block_deliver_xml;

		const bool m_DS_Supported;
		bool m_DefaultConfig_Parsed;
};
