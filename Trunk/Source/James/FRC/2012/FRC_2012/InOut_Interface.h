#pragma once

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
