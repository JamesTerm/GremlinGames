#pragma once


enum DirectSoundType
{
	eDirectSound_in,
	eDirectSound_out,
	eDirectSound_NoTypes
};

class DirectSoundTest_IOSwitcher;
class DirectSoundTest
{
	public:
		DirectSoundTest(DirectSoundType type=eDirectSound_in,const wchar_t *DirectSoundInputName=NULL,size_t ChannelInput=-1);
		virtual ~DirectSoundTest();

		DirectSoundTest(const DirectSoundTest &NewValue);
		//The assignment operator
		inline DirectSoundTest &operator= (const DirectSoundTest &NewValue);

		//returns true to quit
		bool CommandLineInterface();
	private:
		friend DirectSoundTest_IOSwitcher;
		void DisplayHelp();
		void CloseResources();
		void OpenResources();
		void OpenResources(const wchar_t DirectSoundInputName[],const size_t ChannelInput);
		size_t GetGhannelInput() const;

		Modules::Hardware::DirectSound::DirectSound_Initializer m_DSInit;
		//making this a pointer so that I can flush during tests
		Modules::Hardware::DirectSound::DS_Input *m_DS_StreamIn;
		Modules::Hardware::DirectSound::DS_Output *m_DS_StreamOut;
		const DirectSoundType m_ConsoleType;
		std::wstring m_DirectSoundName;
};

