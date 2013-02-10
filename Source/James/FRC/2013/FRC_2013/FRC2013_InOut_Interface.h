#pragma once

class FRC_2013_Robot_Control : public FRC_2013_Control_Interface
{
	protected: //from Robot_Control_Interface
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		
		#ifdef __UsingTestingKit__
		Servo_Robot_Control m_TankRobotControl;  //for x-axis control
		Servo m_PitchAxis;
		double m_LastAxisSetting;  //needed to creep up the angle to position smoothly
		#else
		Tank_Robot_Control m_TankRobotControl;
		#endif
		
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members

		Victor m_PowerWheel_First_Victor,m_PowerWheel_Second_Victor,m_Helix_Victor;
		Victor m_IntakeMotor_Victor,m_Rollers_Victor,m_IntakeDeployment_Victor;
		//pitch ramp is using i2c
		Compressor m_Compress;
		DoubleSolenoid m_EngageDrive,m_EngageLiftWinch,m_EngageDropWinch,m_EngageFirePiston;
		
		Encoder2 m_IntakeDeployment_Encoder, m_PowerWheel_First_Encoder,m_PowerWheel_Second_Encoder;
		//AnalogChannel m_Potentiometer;
		//Cached from properties
		//double m_ArmMaxSpeed;
		
	private:
		//probably will not need these
		//KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		KalmanFilter m_PowerWheelFilter;
		Averager<double,5> m_PowerWheelAverager;
		Priority_Averager m_PowerWheel_PriorityAverager;
		
	public:
		FRC_2013_Robot_Control(bool UseSafety);
		virtual ~FRC_2013_Robot_Control();
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2013_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2013_Robot_Properties &GetRobotProps() {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Note: If the motors are reversed, this is now solved in LUA
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}

	protected: //from FRC_2012_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
};
