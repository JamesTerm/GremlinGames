#pragma once

class Potentiometer_Tester : public Ship_1D
{
	public:
		Potentiometer_Tester();
		void UpdatePotentiometerVoltage(double Voltage);
		virtual double GetPotentiometerCurrentPosition();
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
		void SetBypass(bool bypass) {m_Bypass=bypass;}
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_PotentiometerProps;
		GG_Framework::Base::EventMap m_DummyMap;
		bool m_Bypass;  //used for stress test
};

class Potentiometer_Tester2 : public Ship_1D
{
	public:
		Potentiometer_Tester2();
		virtual void Initialize(const Ship_1D_Properties *props=NULL);

		void UpdatePotentiometerVoltage(double Voltage);
		virtual double GetPotentiometerCurrentPosition();
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
		void SetBypass(bool bypass) {m_Bypass=bypass;}
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_PotentiometerProps;
		GG_Framework::Base::EventMap m_DummyMap;
		bool m_Bypass;  //used for stress test
};

class Encoder_Simulator : public Ship_1D
{
	public:
		Encoder_Simulator(const char *EntityName="EncSimulator");
		virtual void Initialize(const Ship_1D_Properties *props=NULL);

		void UpdateEncoderVoltage(double Voltage);
		virtual double GetEncoderVelocity();
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
		void SetReverseDirection(bool reverseDirection);  //emulates functionality of the encoder (needed because kids put them in differently)
		void SetEncoderScalar(double value) {m_EncoderScalar=value;}  //This helps to simulate differences between sides
		void SetFriction(double StaticFriction,double KineticFriction) {}
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_EncoderProps;
		GG_Framework::Base::EventMap m_DummyMap;
		LatencyFilter m_Latency;
		double m_EncoderScalar; //used to implement reverse
		bool m_GetEncoderFirstCall;  //allows GetEncoderVelocity to know when a new set of calls occur within a time slice
};


struct EncoderSimulation_Props
{
	double Wheel_Mass;  //This is a total mass of all the wheels and gears for one side
	double COF_Efficiency;
	double GearReduction;  //In reciprocal form of spread sheet
	double TorqueAppliedOnWheelRadius; //in meters
	double DriveWheelRadius; //in meters
	double NoMotors;  //Used to get total torque

	struct Motor_Specs
	{
		double FreeSpeed_RPM;
		double Stall_Torque_NM;
		double Stall_Current_Amp;
		double Free_Current_Amp;
	} motor;
};

//This is used in calibration testing to simulate encoder readings
class EncoderSimulation_Properties
{
	public:
		EncoderSimulation_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation_Props;}
		//Get and Set the properties
		EncoderSimulation_Props &EncoderSimulationProps() {return m_EncoderSimulation_Props;}

	protected:
		EncoderSimulation_Props m_EncoderSimulation_Props;
};


class Drive_Train_Characteristics
{
	public:
		Drive_Train_Characteristics();
		void UpdateProps(const EncoderSimulation_Props &props) {m_Props=props;}

		__inline double GetAmp_To_Torque_nm(double Amps);
		__inline double GetVel_To_Torque_nm(double Vel_rps);
		__inline double GetTorque_To_Vel_nm(double Vel_rps);

		__inline double GetWheelStallTorque(double Torque);
		__inline double GetTorqueAtWheel(double Torque);
		__inline double GetWheelRPS(double LinearVelocity);
		__inline double GetMotorRPS(double LinearVelocity);
		__inline double GetWheelRPS_Angular(double AngularVelocity);
		__inline double GetMotorRPS_Angular(double AngularVelocity);
		__inline double GetTorqueFromLinearVelocity(double LinearVelocity);
		__inline double GetWheelTorqueFromVoltage(double Voltage);
		__inline double GetTorqueFromVoltage(double Voltage);
		__inline double GetTorqueFromVelocity(double AngularVelocity);
		const EncoderSimulation_Props &GetDriveTrainProps() const {return m_Props;}
		void SetGearReduction(double NewGearing) {m_Props.GearReduction=NewGearing;}
	private:
		EncoderSimulation_Props m_Props;
};

class Encoder_Simulator2
{
	public:
		Encoder_Simulator2(const char *EntityName="EncSimulator");
		virtual void Initialize(const Ship_1D_Properties *props=NULL);

		void UpdateEncoderVoltage(double Voltage);
		double GetEncoderVelocity() const;
		double GetDistance() const;

		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
		void SetReverseDirection(bool reverseDirection);  //emulates functionality of the encoder (needed because kids put them in differently)
		void SetEncoderScalar(double value) {m_EncoderScalar=value;}  //This helps to simulate differences between sides
		void SetFriction(double StaticFriction,double KineticFriction) {m_Physics.SetFriction(StaticFriction,KineticFriction);}
		void ResetPos();
		void SetGearReduction(double NewGearing) {m_DriveTrain.SetGearReduction(NewGearing);}
	protected:
	private:
		double m_Time_s;
		double m_Position;  //also keep track of position to simulate distance use case (i.e. used as a potentiometer)
		PhysicsEntity_1D m_Physics;
		double m_EncoderScalar; //used for position updates
		double m_ReverseMultiply; //used to implement set reverse direction
		Drive_Train_Characteristics m_DriveTrain;
};

class Encoder_Tester
{
	public:
		Encoder_Tester();
		virtual void Initialize(const Ship_1D_Properties *props=NULL);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		void SetTimeDelta(double dTime_s);
		void TimeChange();

		void SetLeftRightReverseDirectionEncoder(bool Left_reverseDirection,bool Right_reverseDirection)
		{
			m_LeftEncoder.SetReverseDirection(Left_reverseDirection),m_RightEncoder.SetReverseDirection(Right_reverseDirection);
		}
		void SetLeftRightScalar(double LeftScalar,double RightScalar)
		{
			m_LeftEncoder.SetEncoderScalar(LeftScalar),m_RightEncoder.SetEncoderScalar(RightScalar);
		}
	private:
		#if 0
		//This is still good for a lesser stress (keeping the latency disabled)
		Encoder_Simulator m_LeftEncoder;
		Encoder_Simulator m_RightEncoder;
		#else
		Encoder_Simulator2 m_LeftEncoder;
		Encoder_Simulator2 m_RightEncoder;
		#endif
};