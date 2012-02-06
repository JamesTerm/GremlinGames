#pragma once

struct Rotary_Props
{
	double PID[3]; //p,i,d
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///It currently has a single PID (Dual PID may either be integrated or a new class)... to manage voltage error
class Rotary_Linear : public Ship_1D
{
	private:
		typedef Ship_1D __super;

		//Copy these lines to the subclass that binds the events
		//events are a bit picky on what to subscribe so we'll just wrap from here
		//void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

		/// \param DisableFeedback this allows ability to bypass feedback
		Rotary_Control_Interface * const m_RobotControl;
		const size_t m_InstanceIndex;
		PIDController2 m_PIDController;
		Rotary_Props m_Rotary_Props;
		double m_LastPosition;  //used for calibration
		double m_CalibratedScaler; //used for calibration
		double m_LastTime; //used for calibration
		double m_MaxSpeedReference; //used for calibration
		bool m_UsingPotentiometer; //dynamically able to turn off (e.g. panic button)
		bool m_VoltageOverride;  //when true will kill voltage
	public:
		Rotary_Linear(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void PosDisplacementCallback(double posDisplacement_m);
		virtual void SetPotentiometerSafety(bool DisableFeedback);
		bool GetIsUsingPotentiometer() const {return m_UsingPotentiometer;}
};

///This is the next layer of the linear Ship_1D that converts velocity into voltage, on a system that has sensor feedback
///This models itself much like the drive train and encoders where it allows an optional encoder sensor read back to calibrate
class Rotary_Angular : public Ship_1D
{
	public:
		enum EncoderUsage
		{
			eNoEncoder, //Will never read them (ideal for systems that do not have any encoders)
			ePassive,  //Will read them but never alter velocities
			eActive, //Will attempt to match predicted velocity to actual velocity
		};
	private:
		typedef Ship_1D __super;

		//Copy these lines to the subclass that binds the events
		//events are a bit picky on what to subscribe so we'll just wrap from here
		//void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

		/// \param DisableFeedback this allows ability to bypass feedback
		Rotary_Control_Interface * const m_RobotControl;
		const size_t m_InstanceIndex;
		PIDController2 m_PIDController;
		Rotary_Props m_Rotary_Props;
		double m_CalibratedScaler; //used for calibration
		double m_MaxSpeedReference; //used for calibration
		double m_EncoderVelocity;  //cache for later use
		double m_RequestedVelocity_Difference;
		EncoderUsage m_EncoderState; //dynamically able to change state
		const EncoderUsage m_EncoderCachedState; //This is a fall-back state upon recovery of the safety state
		bool m_VoltageOverride;  //when true will kill voltage
	public:
		Rotary_Angular(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0,EncoderUsage EncoderState=eNoEncoder);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
		double GetRequestedVelocity_Difference() const {return m_RequestedVelocity_Difference;}
		const Rotary_Props &GetRotary_Properties() const {return m_Rotary_Props;}
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		virtual void SetEncoderSafety(bool DisableFeedback);

		virtual bool InjectDisplacement(double DeltaTime_s,double &PositionDisplacement);
};

class Rotary_Properties : public Ship_1D_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		void Init();
		Rotary_Properties(const char EntityName[], double Mass,double Dimension,
			double MAX_SPEED,double ACCEL,double BRAKE,double MaxAccelForward, double MaxAccelReverse,	
			Ship_Type ShipType=eDefault, bool UsingRange=false, double MinRange=0.0, double MaxRange=0.0,
			bool IsAngular=false) : Ship_1D_Properties(EntityName,Mass,Dimension,MAX_SPEED,ACCEL,BRAKE,MaxAccelForward,
			MaxAccelReverse,ShipType,UsingRange,MinRange,MaxRange,IsAngular) {Init();}

		Rotary_Properties() {Init();}

		const Rotary_Props &GetRoteryProps() const {return m_RoteryProps;}
		//Get and Set the properties
		Rotary_Props &RoteryProps() {return m_RoteryProps;}
	protected:
		Rotary_Props m_RoteryProps;
};
