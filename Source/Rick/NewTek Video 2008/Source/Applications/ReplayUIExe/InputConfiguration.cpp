#include "stdafx.h"
#include "ReplayUIExe.h"
#include "RootAppNode.h"

using namespace Views::Replay;

// Some constants to work with
static const float BRIGHTNESS_MIN = -50.0f;
static const float BRIGHTNESS_MAX = +50.0f;
static const float BRIGHTNESS_DEF = 0.0f;
static const float CONTRAST_MIN = 0.0f;
static const float CONTRAST_MAX = 200.0f;
static const float CONTRAST_DEF = 100.0f;
static const float HUE_MIN = -180.0f;
static const float HUE_MAX = +180.0f;
static const float HUE_DEF = 0.0f;
static const float SATURATION_MIN = 0.0f;
static const float SATURATION_MAX = 200.0f;
static const float SATURATION_DEF = 100.0f;

// Not sure about these limits and defaults yet
static const float AUDIO_MIN_UI = 0.0f;	
static const float AUDIO_MAX_UI = 1.0f;
static const float AUDIO_DEF_UI = 0.2f;

// These will be populated in the class
static float AUDIO_MIN_DB = 0.0f;	
static float AUDIO_MAX_DB = 1.0f;
static float AUDIO_DEF_DB = 0.2f;

static const int   TERMINATION_DEF = 1;
static const float VERT_MIN = -1.0f;
static const float VERT_MAX =  1.0f;
static const float VERT_DEF =  0.0f;
static const float HORIZ_MIN = -1.0f;
static const float HORIZ_MAX =  1.0f;
static const float HORIZ_DEF =  0.0f;
static const float PHASE_MIN = -1.0f;
static const float PHASE_MAX =  1.0f;
static const float PHASE_DEF =  0.0f;

// These are the different Video Configurations
static const char* VIDEO_INPUT_STRINGS[4] =  {"Composite",	                        "Y/C",                         "Component",		                        "SDI" };
static const unsigned SD_VIDEO_INPUT_OPTS[5]={InputConfigModel::VI_COMPOSITE, InputConfigModel::VI_YC, InputConfigModel::VI_COMPONENT,	InputConfigModel::VI_SDI, (unsigned)-1};
static const unsigned HD_VIDEO_INPUT_OPTS[3]={InputConfigModel::VI_COMPONENT,	InputConfigModel::VI_SDI, (unsigned)-1};
static const unsigned SD_VIDEO_OUTPUT_OPTS[4]={InputConfigModel::VI_COMPOSITE, InputConfigModel::VI_YC, InputConfigModel::VI_COMPONENT,	(unsigned)-1};
static const unsigned HD_VIDEO_OUTPUT_OPTS[2]={InputConfigModel::VI_COMPONENT,	(unsigned)-1};
static const int	VIDEO_TYPE_DEF = InputConfigModel::VI_COMPONENT;

// These are the different Audio Configurations
static const char* AUDIO_INPUT_STRINGS[3] =	{  "Analog Audio",                       "Digital Audio",                       "SDI Embedded Audio" };
static const unsigned SDI_AUDIO_INPUT_OPTS[4]={InputConfigModel::AUDIO_ANALOG, InputConfigModel::AUDIO_DIGITAL, InputConfigModel::AUDIO_SDI, (unsigned)-1};
static const unsigned NONSDI_AUDIO_INPUT_OPTS[3]={InputConfigModel::AUDIO_ANALOG, InputConfigModel::AUDIO_DIGITAL, (unsigned)-1};
static const int AUDIO_TYPE_DEF = InputConfigModel::AUDIO_ANALOG;

// Make a single global DB Converter that everyone here uses
dBConversion DBConverter;

SingleInputConfigModel::SingleInputConfigModel(unsigned inputNum) : 
	m_waitToSend(eNotWaiting), m_inputNumber(inputNum),
	Brightness(BRIGHTNESS_DEF), m_oldBrightness(BRIGHTNESS_DEF),
	Hue(HUE_DEF), m_oldHue(HUE_DEF),
	Contrast(CONTRAST_DEF), m_oldContrast(CONTRAST_DEF),
	Saturation(SATURATION_DEF), m_oldSaturation(SATURATION_DEF),
	AudioLevel_UI(AUDIO_DEF_UI), m_oldAudioLevel_UI(AUDIO_DEF_UI),
	Terminate(TERMINATION_DEF), m_oldTerminate(TERMINATION_DEF),
	VideoMode(VIDEO_TYPE_DEF), m_oldVideoInput(VIDEO_TYPE_DEF),
	AudioMode(AUDIO_TYPE_DEF), m_oldAudioInput(AUDIO_TYPE_DEF)
{
	// Populate the Audio_DB level
	AudioLevel_DB.Set(DBConverter.ConvertUITodBValue(AudioLevel_UI.Get()));
	m_oldAudioLevel_DB = AudioLevel_DB.Get();

	Brightness.AddDependant(this);
	Hue.AddDependant(this);
	Contrast.AddDependant(this);
	Saturation.AddDependant(this);
	AudioLevel_UI.AddDependant(this);
	AudioLevel_DB.AddDependant(this);
	Terminate.AddDependant(this);
	VideoMode.AddDependant(this);
	AudioMode.AddDependant(this);

	m_configLine = new char[512];
	m_configLineW = new wchar_t[512];
}
//////////////////////////////////////////////////////////////////////////

