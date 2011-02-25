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