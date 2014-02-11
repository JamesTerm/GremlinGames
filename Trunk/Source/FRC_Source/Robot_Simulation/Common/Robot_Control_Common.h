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

class Victor
{
public:
	explicit Victor(uint32_t channel) : m_ModuleNumber(1),m_Channel(channel) {}
	Victor(uint8_t moduleNumber, uint32_t channel) : m_ModuleNumber(moduleNumber), m_Channel(channel) {}
	virtual void Set(float value, uint8_t syncGroup=0) {m_CurrentVoltage=value;}
	virtual float Get() {return m_CurrentVoltage;}
	virtual void Disable() {}
	//virtual void PIDWrite(float output);
private:
	uint8_t m_ModuleNumber;
	uint32_t m_Channel;
	float m_CurrentVoltage;
};

class DigitalInput
{
public:
	explicit DigitalInput(uint32_t channel) : m_ModuleNumber(1),m_Channel(channel) {}
	DigitalInput(uint8_t moduleNumber, uint32_t channel) : m_ModuleNumber(moduleNumber), m_Channel(channel) {}
	uint32_t Get() {return 0;}
	uint32_t GetChannel() {return m_Channel;}
private:
	uint8_t m_ModuleNumber;
	uint32_t m_Channel;
};

class DoubleSolenoid
{
public:
	typedef enum {kOff, kForward, kReverse} Value;
	explicit DoubleSolenoid(uint32_t forwardChannel, uint32_t reverseChannel) : m_ModuleNumber(1),
		m_forwardChannel(forwardChannel),m_reverseChannel(reverseChannel) {}
	DoubleSolenoid(uint8_t moduleNumber, uint32_t forwardChannel, uint32_t reverseChannel) : m_ModuleNumber(moduleNumber),
		m_forwardChannel(forwardChannel),m_reverseChannel(reverseChannel) {}
	virtual void Set(Value value) {m_CurrentValue=value;}
	virtual Value Get() {return m_CurrentValue;}
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
		void OpenSolenoid(size_t index,bool Open=true) 
		{	DoubleSolenoid::Value value=Open ? DoubleSolenoid::kForward : DoubleSolenoid::kReverse;
			m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Set(value);
		}
		void CloseSolenoid(size_t index,bool Close=true) {OpenSolenoid(index,!Close);}
		bool GetIsSolenoidOpen(size_t index) const 
		{	return m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Get()==DoubleSolenoid::kForward;
		}
		bool GetIsSolenoidClosed(size_t index) const {return !GetIsSolenoidOpen(index);}

		//digital input method
		virtual bool GetBoolSensorState(size_t index) {return m_DigitalInputs[m_DigitalInputLUT[index]]->Get()!=0;}

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