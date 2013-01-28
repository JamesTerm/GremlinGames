#pragma once
#undef __UsingTestingKit__

#ifdef __UsingTestingKit__

class Servo_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Servo_Robot_Control(bool UseSafety);
		virtual ~Servo_Robot_Control(); 
		void SetSafety(bool UseSafety) {}

		//This is only needed for simulation
		virtual void Tank_Drive_Control_TimeChange(double dTime_s);
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		double RPS_To_LinearVelocity(double RPS);
	protected:
		
		Servo m_YawControl;
		double m_LastYawAxisSetting;  //needed to creep up the angle to position smoothly when testing servo code
		double m_LastLeftVelocity,m_LastRightVelocity;

		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use

		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
	public:
		double Get_dTime_s() const {return m_dTime_s;}
};

#endif