SingleInputConfigModel::~SingleInputConfigModel()
{
	delete[] m_configLine;
	delete[] m_configLineW;
}
//////////////////////////////////////////////////////////////////////////

// This is a function available for all inputs and outputs
unsigned* SingleInputConfigModel::GetVideoOptions()
{
	// Handle based on the Video Resolution being used in Replay
	NT2005::Config::eVidDisplay d = Application::Replay::UI::get_instance().get_video_mode_desc().m_eVidDisplay;
	if ((d == NT2005::Config::eVidDisplay_SD) || (d == NT2005::Config::eVidDisplay_WideSD))
		return (unsigned*)SD_VIDEO_INPUT_OPTS;
	else
		return (unsigned*)HD_VIDEO_INPUT_OPTS;
}
//////////////////////////////////////////////////////////////////////////

unsigned* SingleInputConfigModel::GetAudioOptions()
{
	// Return based on the current video option
	return (unsigned*)((VideoMode.Get()==InputConfigModel::VI_SDI) ? SDI_AUDIO_INPUT_OPTS : NONSDI_AUDIO_INPUT_OPTS);
}
//////////////////////////////////////////////////////////////////////////


// Call to set all of the 
void SingleInputConfigModel::RestoreDefaults()
{
	m_waitToSend = eWaitingNoChanges;
	Brightness=(BRIGHTNESS_DEF); 
	Hue=(HUE_DEF); 
	Contrast=(CONTRAST_DEF); 
	Saturation=(SATURATION_DEF); 
	AudioLevel_UI=(AUDIO_DEF_UI);

	if (m_waitToSend == eWaitingChanges)
		UpdateXML();
	m_waitToSend = eNotWaiting;
}
//////////////////////////////////////////////////////////////////////////


// Here is where the metal hits the road, sending the XML
void SingleInputConfigModel::UpdateXML()
{
	// Here is an example of what this output might look like:
	// <Modules.HW.Input.Video.1 VideoConnection="Y/C" AudioConnection="Digital Audio" Procamp_Brightness="0.0" Procamp_Contrast="1.0" Procamp_Hue="0.0" Procamp_Saturation="1.0" AudioLevelRatio="1.0" />

	// The Audio Level Ratio requires a bit more work
	float UILevel=min(max(AudioLevel_UI.Get(),0.0),1.0); //! Clamps [0-1], The min and max should have already got us there
	float MainInputVolumeRatio=DBConverter.ConvertUIToRatioValue(UILevel);

	//Note: m_inputNumber works with 0-2 range, but the XML works with 1-3
	::sprintf(m_configLine, "<Modules.HW.Input.Video.%d VideoConnection=\"%s\" AudioConnection=\"%s\" Procamp_Brightness=\"%f\" Procamp_Contrast=\"%f\" Procamp_Hue=\"%f\" Procamp_Saturation=\"%f\" AudioLevelRatio=\"%f\" />", 
		m_inputNumber+1, VIDEO_INPUT_STRINGS[VideoMode.Get()], AUDIO_INPUT_STRINGS[AudioMode.Get()], 
		Brightness.Get()/100.0f, Contrast.Get()/100.0f, Hue.Get(), Saturation.Get()/100.0f, MainInputVolumeRatio);

	::wsprintfW(m_configLineW, L"%hS", m_configLine);

	// And tell the application to send this line
	Application::UI::Base::send_xml(m_configLineW);
}
//////////////////////////////////////////////////////////////////////////

void SingleInputConfigModel::ValidateVideoMode()
{
	unsigned* videoInputOpts = GetVideoOptions();
	unsigned i = 0;
	unsigned curr = (unsigned)VideoMode.Get();
	while (videoInputOpts[i] != (unsigned)-1)
	{
		if (curr == videoInputOpts[i])
		{	
			// Watch out for having inappropriate Audio (e.g. we cannot have SDI audio if not SDI Video
			WaitingForChanges waitToSend = m_waitToSend;	// Remember the waiting state
			m_waitToSend = eWaitingChanges;					// We do not want to allow the audio validation to send the XML yet, just validate
			ValidateAudioMode();							// This might set the waiting flag
			m_waitToSend = waitToSend;						// Set it back to what it was, we will handle sending below

			// Send it along if we are not waiting
			if (m_waitToSend == eNotWaiting) 
				UpdateXML();
			else m_waitToSend = eWaitingChanges;

			// All done
			return;
		}
		++i;
	}
	// We have an invalid input, set it back to def. This should call the DynamicCallback again I think
	VideoMode = VIDEO_TYPE_DEF;
}
//////////////////////////////////////////////////////////////////////////

void SingleInputConfigModel::ValidateAudioMode()
{
	unsigned* audioInputOpts = GetAudioOptions();
	unsigned i = 0;
	unsigned curr = (unsigned)AudioMode.Get();
	while (audioInputOpts[i] != (unsigned)-1)
	{
		if (curr == audioInputOpts[i])
		{
			// We have a valid input, send it if we can
			if (m_waitToSend == eNotWaiting) 
				UpdateXML();
			else m_waitToSend = eWaitingChanges;
			return;
		}
		++i;
	}
	// We have an invalid input, set it back to def. This should call the DynamicCallback again I think
	AudioMode = AUDIO_TYPE_DEF;
}
//////////////////////////////////////////////////////////////////////////

