#pragma once

///This class is an Encoder that provides a more accurate way to obtain the current rate
class Encoder2 : public Encoder
{
	public:
		Encoder2(UINT32 aChannel, UINT32 bChannel, bool reverseDirection=false);
		///This uses a different technique of obtaining the rate by use of the actual pulse count.  This should produce less noisy results
		///at higher speeds.  This is simple as it does not support overrlap as it would take about 5 continuous 
		///hours at 78 rps before the end is reached
		/// \param dTime_s delta time slice in seconds
		double GetRate2(double dTime_s);
		///unfortunately reset is not virtual... client code should call this if using reset
		void Reset2();
	private:
		double m_LastDistance;  //keep note of last distance
};

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
		Encoder2 m_LeftEncoder,m_RightEncoder;

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use

		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
	private:
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
	public:
		double Get_dTime_s() const {return m_dTime_s;}
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
