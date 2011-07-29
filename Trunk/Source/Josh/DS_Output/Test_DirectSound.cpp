#include "stdafx.h"
#include "XD300_Test.h"
#include "Test_DirectSound.h"

//#include "../../AudioTesting/Wave/Wave.h"

using namespace FrameWork::xml;
using namespace Modules::Hardware::DirectSound;
using namespace std;

const wchar_t * const cwsz_OutAudioName=L"mixer|input|10:frame";

  /*******************************************************************************************************/
 /*											DirectSoundTest												*/
/*******************************************************************************************************/

DirectSoundTest::DirectSoundTest(DirectSoundType type,const wchar_t *DirectSoundInputName,size_t ChannelInput) :
m_DS_StreamIn(NULL),m_DS_StreamOut(NULL),m_ConsoleType(type)
{
	//TODO enable if we have a global DS setup
	//assert((bool)m_DSInit);
	if (DirectSoundInputName)
		OpenResources(DirectSoundInputName,ChannelInput);
	else
		OpenResources();
}

DirectSoundTest::DirectSoundTest(const DirectSoundTest &NewValue) :
	m_DS_StreamIn(NULL),m_DS_StreamOut(NULL),m_ConsoleType(NewValue.m_ConsoleType)
{
	*this=NewValue;
}

inline DirectSoundTest& DirectSoundTest::operator =(const DirectSoundTest & NewValue)
{
	m_DS_StreamIn=NewValue.m_DS_StreamIn;
	m_DS_StreamOut=NewValue.m_DS_StreamOut;
	const_cast<DirectSoundType>(m_ConsoleType)=const_cast<DirectSoundType>(NewValue.m_ConsoleType);
	m_DirectSoundName=NewValue.m_DirectSoundName;
	return *this;
}


void DirectSoundTest::CloseResources()
{
	delete m_DS_StreamIn;
	m_DS_StreamIn=NULL;

	delete m_DS_StreamOut;
	m_DS_StreamOut=NULL;
}

DirectSoundTest::~DirectSoundTest()
{
	CloseResources();
}


void DirectSoundTest::DisplayHelp()
{
	printf(
		"reset\n"
		"cls\n"
		"AddStream <Name> <channel>\n"
		"Remove <Name>\n"
		"Switch <Name>\n"
		"SetAudRate <0=48 1=96 2=192>\n"
		"Start\n"
		"Stop\n"
		"addto <DestName> \n"
		"LoopThru\n"
		);

	switch(m_ConsoleType)
	{
		case eDirectSound_in:
			printf(
				"SetAudioIn <Input 0-13 (-1=none)>\n"
				"gain <ratio> <channel> [use channel=0]\n"
				"DCFilter <1=filter>\n"
				);
			break;
		case eDirectSound_out:
			printf(
				"SetAudioOut <Input 0-13 (-1=none)>\n"
				"Display <1=enable>\n"
				"addout <Chan 0-11> \n"
				);
			break;
	}
	printf(
		"\n"
		"Help  (displays this)\n"
		"Type \"Exit\" at anytime to exit to main\n"
		"Type \"Quit\" at anytime to exit this application\n"
		);
}

#undef __TestDDR_Port_DeviceSend__