#define CHECK_VALS(lwr, mmin, mmax, old) if ((p_item_changing == &lwr) && (lwr!=old))		\
{	if (lwr < mmin) { lwr = mmin; }		\
	else if (lwr > mmax) { lwr = mmax; } \
	else { old=lwr; if (m_waitToSend == eNotWaiting){ UpdateXML();	}	\
	else {m_waitToSend = eWaitingChanges; } }\
	return; }

// Verifies the values and sends them
void SingleInputConfigModel::DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing)
{
	CHECK_VALS(Brightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX, m_oldBrightness)	
	CHECK_VALS(Hue, HUE_MIN, HUE_MAX, m_oldHue)
	CHECK_VALS(Contrast, CONTRAST_MIN, CONTRAST_MAX, m_oldContrast)
	CHECK_VALS(Saturation, SATURATION_MIN, SATURATION_MAX, m_oldSaturation)

	if ((p_item_changing == &AudioLevel_UI) && (AudioLevel_UI != m_oldAudioLevel_UI))
	{
		if (AudioLevel_UI < AUDIO_MIN_UI) { AudioLevel_UI = AUDIO_MIN_UI; }	
		else if (AudioLevel_UI > AUDIO_MAX_UI) { AudioLevel_UI = AUDIO_MAX_UI; }
		else { 
			m_oldAudioLevel_UI=AudioLevel_UI; 

			// Update the other as well, be sure not to get another callback
			this->ReceiveMessagesOff();
			AudioLevel_DB.Set(DBConverter.ConvertUITodBValue(AudioLevel_UI.Get()));
			m_oldAudioLevel_DB = AudioLevel_DB.Get();
			this->ReceiveMessagesOn();

			if (m_waitToSend == eNotWaiting) { 
				UpdateXML();
			}
			else {
				m_waitToSend = eWaitingChanges; 
			} 
		}
		return;
	}

	if ((p_item_changing == &AudioLevel_DB) && (AudioLevel_DB != m_oldAudioLevel_DB))
	{
		if (AudioLevel_DB < AUDIO_MIN_DB) { AudioLevel_DB = AUDIO_MIN_DB; }	
		else if (AudioLevel_DB > AUDIO_MAX_DB) { AudioLevel_DB = AUDIO_MAX_DB; }
		else { 
			m_oldAudioLevel_DB=AudioLevel_DB; 

			// Update the other as well, be sure not to get another callback
			this->ReceiveMessagesOff();
			AudioLevel_UI.Set(DBConverter.ConertdBToUIValue(AudioLevel_DB.Get()));
			m_oldAudioLevel_UI = AudioLevel_UI.Get();
			this->ReceiveMessagesOn();

			if (m_waitToSend == eNotWaiting) { 
				UpdateXML();
			}
			else {
				m_waitToSend = eWaitingChanges; 
			} 
		}
		return;
	}
	
	if ((p_item_changing == &Terminate) && (Terminate != m_oldTerminate))
	{
		m_oldTerminate = Terminate;
		// No range validation here, just a 0 or 1 will be sent
		if (m_waitToSend == eNotWaiting) 
			UpdateXML();
		else m_waitToSend = eWaitingChanges;
		return;
	}

	if ((p_item_changing == &VideoMode) && (VideoMode != m_oldVideoInput))
	{
		m_oldVideoInput = VideoMode;
		ValidateVideoMode();
		return;
	}

	if ((p_item_changing == &AudioMode) && (AudioMode != m_oldAudioInput))
	{
		m_oldAudioInput = AudioMode;
		ValidateAudioMode();
		return;
	}

	// I did not handle it, so call the base class
	__super::DynamicCallback(id, p_string, p_args, p_item_changing);
}
//////////////////////////////////////////////////////////////////////////

// BaseLoadSave
void SingleInputConfigModel::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{
	MajorVersion = 1;
	MinorVersion = 1;
}
//////////////////////////////////////////////////////////////////////////

bool SingleInputConfigModel::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	if (!SaveContext)
		return false;

	if (!SaveContext->Put<float>(Brightness.Get())) return false;
	if (!SaveContext->Put<float>(Hue.Get())) return false;
	if (!SaveContext->Put<float>(Contrast.Get())) return false;
	if (!SaveContext->Put<float>(Saturation.Get())) return false;
	if (!SaveContext->Put<float>(AudioLevel_DB.Get())) return false; // USe the DB one because the user can see it and we do not want to lose in translation
	if (!SaveContext->Put<int>(Terminate.Get())) return false;
	if (!SaveContext->Put<int>(VideoMode.Get())) return false;
	if (!SaveContext->Put<int>(AudioMode.Get())) return false;

	return true;

}
bool SingleInputConfigModel::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	if (!LoadContext)
		return false;

	float B, H, C, S, Al;
	int T, Vm, Am;
	if (!LoadContext->Get<float>(B)) return false;
	if (!LoadContext->Get<float>(H)) return false;
	if (!LoadContext->Get<float>(C)) return false;
	if (!LoadContext->Get<float>(S)) return false;
	if (!LoadContext->Get<float>(Al)) return false;
	if (!LoadContext->Get<int>(T)) return false;
	if (!LoadContext->Get<int>(Vm)) return false;
	if (!LoadContext->Get<int>(Am)) return false;

	m_waitToSend = eWaitingNoChanges;
	Brightness=B; 
	Hue=H; 
	Contrast=C; 
	Saturation=S; 
	AudioLevel_DB=Al;
	Terminate=T;
	VideoMode = Vm;
	AudioMode = Am;
	
	// ALWAYS send the changes when loading, that way we make sure the hardware is updated
	UpdateXML();
	m_waitToSend = eNotWaiting;

	return true;
}
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

