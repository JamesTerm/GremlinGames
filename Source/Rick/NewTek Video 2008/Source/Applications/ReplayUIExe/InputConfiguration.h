#pragma once

enum WaitingForChanges {eNotWaiting, eWaitingNoChanges, eWaitingChanges};
class SingleInputConfigModel : BaseLoadSave, DynamicListener
{
private:
	// Used to send the appropriate XML
	unsigned m_inputNumber;

	WaitingForChanges m_waitToSend;	// Used in cases where lots of things may be changing so changes are sent one time
									// We do not want to hold on to changes with ReceiveMessagesOff() because I want to allow the
									// DynamicCallback to validate the results

	char* m_configLine;
	wchar_t* m_configLineW;

	float m_oldBrightness, m_oldHue, m_oldContrast, m_oldSaturation, m_oldAudioLevel_UI, m_oldAudioLevel_DB;
	int m_oldTerminate, m_oldVideoInput, m_oldAudioInput;

	void ValidateVideoMode();
	void ValidateAudioMode();

public:
	SingleInputConfigModel(unsigned inputNum);
	~SingleInputConfigModel();
	unsigned GetInputNumber() const {return m_inputNumber;}

	// Be able to get the list of options for audio based on the video selection
	unsigned* GetAudioOptions();

	// Call to set all of the 
	void RestoreDefaults();

	// Sends the data to the HW
	void UpdateXML();

	// Verifies the values and sends them
	virtual void DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing);

	// Access the values directly, they are clamped to min and max values in my DynamicCallback before sent
	DynamicFloat	Brightness;
	DynamicFloat	Hue;
	DynamicFloat	Contrast;
	DynamicFloat	Saturation;
	DynamicInt		Terminate;		// Only 0 and 1 are used
	DynamicInt		VideoMode;
	DynamicInt		AudioMode;

	// These two are tied together, changing one will change the other
	DynamicFloat	AudioLevel_UI;		// In UI units
	DynamicFloat	AudioLevel_DB;		// In DB units

	// BaseLoadSave
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	static unsigned* GetVideoOptions();
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class OutputConfigModel : BaseLoadSave, DynamicListener
{
private:
	WaitingForChanges m_waitToSend;	// Used in cases where lots of things may be changing so changes are sent one time
									// We do not want to hold on to changes with ReceiveMessagesOff() because I want to allow the
									// DynamicCallback to validate the results

	float m_oldVertPos, m_oldHorizPos, m_oldPhase;
	int m_oldVideoInput;

	char* m_configLine;
	wchar_t* m_configLineW;

	void ValidateVideoMode();

public:
	OutputConfigModel();
	~OutputConfigModel();

	// Verifies the values and sends them
	virtual void DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing);

	// Access the values directly, they are clamped to min and max values in my DynamicCallback before sent
	DynamicFloat	VertPos;
	DynamicFloat	HorizPos;
	DynamicFloat	Phase;
	DynamicInt		VideoMode;

	// BaseLoadSave
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	// Sends the data to the HW
	void UpdateXML();

	static unsigned* GetVideoOptions();
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InputConfigModel : public BaseLoadSave
{
public:
	// We have 3 inputs
	static const unsigned NUM_INPUTS = 3;

	InputConfigModel();
	~InputConfigModel();

	// The Data we are keeping track of
	OutputConfigModel Output;
	SingleInputConfigModel* GetInput(unsigned idx){assert(idx<=NUM_INPUTS); return m_inputs[idx];}

	// BaseLoadSave
	virtual void BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion);
	virtual bool BaseLoadSave_SaveData(SaveData *SaveContext);
	virtual bool BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion);

	// I am not sure who should own this, for now lets make it a single static Instance
	static InputConfigModel MainModel;

	// Utility function Just calls its members
	void UpdateXML();

	enum										{ VI_COMPOSITE, VI_YC, VI_COMPONENT,	VI_SDI};
	enum										{ AUDIO_ANALOG,   AUDIO_DIGITAL,   AUDIO_SDI            };

private:
	SingleInputConfigModel* m_inputs[NUM_INPUTS];
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////

class InputConfiguration
	: public SkinControlBase
{
	public:
		InputConfiguration(const char* p_skin_root = "InputConfig", bool bls = false);
		virtual ~InputConfiguration(void);

		// AutoSkinControl
		virtual void InitializeSubControls(void);

		// DynamicListener
		virtual void DynamicCallback(LONG_PTR id, char* p_string, void* p_args, DynamicTalker* p_item_changing);

		// Scripting
		virtual bool ExecuteCommand(char *FunctionName, ScriptExecutionStack &InputParameters, ScriptExecutionStack &OutputParameters, char *&Error);

		// We need to provide some public access to our skin
		ScreenObjectSkin* GetStretchySkin(){return m_autoSkin;}

	private:
		void create_audio_text_knob(DynamicFloat& dyn_UI, DynamicFloat& dyn_DB, const unsigned knob_mask, const unsigned text_mask, tList<BaseWindowClass*>* BWC_List=NULL, tList<SkinControl_SubControl*>* SSC_List=NULL);
		void create_text_spinner(DynamicFloat& dyn, const float dyn_min, const float dyn_max, const unsigned spinner_mask, const unsigned text_mask, float width, const char* suffix, tList<BaseWindowClass*>* BWC_List=NULL);
		void create_togglebutton(DynamicInt& dyn, const unsigned mask);
		UtilLib_Combo* create_combo(const unsigned mask, Dynamic<int>* var, unsigned* p_itemIDX, const char** p_itemSTR);
		void set_combo_items(UtilLib_Combo* p_combo, unsigned* p_itemIDX, const char** p_itemSTR);

		DynamicInt m_restore_defaults[InputConfigModel::NUM_INPUTS];
		UtilLib_Combo* m_audioComboBoxes[InputConfigModel::NUM_INPUTS];

		// The text item that is used in scripting to accept all of the parameters
		TextItem m_scriptingTextItem;

		class DisabledControlSet
		{
		public:
			DisabledControlSet() {Parent = NULL; m_enabled=true;}
			InputConfiguration*				Parent;
			tList<BaseWindowClass*>			BWC_List;
			tList<SkinControl_SubControl*>	SSC_List;
			tList<unsigned>					ColorMasks;

			void DisableControls();
			void EnableControls();

		private:
			bool m_enabled;
			tList<unsigned>					SSC_Layers;
		};

		DisabledControlSet m_controlsToDisable[InputConfigModel::NUM_INPUTS];
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////
