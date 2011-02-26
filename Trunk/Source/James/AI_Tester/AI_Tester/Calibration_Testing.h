#pragma once

class Potentiometer_Tester : public Ship_1D
{
	public:
		Potentiometer_Tester();
		void UpdatePotentiometerVelocity(double Velocity);
		virtual double GetPotentiometerCurrentPosition();
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_PotentiometerProps;
		GG_Framework::Base::EventMap m_DummyMap;
};

class Encoder_Simulator : public Ship_1D
{
	public:
		Encoder_Simulator(const char EntityName[]);
		void UpdateEncoderVoltage(double Voltage);
		virtual double GetEncoderVelocity();
		//This is broken up so that the real interface does not have to pass time
		void SetTimeDelta(double dTime_s) {m_Time_s=dTime_s;}
		void TimeChange();
	protected:
	private:
		double m_Time_s;
		Ship_1D_Properties m_EncoderProps;
		GG_Framework::Base::EventMap m_DummyMap;
};

class Encoder_Tester
{
	public:
		Encoder_Tester();
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		void SetTimeDelta(double dTime_s);
		void TimeChange();
	private:
		Encoder_Simulator m_LeftEncoder;
		Encoder_Simulator m_RightEncoder;
};