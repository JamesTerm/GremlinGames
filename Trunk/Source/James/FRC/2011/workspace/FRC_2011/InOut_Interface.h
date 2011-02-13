#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

class Robot_Control : public Robot_Control_Interface
{
	double m_ENGAGED_MAX_SPEED;  //cache this to covert velocity to motor setting
	RobotDrive m_RobotDrive;
	public:
		Robot_Control() : m_RobotDrive(1,2,3,4) 
		{
			//I'm giving a whole second before the timeout kicks in... I do not want false positives!
			m_RobotDrive.SetExpiration(1.0);
			m_RobotDrive.SetSafetyEnabled(true);
		}
		virtual ~Robot_Control() {}
		virtual void Initialize(const Entity_Properties *props)
		{
			const Ship_Properties *ship_props=static_cast<const Ship_Properties *>(props);
			m_ENGAGED_MAX_SPEED=ship_props->GetEngagedMaxSpeed();
		}
	protected: //from Robot_Control_Interface
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity)
		{
			m_RobotDrive.SetLeftRightMotorOutputs((float)(LeftVelocity/m_ENGAGED_MAX_SPEED),(float)(RightVelocity/m_ENGAGED_MAX_SPEED));
			//m_RobotDrive.SetLeftRightMotorOutputs(0.0f,0.0f);
		}
		virtual void UpdateArmHeight(double Height_m) {}
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
