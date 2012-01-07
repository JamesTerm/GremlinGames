#pragma once

class Robot_Control_Interface
{
	public:
		//See FRC_2011_Robot for enumerations

		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void UpdateVoltage(size_t index,double Voltage)=0;
		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void CloseSolenoid(size_t index,bool Close)=0;
		virtual void OpenSolenoid(size_t index,bool Close)=0;
};

//TODO move to Arm file
class Arm_Control_Interface
{
	public:
		virtual void Reset_Arm(size_t index=0)=0; 

		///This is a implemented by reading the potentiometer and converting its value to correspond to the arm's current angle
		///This is in radians of the arm's gear ratio
		///TODO break this apart to reading pure analog values and have the potentiometer conversion happen within the robot
		virtual double GetArmCurrentPosition(size_t index=0)=0;
		virtual void UpdateArmVoltage(size_t index,double Voltage)=0;
		virtual void CloseRist(bool Close)=0;
		virtual void OpenRist(bool Close)=0;
};

class FRC_2011_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Arm_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC_2011_Robot : public Tank_Robot_UI
{
	public:
		enum SolenoidDevices
		{
			eDeployment,
			eClaw,
			eRist
		};
		enum SpeedControllerDevices
		{
			eArm,
			eRollers
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		FRC_2011_Robot(const char EntityName[],FRC_2011_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);
		void CloseDeploymentDoor(bool Close);

		//TODO for now it it does not matter, but the ship 1d does not normalize to 2pi yet, this class would take advantage of this
		//but fortunately there currently is no use for keep track of the rotation, this however will matter if we have things which
		//rotate within the game
		class Robot_Claw : public Ship_1D
		{
			public:
				Robot_Claw(const char EntityName[],Robot_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//public access needed for goals
				void CloseClaw(bool Close);
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				//typedef Ship_1D __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				Robot_Control_Interface * const m_RobotControl;
				bool m_Grip,m_Squirt;
		};
		class Robot_Arm : public Ship_1D
		{
			public:
				Robot_Arm(const char EntityName[],Arm_Control_Interface *robot_control,size_t InstanceIndex=0);
				IEvent::HandlerList ehl;
				//The parent needs to call initialize
				virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				static double HeightToAngle_r(double Height_m);
				static double Arm_AngleToHeight_m(double Angle_r);
				static double AngleToHeight_m(double Angle_r);
				static double GetPosRest();
				//given the raw potentiometer converts to the arm angle
				static double PotentiometerRaw_To_Arm_r(double raw);
				void CloseRist(bool Close);
				virtual void ResetPos();
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void PosDisplacementCallback(double posDisplacement_m);
			private:
				//typedef Ship_1D __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetPotentiometerSafety(double Value);
				void SetPosRest();
				void SetPos0feet();
				void SetPos3feet();
				void SetPos6feet();
				void SetPos9feet();
				Arm_Control_Interface * const m_RobotControl;
				const size_t m_InstanceIndex;
				PIDController2 m_PIDController;
				double m_LastPosition;  //used for calibration
				double m_CalibratedScaler; //used for calibration
				double m_LastTime; //used for calibration
				double m_MaxSpeedReference; //used for calibration
				bool m_UsingPotentiometer; //dynamically able to turn off (e.g. panic button)
				bool m_VoltageOverride;  //when true will kill voltage
		};

		//Accessors needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
		Robot_Claw &GetClaw() {return m_Claw;}
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		//typedef  Tank_Drive __super;
		FRC_2011_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
		Robot_Claw m_Claw;
		bool m_VoltageOverride;  //when true will kill voltage
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class FRC_2011_Robot_Control : public FRC_2011_Control_Interface
{
	public:
		FRC_2011_Robot_Control();
		//This is only needed for simulation
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Close) {CloseSolenoid(index,!Close);}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Unfortunately the actual wheels are reversed (resolved here since this is this specific robot)
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Arm Interface
		virtual void Reset_Arm(size_t index=0); 
		virtual void UpdateArmVoltage(size_t index,double Voltage) {UpdateVoltage(FRC_2011_Robot::eArm,Voltage);}
		//pacify this by returning its current value
		virtual double GetArmCurrentPosition(size_t index);
		virtual void CloseRist(bool Close) {CloseSolenoid(FRC_2011_Robot::eRist,Close);}
		virtual void OpenRist(bool Close) {CloseSolenoid(FRC_2011_Robot::eRist,!Close);}
	protected: //from FRC_2011_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

	protected:
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		double m_ArmMaxSpeed;
		Potentiometer_Tester m_Potentiometer; //simulate a real potentiometer for calibration testing
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_ArmVoltage,m_RollerVoltage;
		bool m_Deployment,m_Claw,m_Rist;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2011_Robot_UI : public FRC_2011_Robot, public FRC_2011_Robot_Control
{
	public:
		FRC_2011_Robot_UI(const char EntityName[]) : FRC_2011_Robot(EntityName,this),FRC_2011_Robot_Control() {}
};

class FRC_2011_Robot_Properties : public Tank_Robot_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		FRC_2011_Robot_Properties();
		//I'm not going to implement script support mainly due to lack of time, but also this is a specific object that
		//most likely is not going to be sub-classed (i.e. sealed)... if this turns out different later we can implement
		//virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Ship_1D_Properties &GetArmProps() const {return m_ArmProps;}
		const Ship_1D_Properties &GetClawProps() const {return m_ClawProps;}

	private:
		Ship_1D_Properties m_ArmProps,m_ClawProps;
};

class Goal_OperateSolenoid : public AtomicGoal
{
	private:
		FRC_2011_Robot &m_Robot;
		const FRC_2011_Robot::SolenoidDevices m_SolenoidDevice;
		bool m_Terminate;
		bool m_IsClosed;
	public:
		Goal_OperateSolenoid(FRC_2011_Robot &robot,FRC_2011_Robot::SolenoidDevices SolenoidDevice,bool Close);
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate() {m_Terminate=true;}
};

Goal *Get_TestLengthGoal(FRC_2011_Robot *Robot);
Goal *Get_UberTubeGoal(FRC_2011_Robot *Robot);
