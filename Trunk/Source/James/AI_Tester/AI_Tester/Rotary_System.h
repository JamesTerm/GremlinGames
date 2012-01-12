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
	public:
		Rotary_Linear(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex=0);
		IEvent::HandlerList ehl;
		//The parent needs to call initialize
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual void ResetPos();
	protected:
		//Intercept the time change to obtain current height as well as sending out the desired velocity
		virtual void TimeChange(double dTime_s);
		virtual void PosDisplacementCallback(double posDisplacement_m);
	private:
		//typedef Ship_1D __super;

		//Copy these lines to the subclass that binds the events
		//events are a bit picky on what to subscribe so we'll just wrap from here
		//void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

		/// \param DisableFeedback this allows ability to bypass feedback
		void SetPotentiometerSafety(bool DisableFeedback);
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
};

class Rotary_Properties : public Ship_1D_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		Rotary_Properties();

		const Rotary_Props &GetRoteryProps() const {return m_RoteryProps;}
	protected:
		Rotary_Props m_RoteryProps;
};
