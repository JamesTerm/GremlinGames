#pragma once

class Potentiometer_Tester : public Ship_1D
{
	public:
		Potentiometer_Tester();
		void UpdatePotentiometerVelocity(double Velocity);
		virtual double GetPotentiometerCurrentPosition();
		virtual void TimeChange(double dTime_s);
	protected:
	private:
		Ship_1D_Properties m_PotentiometerProps;
		GG_Framework::Base::EventMap m_DummyMap;
};