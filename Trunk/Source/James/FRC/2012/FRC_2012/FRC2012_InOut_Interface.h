#pragma once

class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	protected: //from Robot_Control_Interface
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
	
		//Victor m_ArmMotor,m_RollerMotor;
		//TODO see if we are going to use Solenoid (Check with Parker)
		//Compressor m_Compress;
		//Solenoid m_OnRist,m_OffRist;
		
		//AnalogChannel m_Potentiometer;
		//Cached from properties
		//double m_ArmMaxSpeed;

		FRC_2012_CameraProcessing m_Camera;
		
	private:
		//probably will not need these
		//KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;

	public:
		FRC_2012_Robot_Control(bool UseSafety);
		virtual ~FRC_2012_Robot_Control();
		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}

		FRC_2012_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2012_Robot_Properties &GetRobotProps() {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		//Note: If the motors are reversed, this is where to fix the issue (see 2011)
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
