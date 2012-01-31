#pragma once


class FRC_2012_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
};

class FRC_2012_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			eTurret,
			ePitchRamp,
			ePowerWheels,
			eLowerConveyor,
			eMiddleConveyor,
			eFireConveyor
		};

		enum BoolSensorDevices
		{
			eLowerConveyor_Sensor,
			eMiddleConveyor_Sensor,
			eFireConveyor_Sensor
		};

		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		//TODO change UseEncoders to be passive
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		class Turret : public Rotary_Linear
		{
			public:
				Turret(Rotary_Control_Interface *robot_control,FRC_2012_Robot *parent);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
		};

		class PitchRamp : public Rotary_Linear
		{
			public:
				PitchRamp(Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
		};

		class PowerWheels : public Rotary_Angular
		{
			public:
				PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Angular __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetEncoderSafety(bool DisableFeedback) {__super::SetEncoderSafety(DisableFeedback);}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
			private:
				FRC_2012_Robot * const m_pParent;
				bool m_IsRunning;
		};

		class BallConveyorSystem
		{
			private:
				FRC_2012_Robot * const m_pParent;
				Rotary_Angular m_LowerConveyor,m_MiddleConveyor,m_FireConveyor;
				bool m_Grip,m_Squirt,m_Fire;
			public:
				BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_Fire;}

				IEvent::HandlerList ehl;
				//public access needed for goals
				void Fire(bool on) {m_Fire=on;}
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_Grip=on;}
				void Squirt(bool on) {m_Squirt=on;}

				void ResetPos() {m_LowerConveyor.ResetPos(),m_MiddleConveyor.ResetPos(),m_FireConveyor.ResetPos();}
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				void BindAdditionalEventControls(bool Bind);
			protected:
				void SetRequestedVelocity_FromNormalized(double Velocity);
		};
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		typedef  Tank_Robot __super;
		FRC_2012_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PitchRamp m_PitchRamp;
		PowerWheels m_PowerWheels;
		BallConveyorSystem m_BallConveyorSystem;

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;
		bool m_IsTargeting;
};

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		FRC_2012_Robot_Properties();

		const Ship_1D_Properties &GetTurretProps() const {return m_TurretProps;}
		const Ship_1D_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Ship_1D_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Ship_1D_Properties &GetConveyorProps() const {return m_ConveyorProps;}

	private:
		Rotary_Properties m_TurretProps,m_PitchRampProps,m_PowerWheelProps,m_ConveyorProps;
};

/// This contains all UI controls specific to this years robot.  Since we do not use files the primary use of this is specific keys assigned
class FRC_2012_UI_Controller : public UI_Controller
{
	public:
		FRC_2012_UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller=NULL);
};

