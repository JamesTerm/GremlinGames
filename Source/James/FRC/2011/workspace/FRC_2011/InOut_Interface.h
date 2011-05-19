#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Robot_Control : public Robot_Control_Interface
{
	RobotDrive m_RobotDrive;
	RobotDrive m_ArmMotor;
	Compressor m_Compress;
	Solenoid m_OnClaw,m_OffClaw;
	Solenoid m_OnDeploy,m_OffDeploy;
	Encoder m_LeftEncoder,m_RightEncoder;
	//Servo m_DeployDoor,m_LazySusan;
	AnalogChannel m_Potentiometer;

	double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
	double m_ArmMaxSpeed;
	public:
		Robot_Control(bool UseSafety);
		virtual ~Robot_Control(); 
		virtual void Initialize(const Entity_Properties *props);
		void SetSafety(bool UseSafety);
	protected: //from Robot_Control_Interface
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		virtual void UpdateArmVoltage(double Voltage);
		virtual double GetArmCurrentPosition();
		virtual void CloseClaw(bool Close) {m_OnClaw.Set(Close),m_OffClaw.Set(!Close);}
		virtual void CloseDeploymentDoor(bool Close) {m_OnDeploy.Set(Close),m_OffDeploy.Set(!Close);}
		//virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
		//virtual void ReleaseLazySusan(bool Release) {m_LazySusan.SetAngle(Release?Servo::GetMaxAngle():Servo::GetMinAngle());}
	private:
		KalmanFilter m_KalFilter;
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