void DirectSoundTest::OpenResources(const wchar_t DirectSoundInputName[], const  size_t ChannelInput)
{
	// TODO get the static instance of Direct Sound
	//Configuration_Manager &config=Configuration_Manager::GetConfigurationManager();

	#if 0
	//Note: this works assuming that there will never be more audio rows than video; if this changes we'll need to tweak the logic
	size_t No_Rows_Audio=(m_ConsoleType==eDirectSound_out)?HD_Output2::GetNo_Rows_Audio():HD_Input2::GetNo_Rows_Audio();
	assert(No_Rows_Video>=No_Rows_Audio);
	assert(ChannelInput<No_Rows_Video);
	AudioOutputEnum AudioOutputRow=(AudioOutputEnum)(ChannelInput+1);

	//Ensure there is an audio row in hardware
	if (ChannelInput >= No_Rows_Audio)
		AudioOutputRow=eAudioNone;
	#else
	size_t AudioOutputRow=0;
	#endif

	m_DirectSoundName=DirectSoundInputName;
	SetConsoleTitle(DirectSoundInputName);
	switch (m_ConsoleType)
	{
		case eDirectSound_in:
			if (!m_DS_StreamIn)
			{
				#if 0
				try
				{
					//const size_t DMAChannel=1;
					//m_StreamIn=new HD_Input2((HD_Input2::VideoInputEnum)ChannelInput,HD_Input2::eAudioNone,DirectSoundInputName,NULL,DMAChannel);

					//This works where both the DMA and Register are the same number
					m_DS_StreamIn=new DS_Input((HD_Input2::VideoInputEnum)(ChannelInput+1),(HD_Input2::AudioInputEnum)(ChannelInput+1),DirectSoundInputName,NULL,NULL,0,BypassI2C,HD_Input2::eLockToOutput,pVideoFormatInfo);
					//Here we can test to lock or unlock (probably will change with new hardware)
					//m_DS_StreamIn->SetLockAudioToVideo(false);
					m_DS_StreamIn->StartStreaming();
				}
				catch (char *ErrorMsg)
				{
					printf("\n\n\nXML_DirectSoundTest::OpenResources failed\n %s\n\n\n",ErrorMsg);
					m_DS_StreamIn=NULL;
				}
				#else
				assert (false);
				#endif
			}
			break;
		case eDirectSound_out:
			if (!m_DS_StreamOut)
			{
				try
				{
					//For testing I'd rather it not manage the silence
					m_DS_StreamOut=new DS_Output(AudioOutputRow,DirectSoundInputName);
					m_DS_StreamOut->StartStreaming();
				}
				catch (char *ErrorMsg)
				{
					printf("\n\n\nXML_DirectSoundTest::OpenResources failed\n %s\n\n\n",ErrorMsg);
					m_DS_StreamOut=NULL;
				}
			}

	}
}

void DirectSoundTest::OpenResources()
{
	const wchar_t *DirectSoundInputName=NULL;
	size_t channel=0;
	if (m_ConsoleType==eDirectSound_in)
	{
		DirectSoundInputName=L"in0",channel=0;
		//channel=(size_t)HD_Input2::eTestPattern1;
	}
	else if (m_ConsoleType==eDirectSound_out)
	{
		DirectSoundInputName=cwsz_OutAudioName,channel=0;
	}
	OpenResources(DirectSoundInputName,channel);
}


size_t DirectSoundTest::GetGhannelInput() const
{
	size_t ChannelInput=0;
	if (m_ConsoleType==eDirectSound_in)
	{
		ChannelInput=m_DS_StreamIn->GetAudioInputChannel();
	}
	else if (m_ConsoleType==eDirectSound_out)
	{
		ChannelInput=m_DS_StreamOut->GetAudioOutputChannel();
	}
	return ChannelInput;
}

