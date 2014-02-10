#pragma once

struct Rotary_Props
{
	double VoltageScalar;		//Used to handle reversed voltage wiring
	//Note: EncoderToRS_Ratio and PotentiometerOffset are place holder properties that are implemented in the robot control
	//interface as needed for that control... they are not used in the rotary system code
	//The gear reduction used when multiplied by the encoder RPS will equal the *Rotary System's* RPS
	//This is typically the motor speed since this solves to apply voltage to it
	double EncoderToRS_Ratio;
	//This allows adjustment of the potentiometer in software to avoid manual recalibration (used only for position control)
	double PotentiometerOffset;  
	double PID[3]; //p,i,d
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	//Currently supporting 4 terms in polynomial equation
	PolynomialEquation_forth_Props Voltage_Terms;  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel;  //used for deceleration case
	double Positive_DeadZone;
	double Negative_DeadZone;  //These must be in negative form

	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	enum LoopStates
	{
		eNone, //Will never read them (ideal for systems that do not have any encoders)
		eOpen,  //Will read them but never alter velocities
		eClosed, //Will attempt to match predicted velocity to actual velocity
		eClosed_ManualAssist //For position control this mode is also closed during manual assist
	} LoopState; //This should always be false once control is fully functional
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)

	//Only supported in Rotary_Velocity_Control
	bool UseAggressiveStop;  //If true, will use adverse force to assist in stopping.

	//Only supported in Rotary_Position_Control
	struct Rotary_Arm_GainAssist_Props
	{
		double PID_Up[3]; //p,i,d
		double PID_Down[3]; //p,i,d

		double InverseMaxAccel_Up;
		double InverseMaxDecel_Up;

		double InverseMaxAccel_Down;
		double InverseMaxDecel_Down;

		double SlowVelocityVoltage;  //Empirically solved as the max voltage to keep load just above steady state for worst case scenario
		double SlowVelocity;  //Rate at which the gain assist voltage gets blended out; This may be a bit more than the slow velocity used for SlowVelocityVoltage
		double GainAssistAngleScalar;  //Convert gear ratio into the readable ratio for cos() (i.e. GearToArmRatio)
		double ToleranceConsecutiveCount;
		//In milliseconds predict what the position will be by using the potentiometers velocity to help compensate for lag
		double VelocityPredictUp;
		double VelocityPredictDown;

		double PulseBurstTimeMs;  //Time in milliseconds for how long to enable pulse burst  (if zero this is disabled)
		double PulseBurstRange;  //Extended tolerance time to activate pulse burst
		bool UsePID_Up_Only;
	} ArmGainAssist;
};

class COMMON_API Rotary_System : public Ship_1D
{
	private:
		#ifndef Robot_TesterCode
		typedef Ship_1D __super;
		#endif
		bool m_UsingRange_props;
	protected:
		static void InitNetworkProperties(const Rotary_Props &props,bool AddArmAssist=false);  //This will PutVariables of all properties needed to tweak PID and gain assists
		static void NetworkEditProperties(Rotary_Props &props,bool AddArmAssist=false);  //This will GetVariables of all properties needed to tweak PID and gain assists

		PolynomialEquation_forth m_VoltagePoly;
	public:
		Rotary_System(const char EntityName[]) : Ship_1D(EntityName),m_UsingRange_props(false) {}
		//Cache the m_UsingRange props so that we can know what to set back to
		virtual void Initialize(Base::EventMap& em,const Entity1D_Properties *props=NULL) 
		{
			__super::Initialize(em,props);  //must call predecessor first!
			m_UsingRange_props=m_Ship_1D_Props.UsingRange;
		}
		//This is basically like m_UsingRange from Ship_1D except that it is dynamic, and disabled when potentiometer is disabled (as we cannot detect limits)
		bool GetUsingRange_Props() const {return m_UsingRange_props;}
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///It currently has a single PID (Dual PID may either be integrated or a new class)... to manage voltage error.  This is used for fixed point
/// position setting... like a turret or arm
class COMMON_API Rotary_Position_Control : public Rotary_System
{
	public:
		enum PotUsage
		{
			eNoPot, //Will never read them (ideal for systems that do not have any encoders)
			ePassive,  //Will read them but never alter velocities
			eActive, //Will attempt to match predicted velocity to actual velocity
		};

	private:
		#ifndef Robot_TesterCode
		typedef Rotary_System __super;
		#endif

		//Copy these lines to the subclass that binds the events
		//events are a bit picky on what to subscribe so we'll just wrap from here
		//void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

		/// \param DisableFeedback this allows ability to bypass feedback
		Rotary_Control_Interface * const m_RobotControl;
		const size_t m_InstanceIndex;
		PIDController2 m_PIDControllerUp;
		PIDController2 m_PIDControllerDown;
		Rotary_Props m_Rotary_Props;