OutputConfigModel::OutputConfigModel() : m_waitToSend(eNotWaiting), 
	VertPos(VERT_DEF), m_oldVertPos(VERT_DEF),
	HorizPos(HORIZ_DEF), m_oldHorizPos(HORIZ_DEF),
	Phase(PHASE_DEF), m_oldPhase(PHASE_DEF),
	VideoMode(VIDEO_TYPE_DEF), m_oldVideoInput(VIDEO_TYPE_DEF)
{
	VertPos.AddDependant(this);
	HorizPos.AddDependant(this);
	Phase.AddDependant(this);
	VideoMode.AddDependant(this);

	m_configLine = new char[512];
	m_configLineW = new wchar_t[512];
}
//////////////////////////////////////////////////////////////////////////

OutputConfigModel::~OutputConfigModel()
{
	delete[] m_configLine;
	delete[] m_configLineW;
}

void OutputConfigModel::UpdateXML()
{
	// TODO: we still need to send the values for horz, vert & phase

	/////// Then for the Outputs ///////////////
	// Here is an example of what this output might look like:
	// <Modules.HW.Output.Video.1 VideoConnection="Y/C" />

	for (int outputNum = 1; outputNum <= 3; ++outputNum)
	{
		//Note: m_inputNumber works with 0-2 range, but the XML works with 1-3
		::sprintf(m_configLine, "<Modules.HW.Output.Video.%d VideoConnection=\"%s\" />", 
			outputNum, VIDEO_INPUT_STRINGS[VideoMode.Get()]);

		::wsprintfW(m_configLineW, L"%hS", m_configLine);

		// And tell the application to send this line
		Application::UI::Base::send_xml(m_configLineW);
	}
}
//////////////////////////////////////////////////////////////////////////

// Verifies the values and sends them
void OutputConfigModel::DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing)
{
	CHECK_VALS(VertPos, VERT_MIN, VERT_MAX, m_oldVertPos);
	CHECK_VALS(HorizPos, HORIZ_MIN, HORIZ_MAX, m_oldHorizPos);
	CHECK_VALS(Phase, PHASE_MIN, PHASE_MAX, m_oldPhase);

	if ((p_item_changing == &VideoMode) && (VideoMode != m_oldVideoInput))
	{
		m_oldVideoInput = VideoMode;
		ValidateVideoMode();
		return;
	}

	// I did not handle it, so call the base class
	__super::DynamicCallback(id, p_string, p_args, p_item_changing);
}
//////////////////////////////////////////////////////////////////////////

// This is a function available for all inputs and outputs
unsigned* OutputConfigModel::GetVideoOptions()
{
	// Handle based on the Video Resolution being used in Replay
	NT2005::Config::eVidDisplay d = Application::Replay::UI::get_instance().get_video_mode_desc().m_eVidDisplay;
	if ((d == NT2005::Config::eVidDisplay_SD) || (d == NT2005::Config::eVidDisplay_WideSD))
		return (unsigned*)SD_VIDEO_OUTPUT_OPTS;
	else
		return (unsigned*)HD_VIDEO_OUTPUT_OPTS;
}
//////////////////////////////////////////////////////////////////////////

// BaseLoadSave
void OutputConfigModel::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{
	MajorVersion = 1;
	MinorVersion = 0;
}
//////////////////////////////////////////////////////////////////////////

bool OutputConfigModel::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	if (!SaveContext)
		return false;

	if (!SaveContext->Put<float>(VertPos.Get())) return false;
	if (!SaveContext->Put<float>(HorizPos.Get())) return false;
	if (!SaveContext->Put<float>(Phase.Get())) return false;
	if (!SaveContext->Put<int>(VideoMode.Get())) return false;

	// All was well
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool OutputConfigModel::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	if (!LoadContext)
		return false;

	float V, H, P; int O;
	if (!LoadContext->Get<float>(V)) return false;
	if (!LoadContext->Get<float>(H)) return false;
	if (!LoadContext->Get<float>(P)) return false;
	if (!LoadContext->Get<int>(O)) return false;

	m_waitToSend = eWaitingNoChanges;
	VertPos=V; 
	HorizPos=H; 
	Phase=P;
	VideoMode = O;
	// ALWAYS send the changes when loading, that way we make sure the hardware is updated
	UpdateXML();
	m_waitToSend = eNotWaiting;

	// All was well
	return true;
}
//////////////////////////////////////////////////////////////////////////