//Allow ability to run multiple inputs and outputs on the same console!
class DirectSoundTest_IOSwitcher
{
	public:
		void Flush()
		{
			//delete all open input and outputs
			for (size_t i=0;i<m_XML_DirectSoundTest_List.size();i++)
				delete m_XML_DirectSoundTest_List[i];
			m_XML_DirectSoundTest_List.clear();
		}
		~DirectSoundTest_IOSwitcher()
		{
			Flush();
		}
		void Add(DirectSoundType type,const wchar_t *DirectSoundInputName,size_t ChannelInput)
		{
			DirectSoundTest *Instance=new DirectSoundTest(type,DirectSoundInputName,ChannelInput);
			m_XML_DirectSoundTest_List.push_back(Instance);
		}
		void Remove(const wchar_t *DirectSoundInputName)
		{
			for (size_t i=0;i<m_XML_DirectSoundTest_List.size();i++)
			{
				if (wcscmp(m_XML_DirectSoundTest_List[i]->m_DirectSoundName.c_str(),DirectSoundInputName)==0)
				{
					delete m_XML_DirectSoundTest_List[i];
					m_XML_DirectSoundTest_List[i]=NULL; //pedantic
					// Set this item to the last item
					m_XML_DirectSoundTest_List[i] = m_XML_DirectSoundTest_List[ m_XML_DirectSoundTest_List.size()-1 ];
					// Reduce the list length
					m_XML_DirectSoundTest_List.resize( m_XML_DirectSoundTest_List.size()-1 );
				}
			}
		}
		bool Switch(const wchar_t *DirectSoundInputName,DirectSoundTest *FromThis)
		{
			bool ret=false;
			for (size_t i=0;i<m_XML_DirectSoundTest_List.size();i++)
			{
				if (wcscmp(m_XML_DirectSoundTest_List[i]->m_DirectSoundName.c_str(),DirectSoundInputName)==0)
				{
					//swap((*(m_XML_DirectSoundTest_List[i])),*FromThis);
					DirectSoundTest &Left = *(m_XML_DirectSoundTest_List[i]);
					DirectSoundTest &Right = *FromThis;

					//_Ty _Tmp = _Left;
					DS_Input *HDStreamIn=Left.m_DS_StreamIn;
					DS_Output *HDStreamOut=Left.m_DS_StreamOut;
					DirectSoundType ConsoleType=const_cast<DirectSoundType>(Left.m_ConsoleType);
					wstring HardwareName=Left.m_DirectSoundName;

					Left = Right;
					//_Right = _Tmp;
					Right.m_DS_StreamIn=HDStreamIn;
					Right.m_DS_StreamOut=HDStreamOut;
					const_cast<DirectSoundType>(Right.m_ConsoleType)=ConsoleType;
					Right.m_DirectSoundName=HardwareName;
					ret=true;
					break;
				}
			}
			return ret;
		}
		DirectSoundTest *Query (const wchar_t *DirectSoundInputName)
		{
			DirectSoundTest *ret;
			for (size_t i=0;i<m_XML_DirectSoundTest_List.size();i++)
			{
				if (wcscmp(m_XML_DirectSoundTest_List[i]->m_DirectSoundName.c_str(),DirectSoundInputName)==0)
				{
					ret=m_XML_DirectSoundTest_List[i];
					break;
				}
			}
			return ret;
		}
	private:
		std::vector<DirectSoundTest *>	m_XML_DirectSoundTest_List; 
} g_DirectSoundTest_IOSwitcher;