		double m_LastPosition;  //used for calibration
		double m_MatchVelocity;
		double m_ErrorOffset;
		double m_LastTime; //used for calibration
		double m_MaxSpeedReference; //used for calibration
		double m_PreviousVelocity; //used to compute acceleration
		//We use the negative sign bit to indicate it was turned off... or zero
		double m_BurstIntensity;  //This keeps track of the current level of burst to apply... it usually is full 1.0 or 0.0 but will blend on unaligned frame boundaries
		double m_CurrentBurstTime; //This keeps track of the time between bursts and the burst itself depending on the current state
		PotUsage m_PotentiometerState; //dynamically able to turn off (e.g. panic button)
		//A counter to count how many times the predicted position and intended position are withing tolerance consecutively
		size_t m_ToleranceCounter;
	public:
		Rotary_Position_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
		//This is optionally used to lock to another ship (e.g. drive using rotary system)
		void SetMatchVelocity(double MatchVel) {m_MatchVelocity=MatchVel;}
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void SetPotentiometerSafety(bool DisableFeedback);
		PotUsage GetPotUsage() const {return m_PotentiometerState;}
		virtual double GetMatchVelocity() const {return m_MatchVelocity;}
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///This models itself much like the drive train and encoders where it allows an optional encoder sensor read back to calibrate.
///This is a kind of speed control system that manages the velocity and does not need to keep track of position (like the drive or a shooter)
class COMMON_API Rotary_Velocity_Control : public Rotary_System
{
	public:
		enum EncoderUsage
		{
			eNoEncoder, //Will never read them (ideal for systems that do not have any encoders)
			ePassive,  //Will read them but never alter velocities
			eActive, //Will attempt to match predicted velocity to actual velocity
		};
	private:
		#ifndef Robot_TesterCode
		typedef Rotary_System __super;
		#endif

		//Copy these lines to the subclass that binds the events
		//events are a bit picky on what to subscribe so we'll just wrap from here
		//void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

		/// \param DisableFeedback this allows ability to bypass feedback
		Rotary_Control_Interface * const m_RobotControl;
		const size_t m_InstanceIndex;
		PIDController2 m_PIDController;
		Rotary_Props m_Rotary_Props;
		#ifdef __Rotary_UseInducedLatency__
		LatencyFilter m_PID_Input_Latency;
		#else
		LatencyPredictionFilter m_PID_Input_Latency;
		#endif

		//We have both ways to implement PID calibration depending on if we have aggressive stop property enabled
		double m_MatchVelocity;
		double m_CalibratedScaler; //used for calibration
		double m_ErrorOffset; //used for calibration

		double m_MaxSpeedReference; //used for calibration
		double m_EncoderVelocity;  //cache for later use
		double m_RequestedVelocity_Difference;
		EncoderUsage m_EncoderState; //dynamically able to change state
		double m_PreviousVelocity; //used to compute acceleration
	public:
		Rotary_Velocity_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0,EncoderUsage EncoderState=eNoEncoder);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		double GetRequestedVelocity_Difference() const {return m_RequestedVelocity_Difference;}
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
		//This is optionally used to lock to another ship (e.g. drive using rotary system)
		void SetMatchVelocity(double MatchVel) {m_MatchVelocity=MatchVel;}
		//Give ability to change properties
		void UpdateRotaryProps(const Rotary_Props &RotaryProps);
		virtual void SetEncoderSafety(bool DisableFeedback);
		EncoderUsage GetEncoderUsage() const {return m_EncoderState;}
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);

		virtual bool InjectDisplacement(double DeltaTime_s,double &PositionDisplacement);
		virtual double GetMatchVelocity() const {return m_MatchVelocity;}
};

class COMMON_API Rotary_Properties : public Ship_1D_Properties
{
	public:
		void Init();
		Rotary_Properties(const char EntityName[], double Mass,double Dimension,
			double MAX_SPEED,double ACCEL,double BRAKE,double MaxAccelForward, double MaxAccelReverse,	
			Ship_Type ShipType=Ship_1D_Props::eDefault, bool UsingRange=false, double MinRange=0.0, double MaxRange=0.0,
			bool IsAngular=false) : Ship_1D_Properties(EntityName,Mass,Dimension,MAX_SPEED,ACCEL,BRAKE,MaxAccelForward,
			MaxAccelReverse,ShipType,UsingRange,MinRange,MaxRange,IsAngular) {Init();}

		Rotary_Properties() {Init();}
		virtual void LoadFromScript(Scripting::Script& script);
		const Rotary_Props &GetRotaryProps() const {return m_RotaryProps;}
		//Get and Set the properties
		Rotary_Props &RotaryProps() {return m_RotaryProps;}
		#ifdef Robot_TesterCode
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation.GetEncoderSimulationProps();}
		EncoderSimulation_Props &EncoderSimulationProps() {return m_EncoderSimulation.EncoderSimulationProps();}
		#endif
	protected:
		Rotary_Props m_RotaryProps;
		#ifdef Robot_TesterCode
		EncoderSimulation_Properties m_EncoderSimulation;
		#endif
	private:
		#ifndef Robot_TesterCode
		typedef Ship_1D_Properties __super;
		#endif
};
