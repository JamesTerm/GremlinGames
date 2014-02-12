#pragma once

//This parses out the LUA into two table for each control element... its population properties and LUT
class COMMON_API Control_Assignment_Properties
{
	public:
		struct Control_Element_1C
		{
			std::string name;
			size_t Channel;
			size_t Module;
		};
		struct Control_Element_2C
		{
			std::string name;
			size_t ForwardChannel,ReverseChannel;
			size_t Module;
		};
		typedef std::vector<Control_Element_1C> Controls_1C;
		typedef std::vector<Control_Element_2C> Controls_2C;

		virtual void LoadFromScript(Scripting::Script& script);

		const Controls_1C &GetVictors() const {return m_Victors;}
		const Controls_1C &GetDigitalInputs() const {return m_Digital_Inputs;}
		const Controls_2C &GetDoubleSolenoids() const {return m_Double_Solenoids;}
	private:
		Controls_1C m_Victors,m_Digital_Inputs;
		Controls_2C m_Double_Solenoids;
};


//Add simulated WPILib control elements here... these are to reflect the latest compatible interface with the WPI libraries
#ifdef Robot_TesterCode
typedef unsigned     int uint32_t;
typedef unsigned    char uint8_t;

class Control_1C_Element_UI
{
	public:
		Control_1C_Element_UI(uint8_t moduleNumber, uint32_t channel,const char *name);
		void display_number(double value);
		void display_bool(bool value);
		bool get_bool() const;
		double get_number() const;
	private:
	std::string m_Name;
};

class Control_2C_Element_UI
{
public:
	Control_2C_Element_UI(uint8_t moduleNumber, uint32_t forward_channel, uint32_t reverse_channel,const char *name);
	void display_bool(bool value);
	bool get_bool() const;
private:
	std::string m_Name;
};

class Victor : public Control_1C_Element_UI
{
public:
	Victor(uint8_t moduleNumber, uint32_t channel,const char *name) : Control_1C_Element_UI(moduleNumber,channel,name),
	  m_ModuleNumber(moduleNumber), m_Channel(channel) {}
	virtual void Set(float value, uint8_t syncGroup=0) {m_CurrentVoltage=value; display_number(value);}
	virtual float Get() {return m_CurrentVoltage;}
	virtual void Disable() {}
	//virtual void PIDWrite(float output);
private:
	uint8_t m_ModuleNumber;
	uint32_t m_Channel;
	float m_CurrentVoltage;
};

class DigitalInput : public Control_1C_Element_UI
{
public:
	DigitalInput(uint8_t moduleNumber, uint32_t channel,const char *name) : Control_1C_Element_UI(moduleNumber,channel,name),
		m_ModuleNumber(moduleNumber), m_Channel(channel) {}
	uint32_t Get() {return get_number();}
	uint32_t GetChannel() {return m_Channel;}
private:
	uint8_t m_ModuleNumber;
	uint32_t m_Channel;
};

class DoubleSolenoid : public Control_2C_Element_UI
{
public:
	typedef enum {kOff, kForward, kReverse} Value;
	DoubleSolenoid(uint8_t moduleNumber, uint32_t forwardChannel, uint32_t reverseChannel,const char *name) : 
		Control_2C_Element_UI(moduleNumber,forwardChannel,reverseChannel,name),
		m_ModuleNumber(moduleNumber),m_forwardChannel(forwardChannel),m_reverseChannel(reverseChannel) {}
	virtual void Set(Value value) {m_CurrentValue=value; display_bool(value==kForward);}
	virtual Value Get() {return m_CurrentValue=get_bool()?kForward:kReverse;}
private:
	uint8_t m_ModuleNumber;
	uint32_t m_forwardChannel; ///< The forward channel on the module to control.
	uint32_t m_reverseChannel; ///< The reverse channel on the module to control.
	Value m_CurrentValue;
};

#endif


class COMMON_API RobotControlCommon
{
	public:
		virtual ~RobotControlCommon();

		//victor methods
		double Victor_GetCurrentPorV(size_t index) {return m_Victors[m_VictorLUT[index]]->Get();}
		void Victor_UpdateVoltage(size_t index,double Voltage) {m_Victors[m_VictorLUT[index]]->Set(Voltage);}

		//solenoid methods
		void Solenoid_Open(size_t index,bool Open=true) 
		{	DoubleSolenoid::Value value=Open ? DoubleSolenoid::kForward : DoubleSolenoid::kReverse;
			m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Set(value);
		}
		void Solenoid_Close(size_t index,bool Close=true) {Solenoid_Open(index,!Close);}
		bool Solenoid_GetIsOpen(size_t index) const 
		{	return m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Get()==DoubleSolenoid::kForward;
		}
		bool Solenoid_GetIsClosed(size_t index) const {return !Solenoid_GetIsOpen(index);}

		//digital input method
		virtual bool BoolSensor_GetState(size_t index) {return m_DigitalInputs[m_DigitalInputLUT[index]]->Get()!=0;}

	protected:
		virtual void RobotControlCommon_Initialize(const Control_Assignment_Properties &props);
		//Override by derived class
		virtual size_t RobotControlCommon_Get_Victor_EnumValue(const char *name) const =0;
		virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const =0;
		virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const =0;
	private:
		Control_Assignment_Properties m_Props;  //cache a copy of the assignment props
		std::vector<Victor *> m_Victors;
		std::vector<DigitalInput *> m_DigitalInputs;
		std::vector<DoubleSolenoid *> m_DoubleSolenoids;

		typedef std::vector<size_t> Controls_LUT;
		Controls_LUT m_VictorLUT,m_DigitalInputLUT,m_DoubleSolenoidLUT;
};