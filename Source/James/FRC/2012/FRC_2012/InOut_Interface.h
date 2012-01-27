#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Tank_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Tank_Robot_Control(bool UseSafety);
		virtual ~Tank_Robot_Control(); 
		void SetSafety(bool UseSafety);

		//This is only needed for simulation
		virtual void Tank_Drive_Control_TimeChange(double dTime_s);
		//double GetLeftVoltage() const {return m_LeftVoltage;}
		//double GetRightVoltage() const {return m_RightVoltage;}
		//void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		double RPS_To_LinearVelocity(double RPS);
	protected:
		
		Victor m_1,m_2,m_3,m_4;  //explicitly specify victor speed controllers for the robot drive
		RobotDrive m_RobotDrive;
		Encoder m_LeftEncoder,m_RightEncoder;

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
};

class FRC_2011_Robot_Control : public FRC_2011_Control_Interface
{
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
	
		Victor m_ArmMotor,m_RollerMotor;
		Compressor m_Compress;
		Solenoid m_OnRist,m_OffRist;
		Solenoid m_OnClaw,m_OffClaw;
		Solenoid m_OnDeploy,m_OffDeploy;
		
		//Servo m_DeployDoor,m_LazySusan;
		AnalogChannel m_Potentiometer;
		AxisCamera *m_Camera;  //This is a singleton, but treated as a member that is optional
	
		double m_ArmMaxSpeed;
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;

	public:
		FRC_2011_Robot_Control(bool UseSafety);
		virtual ~FRC_2011_Robot_Control(); 
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2011_Control_Interface &AsControlInterface() {return *this;}
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
};


class Driver_Station_Joystick : public Framework::Base::IJoystick
{	
	public:
		//Note: this current configuration requires the two joysticks reside in adjacent ports (e.g. 2,3)
		Driver_Station_Joystick(int NoJoysticks,int StartingPort);
		virtual ~Driver_Station_Joystick();
	protected:  //from IJoystick
		virtual size_t GetNoJoysticksFound();
		virtual bool read_joystick (size_t nr, JoyState &Info);
		
		virtual const JoystickInfo &GetJoyInfo(size_t nr) const {return m_JoyInfo[nr];}
	private:
		std::vector<JoystickInfo> m_JoyInfo;
		DriverStation *m_ds;
		int m_NoJoysticks,m_StartingPort;
};

#endif /*INOUT_INTERFACE_H_*/
