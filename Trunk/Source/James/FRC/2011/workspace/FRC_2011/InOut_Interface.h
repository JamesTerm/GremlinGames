#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Robot_Control : public Robot_Control_Interface
{
	RobotDrive m_RobotDrive;
	RobotDrive m_ArmMotor;
	Compressor m_Compress;
	Solenoid m_OnClaw,m_OffClaw;
	Encoder m_LeftEncoder,m_RightEncoder;
	Servo m_DeployDoor;

	double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
	double m_ArmMaxSpeed;
	public:
		Robot_Control(bool UseSafety) : m_RobotDrive(1,2,3,4),m_ArmMotor(5,6),m_Compress(5,2),m_OnClaw(2),m_OffClaw(1),
			m_LeftEncoder(4,3,4,4),m_RightEncoder(4,1,4,2),m_DeployDoor(1)
		{
			m_Compress.Start();
			if (UseSafety)
			{
				//I'm giving a whole second before the timeout kicks in... I do not want false positives!
				m_RobotDrive.SetExpiration(1.0);
				m_RobotDrive.SetSafetyEnabled(true);
			}
			else
				m_RobotDrive.SetSafetyEnabled(false);
			m_LeftEncoder.Start(),m_RightEncoder.Start();
		}
		virtual ~Robot_Control() 
		{
			m_LeftEncoder.Stop(),m_RightEncoder.Stop();  //TODO Move for autonomous mode only
			m_RobotDrive.SetSafetyEnabled(false);
			m_Compress.Stop();
		}
		virtual void Initialize(const Entity_Properties *props);
	protected: //from Robot_Control_Interface
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity);
		virtual void UpdateArmVelocity(double Velocity);
		virtual double GetArmCurrentPosition();
		virtual void CloseClaw(bool Close) {m_OnClaw.Set(Close),m_OffClaw.Set(!Close);}
		virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
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
