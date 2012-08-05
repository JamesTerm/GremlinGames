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
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_EncoderProps;
		GG_Framework::Base::EventMap m_DummyMap;
		LatencyFilter m_Latency;
		double m_EncoderScalar; //used to implement reverse
		bool m_GetEncoderFirstCall;  //allows GetEncoderVelocity to know when a new set of calls occur within a time slice
};

class Drive_Train_Characteristics
{
	public:
		__inline static double GetAmp_To_Torque_nm(double Amps);
		__inline static double GetVel_To_Torque_nm(double Vel_rps);
		__inline static double GetTorque_To_Vel_nm(double Vel_rps);
		struct DriveTrainProps
		{
			DriveTrainProps(double dt,double gr,double dwr,double mc) : 
			DriveTrain_Efficiency(dt),GearReduction(gr),DriveWheelRadius(dwr),m_NoMotors(mc) {}
			double DriveTrain_Efficiency;
			double GearReduction;  //In reciprocal form of spread sheet
			double DriveWheelRadius; //in meters
			double m_NoMotors;  //Used to get total torque
		};
		Drive_Train_Characteristics();
		__inline double GetWheelStallTorque(double Torque);
		__inline double GetTorqueAtWheel(double Torque);
		__inline double GetWheelRPS(double LinearVelocity);
		__inline double GetMotorRPS(double LinearVelocity);
		__inline double GetTorqueFromLinearVelocity(double LinearVelocity);
	private:
		DriveTrainProps m_Props;
};

class Encoder_Simulator2
{
	public:
		Encoder_Simulator2(const char *EntityName="EncSimulator");
		virtual void Initialize(const Ship_1D_Properties *props=NULL);

		void UpdateEncoderVoltage(double Voltage);
		virtual double GetEncoderVelocity() const;
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
		void SetReverseDirection(bool reverseDirection);  //emulates functionality of the encoder (needed because kids put them in differently)
		void SetEncoderScalar(double value) {m_EncoderScalar=value;}  //This helps to simulate differences between sides
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_EncoderProps;
		PhysicsEntity_1D m_Physics;
		double m_EncoderScalar; //used to implement reverse
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
		Encoder_Simulator m_LeftEncoder;
		Encoder_Simulator m_RightEncoder;
		#else
		Encoder_Simulator2 m_LeftEncoder;
		Encoder_Simulator2 m_RightEncoder;
		#endif
};