void OutputConfigModel::ValidateVideoMode()
{
	unsigned* videoInputOpts = GetVideoOptions();
	unsigned i = 0;
	unsigned curr = (unsigned)VideoMode.Get();
	while (videoInputOpts[i] != (unsigned)-1)
	{
		if (curr == videoInputOpts[i])
		{	
			// Send it along if we are not waiting
			if (m_waitToSend == eNotWaiting) 
				UpdateXML();
			else m_waitToSend = eWaitingChanges;

			// All done
			return;
		}
		++i;
	}
	// We have an invalid input, set it back to def. This should call the DynamicCallback again I think
	VideoMode = VIDEO_TYPE_DEF;
}
//////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

InputConfigModel::InputConfigModel()
{
	// Set the global limits of the Audio Level DB based on the UI limits
	AUDIO_DEF_DB = DBConverter.ConvertUITodBValue(AUDIO_DEF_UI);
	AUDIO_MIN_DB = DBConverter.ConvertUITodBValue(AUDIO_MIN_UI);
	AUDIO_MAX_DB = DBConverter.ConvertUITodBValue(AUDIO_MAX_UI);

	for (unsigned i = 0; i < NUM_INPUTS; ++i)
		m_inputs[i] = new SingleInputConfigModel(i);

	// Send it to the Hardware for the first time
	UpdateXML();
}
//////////////////////////////////////////////////////////////////////////

InputConfigModel::~InputConfigModel()
{
	for (unsigned i = 0; i < NUM_INPUTS; ++i)
		delete m_inputs[i];
}
//////////////////////////////////////////////////////////////////////////

// BaseLoadSave
void InputConfigModel::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{
	MajorVersion = 1;
	MinorVersion = NUM_INPUTS;
}
//////////////////////////////////////////////////////////////////////////

bool InputConfigModel::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	if (!SaveContext)
		return false;

	for (unsigned i = 0; i < NUM_INPUTS; ++i)
	{
		BaseLoadSave_SaveData_Member(m_inputs[i], SaveContext);
	}
	BaseLoadSave_SaveData_Member(&Output, SaveContext);

	// All was well
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool InputConfigModel::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	if (!LoadContext)
		return false;

	// Make sure we have the correct number of inputs
	if (MinorVersion != NUM_INPUTS)
		return false;

	for (unsigned i = 0; i < NUM_INPUTS; ++i)
	{
		BaseLoadSave_LoadData_Member(m_inputs[i], LoadContext);
	}
	BaseLoadSave_LoadData_Member(&Output, LoadContext);

	// All was well
	return true;
}
//////////////////////////////////////////////////////////////////////////

void InputConfigModel::UpdateXML()
{
	for (unsigned i = 0; i < NUM_INPUTS; ++i)
	{
		m_inputs[i]->UpdateXML();
	}
	Output.UpdateXML();
}
//////////////////////////////////////////////////////////////////////////

