// ThrustShip2_TransmittedEntity.h
#pragma once

namespace Fringe
{
	namespace Base
	{
		struct WayPoint
		{
			WayPoint() : Power(0.0), Position(0,0,0),TurnSpeedScaler(1.0) {}
			double Power;
			double TurnSpeedScaler;  //This will have a default value if not in script
			osg::Vec3d Position;
		};

		struct FollowShip
		{
			FollowShip() : ShipID(""), Ship(NULL), RelPosition(0,0,0), TrajectoryPosition(0,0,0), IsTrajectoryPositionComputed(false) {}
			std::string ShipID;
			GG_Framework::AppReuse::DestroyableEntity* Ship;
			osg::Vec3d RelPosition;
			//This is computed once the bounding sphere has been initialized
			osg::Vec3d TrajectoryPosition;
			bool IsTrajectoryPositionComputed;  //pedantic way to handle the valve operation
		};

		struct FlightCharacteristics
		{
			FlightCharacteristics();

			//[James Says) I have found that I do not need these in acceleration form like I do for position, but unfortunately this means I'm inconsistent
			void HackTheMass(double mass)
			{
				MaxTorqueYaw*=mass;
				MaxTorquePitch*=mass;
				MaxTorqueRoll*=mass;
			}

			double MaxAccelLeft,MaxAccelRight,MaxAccelUp,MaxAccelDown,MaxAccelForward,MaxAccelReverse;
			double MaxTorqueYaw,MaxTorquePitch,MaxTorqueRoll;
			double Camera_Restraint;
			double G_Dampener;

			void WriteToBitStream(RakNet::BitStream& bs);
			void LoadFromBitStream(RakNet::BitStream& bs);
			void LoadFromScript(GG_Framework::Logic::Scripting::Script& script, double defAccel, double defBrake, double defStrafe);
		};

		struct ThrustShip2_TransmittedEntity : public GG_Framework::AppReuse::DestroyableEntity_TransmittedEntity
		{
			//! Write the entity into the BitStream to be sent, WITHOUT the header
			virtual void WriteToBitStream(RakNet::BitStream& bs);

			FlightCharacteristics NormalFlight_Characteristics, Afterburner_Characteristics, Braking_Characteristics;

			// These are the rates used by the keyboard, and will be modified by Camera_Restraint
			double dHeading, dPitch, dRoll;

			//! G-Force limits
			double StructuralDmgGLimit, PilotGLimit, PilotTimeToPassOut, PilotTimeToRecover, PilotMaxTimeToRecover;

			//! We can break this up even more if needed
			double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
			double EngineDeceleration,EngineRampStrafe;

			//! Engaged max speed is basically the fastest speed prior to using after-burner.  For AI and auto pilot it is the trigger speed to
			//! enable the afterburner
			double MAX_SPEED,ENGAGED_MAX_SPEED;
			double ACCEL, BRAKE, STRAFE, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE;

			bool CanUserPilot;
			std::vector<WayPoint> WayPoints;
			FollowShip ShipToFollow;
			std::string TARGET_SHIP;

			std::string LEAD_RET_OSGV;
			std::string INRANGE_LEAD_RET_OSGV;
			std::string FWD_RET_OSGV;

			std::vector<GG_Framework::AppReuse::CannonDesc> Cannons;

			//! Build a Transmitted entity from the LUA Script
			//! This assumes that there is a Table on the top of the LUA stack
			virtual void LoadFromScript(
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
				GG_Framework::Logic::ITransmittedEntityOwner* entityOwner);

			//! Build a Transmitted entity from a received Packet data
			//! Apply this AFTER any packet identifiers
			virtual void LoadFromBitStream(RakNet::BitStream& bs);
		};
	}
}