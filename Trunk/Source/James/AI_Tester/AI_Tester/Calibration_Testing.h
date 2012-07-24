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
		Encoder_Simulator m_LeftEncoder;
		Encoder_Simulator m_RightEncoder;
};