bool DirectSoundTest::CommandLineInterface()
{
	size_t FileSelection=InitFileSelection();
	//Configuration_Manager &config=Configuration_Manager::GetConfigurationManager();
	cout << endl;
	ShowCurrentSelection();
	cout << endl;
	DisplayHelp();
 	cout << "Ready." << endl;

	string AppName;
	{
		wchar2char(m_DirectSoundName.c_str());
		AppName=wchar2char_pchar;
	}
	size_t ChannelInput;

   	char input_line[128];
    while (cout << AppName << "[" << (ChannelInput=GetGhannelInput()) << "]" << " >",cin.getline(input_line,128))
    {
		char		command[32];
		char		str_1[64];
		char		str_2[64];
		char		str_3[64];
		char		str_4[64];

		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;

		if (FillArguments(input_line,command,str_1,str_2,str_3,str_4)>=1) 
		{
			if (!_strnicmp( input_line, "cls", 3))
				cls();
			else if (!_strnicmp( input_line, "AddStream", 6))
			{	//AddStream <Name> <channel>
				char2wchar(str_1);
				g_DirectSoundTest_IOSwitcher.Add(m_ConsoleType,char2wchar_pwchar,atoi(str_2));
				//Go ahead and switch to it
				g_DirectSoundTest_IOSwitcher.Switch(char2wchar_pwchar,this);
				AppName=str_1;
				ChannelInput=GetGhannelInput();
			}
			else if (!_strnicmp( input_line, "Remove", 6))
			{	//Remove <Name>
				char2wchar(str_1);
				g_DirectSoundTest_IOSwitcher.Remove(char2wchar_pwchar);
			}
			else if (!_strnicmp( input_line, "LoopThru", 7))
			{
				if (m_ConsoleType==eDirectSound_out)
				{
					wchar_t inputname[8];
					wsprintfW(inputname,L"in%d",ChannelInput);
					printf("Adding Input %ls\n",inputname);
					g_DirectSoundTest_IOSwitcher.Add(eDirectSound_in,inputname,ChannelInput);
					DirectSoundTest *input=g_DirectSoundTest_IOSwitcher.Query(inputname);
					assert(input);
					input->m_DS_StreamIn->add_dest(m_DirectSoundName.c_str());
				}
			}
			else if (!_strnicmp( input_line, "Switch", 6))
			{	//Switch <Name>
				char2wchar(str_1);
				if (g_DirectSoundTest_IOSwitcher.Switch(char2wchar_pwchar,this))
				{
					AppName=str_1;
					ChannelInput=GetGhannelInput();
				}
				else
					printf("Error %s not found\n",str_1);
			}
			//Use this to flush resources
			else if (!_strnicmp( input_line, "reset", 5))
			{
				g_DirectSoundTest_IOSwitcher.Flush();
				CloseResources();
				OpenResources();
			}
			else if (!_strnicmp( input_line, "addto", 5))
			{
				if  (str_1[0])
				{
					char2wchar(str_1);
					if (m_DS_StreamIn)
						m_DS_StreamIn->add_dest(char2wchar_pwchar);
					else if (m_DS_StreamOut)
						m_DS_StreamOut->add_thru_dest(char2wchar_pwchar);
 				}
			}
			else if (!_strnicmp( input_line, "DCFilter", 3))
			{
				if (m_DS_StreamIn)
					m_DS_StreamIn->SetUseDCFilter(str_1[0]=='1');
				else
					printf("Doh!, This only works for input\n");
			}
			else if (!_strnicmp( input_line, "addout", 6))
			{
				if (m_DS_StreamOut)
				{
					m_DS_StreamOut->StopStreaming();
					m_DS_StreamOut->AddOutputChannel(atoi(str_1));
					m_DS_StreamOut->StartStreaming();
				}
			}

			else if (!_strnicmp( input_line, "SetAudioIn", 10))
			{
				if (m_DS_StreamIn)
					m_DS_StreamIn->ChangeAudioFormat(atoi(str_1));
			}

			else if (!_strnicmp( input_line, "SetAudioOut", 11))
			{
				if (m_DS_StreamOut)
				{
					m_DS_StreamOut->StopStreaming();
					m_DS_StreamOut->SetAudioOutputChannel(atoi(str_1));
					m_DS_StreamOut->StartStreaming();
				}
			}
			else if (!_strnicmp( input_line, "Display", 7))
			{
				if (m_DS_StreamOut)
					m_DS_StreamOut->SetUseDisplayedCallbacks(str_1[0]!='0');
			}

			else if (!_strnicmp( input_line, "start", 5))
			{
				if (m_DS_StreamIn)
					m_DS_StreamIn->StartStreaming();
				else if (m_DS_StreamOut)
					m_DS_StreamOut->StartStreaming();
				else
					assert(false);
			}
			else if (!_strnicmp( input_line, "stop", 5))
			{
				if (m_DS_StreamIn)
					m_DS_StreamIn->StopStreaming();
				else if (m_DS_StreamOut)
					m_DS_StreamOut->StopStreaming();
				else
					assert(false);
			}

			else if (!_strnicmp( input_line,"SetAudRate", 7))
			{
				assert(false);
				if (m_DS_StreamIn)
				{
					m_DS_StreamIn->StopStreaming();
					//TODO
					m_DS_StreamIn->StartStreaming();
				}
				else if (m_DS_StreamOut)
				{
					//TODO
					//m_DS_StreamOut->ChangeFormat(eAudioOut,info.Format);
				}
			}
			else if (!_strnicmp( input_line, "gain", 4))
			{
				printf("not yet supported\n");
			}

			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Exit", 4))
			{
				g_DirectSoundTest_IOSwitcher.Flush();
				break;
			}
			else if (!_strnicmp( input_line, "Quit", 4))
			{
				g_DirectSoundTest_IOSwitcher.Flush();
				return true;
			}
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
	return false;  //just exit
}


