#pragma once

//This parses out the LUA into two table for each control element... its population properties and LUT
class COMMON_API Control_Assignment_Properties
{
	public:
		virtual ~Control_Assignment_Properties() {}
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
		const Controls_1C &GetRelays() const {return m_Relays;}
		const Controls_1C &GetDigitalInputs() const {return m_Digital_Inputs;}
		const Controls_2C &GetDoubleSolenoids() const {return m_Double_Solenoids;}
		const Controls_2C &GetEncoders() const {return m_Encoders;}
		size_t GetCompressorRelay() {return m_Compressor_Relay;}
		size_t GetCompressorLimit() {return m_Compressor_Limit;}
	private:
		Controls_1C m_Victors,m_Relays,m_Digital_Inputs;
		Controls_2C m_Double_Solenoids,m_Encoders;
		size_t m_Compressor_Relay,m_Compressor_Limit;
};


//Add simulated WPILib control elements here... these are to reflect the latest compatible interface with the WPI libraries
#ifdef Robot_TesterCode
typedef unsigned     int uint32_t;
typedef unsigned    char uint8_t;
typedef unsigned	 int UINT32;
typedef              int int32_t;

class Control_1C_Element_UI
{
public:
	Control_1C_Element_UI(uint8_t moduleNumber, uint32_t channel,const char *name);
	void display_number(double value);
	void display_bool(bool value);
	bool get_bool() const;
	double get_number() const;
protected:
	std::string m_Name;
};

class Control_2C_Element_UI
{
public:
	Control_2C_Element_UI(uint8_t moduleNumber, uint32_t forward_channel, uint32_t reverse_channel,const char *name);
	void display_bool(bool value);
	void display_number(double value);
	bool get_bool() const;
protected:
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

class Encoder2 : public Control_2C_Element_UI
{
public:
	//Note: Encoder allows two module numbers... we'll skip that support since double solenoid doesn't have it, and we can reuse 2C
	//for both... we can change if needed
	Encoder2(uint8_t ModuleNumber,UINT32 aChannel, UINT32 bChannel,const char *name);
	void TimeChange(double dTime_s,double current_voltage); //only used in simulation
	double GetRate2(double dTime_s);
	void Reset2();

	void Start();
	int32_t Get();
	int32_t GetRaw();
	void Reset();
	void Stop();
	double GetDistance();
	double GetRate();
	void SetMinRate(double minRate);
	void SetDistancePerPulse(double distancePerPulse);
	void SetReverseDirection(bool reverseDirection);
private:
	double m_LastDistance;  //keep note of last distance
	double m_Distance;
	double m_LastTime;
};

class Relay : public Control_1C_Element_UI
{
public:
	typedef enum {kOff, kOn, kForward, kReverse} Value;
	typedef enum {kBothDirections, kForwardOnly, kReverseOnly} Direction;

	Relay(uint8_t moduleNumber, uint32_t channel,const char *name) :  Control_1C_Element_UI(moduleNumber,channel,name),
		m_ModuleNumber(moduleNumber), m_Channel(channel) {}
	//virtual ~Relay();

	void Set(Value value) {m_CurrentValue=value; display_bool(value==kForward);}
	Value Get() {return m_CurrentValue=get_bool()?kForward:kReverse;}

private:
	uint8_t m_ModuleNumber;
	uint32_t m_Channel;
	Value m_CurrentValue;
};

class COMMON_API Compressor : public Control_2C_Element_UI
{
public:
	Compressor(uint32_t pressureSwitchChannel, uint32_t compressorRelayChannel) : 
	  Control_2C_Element_UI(1,pressureSwitchChannel,compressorRelayChannel,"compressor"), m_enabled(false)
	  {
		  display_bool(false);
	  }

