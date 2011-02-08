#pragma once

class Entity2D;
class Entity_Properties
{
	public:
		Entity_Properties();
		void Initialize(Entity2D *NewEntity) const;
	protected:
		std::string m_EntityName;  //derived classes can let base class know what type to read
	private:
		//std::string m_NAME;  <-do not need this
		//Stuff needed for physics
		double m_Mass;
		double m_Dimensions[2]; //Dimensions- Length Width

		//Note: this is in Transmitted entity, but I don't think it belongs here as it doesn't describe what, but rather where
		//! Positions in meters, rotation in degrees
		//double m_X, m_Y, m_Heading;
};

class Ship_2D;
class Ship_Properties : public Entity_Properties
{
	public:
		Ship_Properties();
		void Initialize(Ship_2D *NewShip) const;
		double GetEngagedMaxSpeed() const {return m_ENGAGED_MAX_SPEED;}
	private:
		// This is the rate used by the keyboard
		double m_dHeading;

		//May need these later to simulate pilot error in the AI
		//! G-Force limits
		//double StructuralDmgGLimit, PilotGLimit, PilotTimeToPassOut, PilotTimeToRecover, PilotMaxTimeToRecover;

		//! We can break this up even more if needed
		double m_EngineRampForward,m_EngineRampReverse,m_EngineRampAfterBurner;
		double m_EngineDeceleration,m_EngineRampStrafe;

		//! Engaged max speed is basically the fastest speed prior to using after-burner.  For AI and auto pilot it is the trigger speed to
		//! enable the afterburner
		double m_MAX_SPEED,m_ENGAGED_MAX_SPEED;
		double m_ACCEL, m_BRAKE, m_STRAFE, m_AFTERBURNER_ACCEL, m_AFTERBURNER_BRAKE;

		double m_MaxAccelLeft,m_MaxAccelRight,m_MaxAccelForward,m_MaxAccelReverse;
		double m_MaxTorqueYaw;
};
