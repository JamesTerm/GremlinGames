#pragma once

#undef __Rotary_UseScalerPID__
#undef __Rotary_UseInducedLatency__
#undef __Rotary_ShowEncoderPrediction__

struct Rotary_Props
{
	double VoltageScalar;		//Used to handle reversed voltage wiring
	//The gear reduction used when multiplied by the encoder RPS will equal the *Rotary System's* RPS
	//Note: This is not necessarily the multiply to match the motor speed as there may be some gearing to the system as well
	double EncoderToRS_Ratio;
	double PID[3]; //p,i,d
	double InputLatency;  //Used with PID to help avoid oscillation in the error control (We can make one for each if needed)
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	//Currently supporting 4 terms in polynomial equation
	double Polynomial[5];  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel;  //used for deceleration case
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	enum LoopStates
	{
		eNone, //Will never read them (ideal for systems that do not have any encoders)
		eOpen,  //Will read them but never alter velocities
		eClosed, //Will attempt to match predicted velocity to actual velocity
	} LoopState; //This should always be false once control is fully functional
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)
	//Only supported in RoteryAngular
	bool UseAggressiveStop;  //If true, will use adverse force to assist in stopping.  Recommended not to use I to avoid thrashing
};

class Rotary_System : public Ship_1D
{
	private:
		//typedef Ship_1D __super;
		bool m_UsingRange_props;
	public:
		Rotary_System(const char EntityName[]) : Ship_1D(EntityName),m_UsingRange_props(false) {}
		//Cache the m_UsingRange props so that we can know what to set back to
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL) 
		{
			__super::Initialize(em,props);  //must call predecessor first!
			m_UsingRange_props=m_UsingRange;
		}
		bool GetUsingRange_Props() const {return m_UsingRange_props;}
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///It currently has a single PID (Dual PID may either be integrated or a new class)... to manage voltage error.  This is used for fixed point
/// position setting... like a turret or arm
class Rotary_Position_Control : public Rotary_System
{
	private:
		//typedef Rotary_System __super;

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

		double m_LastPosition;  //used for calibration
		double m_MatchVelocity;
		#ifdef __UseScalerPID__
		double m_CalibratedScaler; //used for calibration
		#else
		double m_ErrorOffset;
		#endif
		double m_LastTime; //used for calibration
		double m_MaxSpeedReference; //used for calibration
		bool m_UsingPotentiometer; //dynamically able to turn off (e.g. panic button)
		#ifdef __UseScalerPID__
		bool m_VoltageOverride;  //when true will kill voltage
		#endif
	public:
		Rotary_Position_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
		//This is optionally used to lock to another ship (e.g. drive using rotary system)
		void SetMatchVelocity(double MatchVel) {m_MatchVelocity=MatchVel;}
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		#ifdef __UseScalerPID__
		virtual void PosDisplacementCallback(double posDisplacement_m);
		#endif
		virtual void SetPotentiometerSafety(bool DisableFeedback);
		bool GetIsUsingPotentiometer() const {return m_UsingPotentiometer;}
		virtual double GetMatchVelocity() const {return m_MatchVelocity;}
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///This models itself much like the drive train and encoders where it allows an optional encoder sensor read back to calibrate.
///This is a kind of speed control system that manages the velocity and does not need to keep track of position (like the drive or a shooter)
class Rotary_Velocity_Control : public Rotary_System
{
	public:
		enum EncoderUsage
		{
			eNoEncoder, //Will never read them (ideal for systems that do not have any encoders)
			ePassive,  //Will read them but never alter velocities
			eActive, //Will attempt to match predicted velocity to actual velocity
		};
	private:
		//typedef Rotary_System __super;

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
		const EncoderUsage m_EncoderCachedState; //This is a fall-back state upon recovery of the safety state
		double m_PreviousVelocity; //used to compute acceleration
	public:
		Rotary_Velocity_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0,EncoderUsage EncoderState=eNoEncoder);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		double GetRequestedVelocity_Difference() const {return m_RequestedVelocity_Difference;}
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
		//This is optionally used to lock to another ship (e.g. drive using rotary system)
		void SetMatchVelocity(double MatchVel) {m_MatchVelocity=MatchVel;}
		//Give ability to change properties
		void UpdateRotaryProps(const Rotary_Props &RotaryProps);
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		virtual void SetEncoderSafety(bool DisableFeedback);

		virtual bool InjectDisplacement(double DeltaTime_s,double &PositionDisplacement);
		EncoderUsage GetEncoderUsage() const {return m_EncoderCachedState;}
		virtual double GetMatchVelocity() const {return m_MatchVelocity;}
};

class Rotary_Properties : public Ship_1D_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		void Init();
		Rotary_Properties(const char EntityName[], double Mass,double Dimension,
			double MAX_SPEED,double ACCEL,double BRAKE,double MaxAccelForward, double MaxAccelReverse,	
			Ship_Type ShipType=Ship_1D_Props::eDefault, bool UsingRange=false, double MinRange=0.0, double MaxRange=0.0,
			bool IsAngular=false) : Ship_1D_Properties(EntityName,Mass,Dimension,MAX_SPEED,ACCEL,BRAKE,MaxAccelForward,
			MaxAccelReverse,ShipType,UsingRange,MinRange,MaxRange,IsAngular) {Init();}

		Rotary_Properties() {Init();}
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Rotary_Props &GetRoteryProps() const {return m_RoteryProps;}
		//Get and Set the properties
		Rotary_Props &RoteryProps() {return m_RoteryProps;}
		#ifdef AI_TesterCode
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation.GetEncoderSimulationProps();}
		EncoderSimulation_Props &EncoderSimulationProps() {return m_EncoderSimulation.EncoderSimulationProps();}
		#endif
	protected:
		Rotary_Props m_RoteryProps;
		#ifdef AI_TesterCode
		EncoderSimulation_Properties m_EncoderSimulation;
		#endif
	private:
		//typedef Ship_1D_Properties __super;
};