// I am not sure who should own this, for now lets make it a single static Instance
InputConfigModel InputConfigModel::MainModel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CREATE_CONTROL(mask, control) \
	GetWindowInterface<control>(OpenChild((mask), (#control)))

#define CREATE_SUBCONTROL(mask, control) \
	GetInterface<control>(OpenChild_SubControl((mask), (#control)))

////////////////////////////////////////////////////////////////////////////////////////////////////////////

InputConfiguration::InputConfiguration(const char* p_skin_root, bool bls)
	: SkinControlBase(p_skin_root, bls)
{
	// Set up some basics for how the TextItem works.  This item is scriptable so may be modified there
		m_scriptingTextItem.Set_ForUI();
		WindowPixel wp;
		wp.SetRGB_Clipped(0,0,0);
		m_scriptingTextItem.SetColor(wp);
}

InputConfiguration::~InputConfiguration(void)
{
}

bool InputConfiguration::ExecuteCommand
	(char *FunctionName, ScriptExecutionStack &InputParameters, ScriptExecutionStack &OutputParameters, char *&Error)
{
	if (!strcmp(FunctionName,"GetDisplayTextItem"))
	{
		// Place my text item on the stack so the script can work with it
		ScriptExecutionStack *NewStack = ScriptExecutionStack::GetNewStack(&OutputParameters);
		NewStack->SetContext(&m_scriptingTextItem);
		OutputParameters.Add() = NewStack;
		NewStack->Release();
		return true;
	}
	return __super::ExecuteCommand(FunctionName, InputParameters, OutputParameters, Error);
}


void InputConfiguration::InitializeSubControls(void)
{
	SkinControlBase::InitializeSubControls();

	char degStr[2];
	sprintf(degStr, "%c", 176);	// The degree sign

	const unsigned brightness_spinner_masks[InputConfigModel::NUM_INPUTS] = { RGBA(222,150,150), RGBA(222,150,180), RGBA(222,150,210) };
	const unsigned brightness_text_masks[InputConfigModel::NUM_INPUTS]    = { RGBA(222,150,0),   RGBA(222,160,0),   RGBA(222,170,0)   };
	const unsigned contrast_spinner_masks[InputConfigModel::NUM_INPUTS]   = { RGBA(222,150,155), RGBA(222,150,185), RGBA(222,150,215) };
	const unsigned contrast_text_masks[InputConfigModel::NUM_INPUTS]      = { RGBA(222,150,10),  RGBA(222,160,10),  RGBA(222,170,10)  };
	const unsigned hue_spinner_masks[InputConfigModel::NUM_INPUTS]        = { RGBA(222,150,160), RGBA(222,150,190), RGBA(222,150,220) };
	const unsigned hue_text_masks[InputConfigModel::NUM_INPUTS]           = { RGBA(222,150,20),  RGBA(222,160,20),  RGBA(222,170,20)  };
	const unsigned saturation_spinner_masks[InputConfigModel::NUM_INPUTS] = { RGBA(222,150,165), RGBA(222,150,195), RGBA(222,150,225) };
	const unsigned saturation_text_masks[InputConfigModel::NUM_INPUTS]    = { RGBA(222,150,30),  RGBA(222,160,30),  RGBA(222,170,30)  };
	const unsigned restore_defaults_masks[InputConfigModel::NUM_INPUTS]   = { RGBA(222,50,50),   RGBA(222,50,60),   RGBA(222,50,70)   };
	const unsigned video_input_mask[InputConfigModel::NUM_INPUTS]         = { RGBA(222,100,0),   RGBA(222,100,10),  RGBA(222,100,20)  };
	const unsigned audio_input_mask[InputConfigModel::NUM_INPUTS]         = { RGBA(222,110,0),   RGBA(222,110,10),  RGBA(222,110,20)  };
	const unsigned terminate_mask[InputConfigModel::NUM_INPUTS]           = { RGBA(20,130,255),  RGBA(30,130,255),  RGBA(40,130,255)  };
	const unsigned audio_knob_mask[InputConfigModel::NUM_INPUTS]          = { RGBA(30,141,100),  RGBA(30,151,100),  RGBA(30,161,100)  };
	const unsigned audio_text_mask[InputConfigModel::NUM_INPUTS]          = { RGBA(30,141,136),  RGBA(30,151,136),  RGBA(30,161,136)  };
	
	for (unsigned i = 0; i < InputConfigModel::NUM_INPUTS; ++i)
	{
		SingleInputConfigModel* input = InputConfigModel::MainModel.GetInput(i);
		create_text_spinner(input->Brightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX, brightness_spinner_masks[i], brightness_text_masks[i],	1.0f, "%", &m_controlsToDisable[i].BWC_List);
		m_controlsToDisable[i].ColorMasks.Add(brightness_spinner_masks[i]); m_controlsToDisable[i].ColorMasks.Add(brightness_text_masks[i]); 
		create_text_spinner(input->Contrast,   CONTRAST_MIN,   CONTRAST_MAX,   contrast_spinner_masks[i],   contrast_text_masks[i],		1.0f, "%", &m_controlsToDisable[i].BWC_List);
		m_controlsToDisable[i].ColorMasks.Add(contrast_spinner_masks[i]); m_controlsToDisable[i].ColorMasks.Add(contrast_text_masks[i]); 
		create_text_spinner(input->Hue,        HUE_MIN,        HUE_MAX,        hue_spinner_masks[i],        hue_text_masks[i],			1.0f, degStr, &m_controlsToDisable[i].BWC_List);
		m_controlsToDisable[i].ColorMasks.Add(hue_spinner_masks[i]); m_controlsToDisable[i].ColorMasks.Add(hue_text_masks[i]); 
		create_text_spinner(input->Saturation, SATURATION_MIN, SATURATION_MAX, saturation_spinner_masks[i], saturation_text_masks[i],	1.0f, "%", &m_controlsToDisable[i].BWC_List);
		m_controlsToDisable[i].ColorMasks.Add(saturation_spinner_masks[i]); m_controlsToDisable[i].ColorMasks.Add(saturation_text_masks[i]); 

		create_togglebutton(input->Terminate, terminate_mask[i]);
		create_togglebutton(m_restore_defaults[i], restore_defaults_masks[i]);
		m_restore_defaults[i].AddDependant(this);

		create_combo(video_input_mask[i], &(input->VideoMode), SingleInputConfigModel::GetVideoOptions(), VIDEO_INPUT_STRINGS);

		// Keep track of the audio combo boxes so we can change them when the video changes
		m_audioComboBoxes[i] = create_combo(audio_input_mask[i], &(input->AudioMode), input->GetAudioOptions(), AUDIO_INPUT_STRINGS);

		// Listen for those video changes
		input->VideoMode.AddDependant(this);

		// Make the audio knobs
		create_audio_text_knob(input->AudioLevel_UI, input->AudioLevel_DB, audio_knob_mask[i], audio_text_mask[i], &m_controlsToDisable[i].BWC_List, &m_controlsToDisable[i].SSC_List);
		m_controlsToDisable[i].ColorMasks.Add(audio_knob_mask[i]); m_controlsToDisable[i].ColorMasks.Add(audio_text_mask[i]); 

		// Make sure to enable or disable based on the video mode
		m_controlsToDisable[i].Parent = this;
		if (input->VideoMode == InputConfigModel::VI_SDI)
			m_controlsToDisable[i].DisableControls();
		else
			m_controlsToDisable[i].EnableControls();
	}

	create_text_spinner(InputConfigModel::MainModel.Output.VertPos,  VERT_MIN,  VERT_MAX,  RGBA(222,150,240), RGBA(222,180,0),		0.01f, "");
	create_text_spinner(InputConfigModel::MainModel.Output.HorizPos, HORIZ_MIN, HORIZ_MAX, RGBA(222,150,245), RGBA(222,180,10),	0.01f, "");
	create_text_spinner(InputConfigModel::MainModel.Output.Phase,    PHASE_MIN, PHASE_MAX, RGBA(222,150,250), RGBA(222,180,20),	0.01f, "");
	create_combo(RGBA(222,120,0), &(InputConfigModel::MainModel.Output.VideoMode), OutputConfigModel::GetVideoOptions(), VIDEO_INPUT_STRINGS);
}

void InputConfiguration::DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing)
{
	for (unsigned i = 0; i < InputConfigModel::NUM_INPUTS; ++i)
	{
		SingleInputConfigModel* input = InputConfigModel::MainModel.GetInput(i);
		if (p_item_changing == &(input->VideoMode))
		{
			// Update the audio combo with the proper items
			 set_combo_items(m_audioComboBoxes[i], input->GetAudioOptions(), AUDIO_INPUT_STRINGS);

			 // Make sure to enable or disable based on the video mode
			 if (input->VideoMode == InputConfigModel::VI_SDI)
				 m_controlsToDisable[i].DisableControls();
			 else
				 m_controlsToDisable[i].EnableControls();

			// We have covered it
			return;
		}
		else if (p_item_changing == &(m_restore_defaults[i]))
		{
			if (m_restore_defaults[i].Get() != 0)
			{
				// Watch for restore defaults being clicked
				input->RestoreDefaults();
				m_restore_defaults[i].Set(0);
			}
			// We have covered it
			return;
		}
	}
	SkinControlBase::DynamicCallback(id, p_string, p_args, p_item_changing);
}

void InputConfiguration::create_audio_text_knob(
	DynamicFloat& dyn_UI, DynamicFloat& dyn_DB, const unsigned knob_mask, const unsigned text_mask,
	tList<BaseWindowClass*>* BWC_List, tList<SkinControl_SubControl*>* SSC_List)
{
	// Volume knobs
	const int starting_knob_layer = 5;
	const int num_knob_layers = 128;
	SkinControl_SubControl_TwistKnob_Float* p_dial = ::GetInterface<SkinControl_SubControl_TwistKnob_Float>(this->OpenChild_SubControl(knob_mask, "SkinControl_SubControl_TwistKnob_Float"));
	if (p_dial)
	{
		for (int j = 0; j < num_knob_layers; ++j)
			p_dial->SetLayerMapping(j, starting_knob_layer + j);

		p_dial->UpdateSetLayerMappings();
		p_dial->TwistKnob_SetDirection(SkinControl_SubControl_TwistKnob_Direction_LeftRight);
		p_dial->SkinControl_SubControl_TwistKnob_GetMin()->Set(AUDIO_MIN_UI);
		p_dial->SkinControl_SubControl_TwistKnob_GetMax()->Set(AUDIO_MAX_UI);
		p_dial->SkinControl_SubControl_TwistKnob_SetDefault(AUDIO_DEF_UI);
		p_dial->SkinControl_SubControl_TwistKnob_SetShiftDefault(AUDIO_DEF_UI);
		p_dial->SkinControl_SubControl_TwistKnob_SetValue(&dyn_UI);
	}

	// We may need to keep track of the control we just made in order to disable it later
	if (SSC_List)
		SSC_List->Add(p_dial);

	UtilLib_Edit* p_text = CREATE_CONTROL(text_mask, UtilLib_Edit);
	if (p_text)
	{
		*(p_text->Controls_Edit_GetItem()) = m_scriptingTextItem;
		p_text->UtilLibEdit_SetNoDecimalPlaces(2);
		p_text->Slider_GetMinVariable()->Set(AUDIO_MIN_DB);
		p_text->Slider_GetMaxVariable()->Set(AUDIO_MAX_DB);
		p_text->Slider_SetVariable(&dyn_DB);
	}

	// We may need to keep track of the control we just made in order to disable it later
	if (BWC_List)
		BWC_List->Add(p_text);
}

void InputConfiguration::create_text_spinner(
	DynamicFloat& dyn, const float dyn_min, const float dyn_max, const unsigned spinner_mask, 
	const unsigned text_mask, float width, const char* suffix, tList<BaseWindowClass*>* BWC_List)
{
	UtilLib_Spinner* p_spinner = CREATE_CONTROL(spinner_mask, UtilLib_Spinner);
	if (p_spinner)
	{
		p_spinner->SetAttr(UtilLib_Spinner_LeftRight);
		p_spinner->Spinner_GetMinVariable()->Set(dyn_min);
		p_spinner->Spinner_GetMaxVariable()->Set(dyn_max);
		p_spinner->Spinner_SetWidth(width);
		p_spinner->Spinner_UseVariable(&dyn);
	}

	// We may need to keep track of the control we just made in order to disable it later
	if (BWC_List)
		BWC_List->Add(p_spinner);

	UtilLib_Edit* p_text = CREATE_CONTROL(text_mask, UtilLib_Edit);
	if (p_text)
	{
		*(p_text->Controls_Edit_GetItem()) = m_scriptingTextItem;
		unsigned numDecPl = 0;
		if (width < 1.0f)
			numDecPl = 1;
		if (width < 0.1f)
			numDecPl = 2;
		p_text->UtilLibEdit_SetNoDecimalPlaces(numDecPl);
		p_text->Slider_GetMinVariable()->Set(dyn_min);
		p_text->Slider_GetMaxVariable()->Set(dyn_max);
		p_text->SetSuffix(suffix);
		p_text->Slider_SetVariable(&dyn);
	}

	// We may need to keep track of the control we just made in order to disable it later
	if (BWC_List)
		BWC_List->Add(p_text);
}

void InputConfiguration::create_togglebutton(DynamicInt& dyn, const unsigned mask)
{
	SkinControl_SubControl_ToggleButton* p_button = CREATE_SUBCONTROL(mask, SkinControl_SubControl_ToggleButton);
	if (p_button)
	{
		p_button->Button_UseVariable(&dyn);
		p_button->Button_SetSelectedState(1);
	}
}

void InputConfiguration::set_combo_items(UtilLib_Combo* p_combo, unsigned* p_itemIDX, const char** p_itemSTR)
{
	// We will need to restore the original item
	Dynamic<int>* buttonVar = p_combo->Button_GetVariable();
	buttonVar->MessagesOff();
	int origVal = buttonVar->Get();

	p_combo->DeleteAllItems();
	unsigned i = 0;
	while (p_itemIDX[i] != (unsigned)-1)
	{
		p_combo->AddItem(p_itemSTR[p_itemIDX[i]], p_itemIDX[i]);
		++i;
	}

	// Set the item back, should fire a changed message so we can validate the audio one
	buttonVar->Set(origVal);
	buttonVar->MessagesOn();
	buttonVar->Changed();
}

UtilLib_Combo* InputConfiguration::create_combo(const unsigned mask, Dynamic<int>* var, unsigned* p_itemIDX, const char** p_itemSTR)
{
	UtilLib_Combo* p_combo = CREATE_CONTROL(mask, UtilLib_Combo);

	// Make sure the text items are copied over
	for (unsigned i = 0; i < 4; ++i)
		*(p_combo->ButtonLabel_GetTextItem(i)) = m_scriptingTextItem;

	if (p_combo)
	{
		set_combo_items(p_combo, p_itemIDX, p_itemSTR);
		p_combo->Button_UseVariable(var);
	}
	return p_combo;
}

void InputConfiguration::DisabledControlSet::DisableControls()
{
	if (!m_enabled) return;
	size_t i, num;

	// Make all of the BWC classes not visible
	num = BWC_List.NoItems();
	for (i = 0; i < num; ++i)
	{
		// Not only will we disable these, but make them completely not visible
		BWC_List[i]->ShowWindow(false);
	}

	// Disable all of the SubControls and set their layer to 4 (the disabled layer)
	num = SSC_List.NoItems();
	for (i = 0; i < num; ++i)
	{
		// Disable from having input, the layer will be set later
		SSC_List[i]->SetEnabled(false);
	}

	// Set the layer for all of the mask colors we have defined to the disabled state
	num = ColorMasks.NoItems();
	for (i = 0; i < num; ++i)
	{
		// Remember the layer we were ON so that we can reset it
		unsigned oldLayerNum = Parent->GetStretchySkin()->GetLayerNumber(ColorMasks[i]);
		if (SSC_Layers.NoItems() <= i)
			SSC_Layers.Add(oldLayerNum);
		else 
			SSC_Layers[i] = oldLayerNum;

		// Set all of the layers to the disabled state
		Parent->GetStretchySkin()->ChangeLayer(ColorMasks[i], 4);

		// We have to invalidate the RECT manually (too bad ChangeLayer does not do it)
		RECT rect; Parent->GetStretchySkin()->GetRect(ColorMasks[i], rect);
		rect.right	+= 1;
		rect.bottom += 1;
		Parent->InvalidateRect(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
	}

	// Remember enabled state
	m_enabled = false;
}
//////////////////////////////////////////////////////////////////////////

void InputConfiguration::DisabledControlSet::EnableControls()
{
	if (m_enabled) return;
	size_t i, num;

	// Re-enable all of the SubControls and redraw to make sure they are showing their proper layer
	num = SSC_List.NoItems();
	for (i = 0; i < num; ++i)
	{
		// Enable it again for user control
		SSC_List[i]->SetEnabled(true);
	}

	// Make all of the BWC classes visible again
	num = BWC_List.NoItems();
	for (i = 0; i < num; ++i)
		BWC_List[i]->ShowWindow(true);

	// Set the layer for all of the mask colors we have defined back to the original state
	num = ColorMasks.NoItems();
	for (i = 0; i < num; ++i)
	{
		Parent->GetStretchySkin()->ChangeLayer(ColorMasks[i], SSC_Layers[i]);

		// We have to invalidate the RECT manually (too bad ChangeLayer does not do it)
		RECT rect; Parent->GetStretchySkin()->GetRect(ColorMasks[i], rect);
		rect.right	+= 1;
		rect.bottom += 1;
		Parent->InvalidateRect(rect.left,rect.top,rect.right-rect.left,rect.bottom-rect.top);
	}

	// Remember enabled state
	m_enabled = true;
}
//////////////////////////////////////////////////////////////////////////

NewTek_RegisterPluginControl(InputConfiguration);
