#pragma once

//I wanted to put this at a low enough level for common code to use... this is code that should be disabled during release
#define __DebugLUA__

class Entity1D;
class Entity1D_Properties
{
	public:
		Entity1D_Properties();
		Entity1D_Properties(const char EntityName[],double Mass,double Dimension,bool IsAngular=false);
		virtual ~Entity1D_Properties() {}
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		void Initialize(Entity1D *NewEntity) const;
		double GetMass() const {return m_Mass;}
	protected:
		std::string m_EntityName;  //derived classes can let base class know what type to read
	private:
		//Stuff needed for physics
		double m_Mass;
		double m_Dimension; //Dimension- Length for linear and diameter for angular
		bool m_IsAngular;
};

class Ship_Properties;
class Ship_1D;
class Ship_1D_Properties : public Entity1D_Properties
{
	public:
		Ship_1D_Properties();
		//Allow to construct props in constructor instead of using script
		//TODO these are somewhat specific, we may want to move subclass them or have more generic meaning
		enum Ship_Type
		{
			eDefault,
			eRobotArm,
			eSimpleMotor,
			eSwivel,
		};
		Ship_1D_Properties(const char EntityName[], double Mass,double Dimension,
			double MAX_SPEED,double ACCEL,double BRAKE,double MaxAccelForward, double MaxAccelReverse,	
			Ship_Type ShipType=eDefault, bool UsingRange=false, double MinRange=0.0, double MaxRange=0.0,
			bool IsAngular=false);

		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		void Initialize(Ship_1D *NewShip) const;
		Ship_Type GetShipType() const {return m_ShipType;}
		double GetMaxSpeed() const {return m_MAX_SPEED;}
	private:
		//typedef Entity1D_Properties __super;
		//! We can break this up even more if needed
		//double m_EngineRampForward,m_EngineRampReverse,m_EngineRampAfterBurner;
		//double m_EngineDeceleration;

		//! Engaged max speed is basically the fastest speed prior to using after-burner.  For AI and auto pilot it is the trigger speed to
		//! enable the afterburner
		double m_MAX_SPEED;
		double m_ACCEL, m_BRAKE;

		double m_MaxAccelForward,m_MaxAccelReverse;
		double m_MinRange,m_MaxRange;
		Ship_Type m_ShipType;
		bool m_UsingRange;
	public:
		//These are for testing purposes only (do not use)
		void SetMinRange(double MinRange) {m_MinRange=MinRange;}
		void SetMaxRange(double MaxRange) {m_MaxRange=MaxRange;}
		void SetUsingRange(bool UsingRange) {m_UsingRange=UsingRange;}
		//copy constructor that can interpret the other type
		void SetFromShip_Properties(const Ship_Properties & NewValue);
};

class Entity2D;
class Entity_Properties
{
	public:
		Entity_Properties();
		virtual ~Entity_Properties() {}
		//The prep script takes care of the outer layer global table setup
		//override to search the appropriate global table
		virtual const char *SetUpGlobalTable(GG_Framework::Logic::Scripting::Script& script) {return script.GetGlobalTable(m_EntityName.c_str());}
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
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
