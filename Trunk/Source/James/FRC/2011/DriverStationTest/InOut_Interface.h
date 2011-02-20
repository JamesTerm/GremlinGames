#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Robot_Control : public Robot_Control_Interface
{
	RobotDrive m_RobotDrive;
	RobotDrive m_ArmMotor;
	Solenoid m_Claw;

	double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
	double m_ArmMaxSpeed;
	public:
		Robot_Control(bool UseSafety) : m_RobotDrive(1,2,3,4),m_ArmMotor(5,6),m_Claw(1)
		{
			if (UseSafety)
			{
				//I'm giving a whole second before the timeout kicks in... I do not want false positives!
				//m_RobotDrive.SetExpiration(1.0);
				//m_RobotDrive.SetSafetyEnabled(true);
			}
			//else
				//m_RobotDrive.SetSafetyEnabled(false);
		}
		virtual ~Robot_Control() {}
		virtual void Initialize(const Entity_Properties *props);
	protected: //from Robot_Control_Interface
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity);
		virtual void UpdateArmVelocity(double Velocity);
		virtual double GetArmCurrentPosition();
		virtual void CloseClaw(bool Close) {m_Claw.Set(Close);}
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