	void Start() {display_bool(true); m_enabled=true;}
	void Stop() {display_bool(false); m_enabled=false;}
	bool Enabled() {return m_enabled;}
	//uint32_t GetPressureSwitchValue();
	//void SetRelayValue(Relay::Value relayValue);

private:
	bool m_enabled;
};

#endif

class COMMON_API RobotControlCommon
{
	public:
		typedef std::vector<size_t> Controls_LUT;
		virtual ~RobotControlCommon();

		//victor methods
		__inline double Victor_GetCurrentPorV(size_t index) {return m_Victors[m_VictorLUT[index]]->Get();}
		__inline void Victor_UpdateVoltage(size_t index,double Voltage) {m_Victors[m_VictorLUT[index]]->Set(Voltage);}

		//solenoid methods
		__inline void Solenoid_Open(size_t index,bool Open=true) 
		{	DoubleSolenoid::Value value=Open ? DoubleSolenoid::kForward : DoubleSolenoid::kReverse;
			m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Set(value);
		}
		__inline void Solenoid_Close(size_t index,bool Close=true) {Solenoid_Open(index,!Close);}
		__inline bool Solenoid_GetIsOpen(size_t index) const 
		{	return m_DoubleSolenoids[m_DoubleSolenoidLUT[index]]->Get()==DoubleSolenoid::kForward;
		}
		__inline bool Solenoid_GetIsClosed(size_t index) const {return !Solenoid_GetIsOpen(index);}

		//digital input method
		__inline bool BoolSensor_GetState(size_t index) {return m_DigitalInputs[m_DigitalInputLUT[index]]->Get()!=0;}

		__inline double Encoder_GetRate(size_t index) {return m_Encoders[m_EncoderLUT[index]]->GetRate();}
		__inline double Encoder_GetDistance(size_t index) {return m_Encoders[m_EncoderLUT[index]]->GetDistance();}
		__inline void Encoder_Start(size_t index) { m_Encoders[m_EncoderLUT[index]]->Start();}
		__inline void Encoder_Stop(size_t index) { m_Encoders[m_EncoderLUT[index]]->Stop();}
		__inline void Encoder_Reset(size_t index) 
		{	if ((index<m_EncoderLUT.size()) && (m_EncoderLUT[index]!=(size_t)-1)) 	m_Encoders[m_EncoderLUT[index]]->Reset();
		}
		__inline void Encoder_SetDistancePerPulse(size_t index,double distancePerPulse) {m_Encoders[m_EncoderLUT[index]]->SetDistancePerPulse(distancePerPulse);}
		__inline void Encoder_SetReverseDirection(size_t index,bool reverseDirection) {m_Encoders[m_EncoderLUT[index]]->SetReverseDirection(reverseDirection);}
		#ifdef Robot_TesterCode
		__inline void Encoder_TimeChange(size_t index,double adjustment_delta) {m_Encoders[m_EncoderLUT[index]]->TimeChange(index,adjustment_delta);}
		#endif
		void TranslateToRelay(size_t index,double Voltage);
		__inline Compressor *CreateCompressor()
		{	return new Compressor(m_Props.GetCompressorLimit(),m_Props.GetCompressorRelay());
		}
		__inline void DestroyCompressor(Compressor *instance) {delete instance;}
	protected:
		virtual void RobotControlCommon_Initialize(const Control_Assignment_Properties &props);
		//Override by derived class
		virtual size_t RobotControlCommon_Get_Victor_EnumValue(const char *name) const =0;
		virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const =0;
		virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const =0;
	private:
		Control_Assignment_Properties m_Props;  //cache a copy of the assignment props
		std::vector<Victor *> m_Victors;
		std::vector<Relay *> m_Relays;
		std::vector<DigitalInput *> m_DigitalInputs;
		std::vector<DoubleSolenoid *> m_DoubleSolenoids;
		std::vector<Encoder2 *> m_Encoders;

		Controls_LUT m_VictorLUT,m_RelayLUT,m_DigitalInputLUT,m_DoubleSolenoidLUT,m_EncoderLUT;
};