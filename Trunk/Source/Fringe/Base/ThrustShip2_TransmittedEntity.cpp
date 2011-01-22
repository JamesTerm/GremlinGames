// ThrustShip2_TransmittedEntity.cpp
#include "stdafx.h"
#include "Fringe.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace Fringe::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;

ClassFactoryT<ThrustShip2_TransmittedEntity, TransmittedEntity> ThrustShip2_TE_ClassFactory("ThrustShip2", TransmittedEntity::ClassFactory);


FlightCharacteristics::FlightCharacteristics()
{
	MaxAccelLeft = MaxAccelRight = MaxAccelUp = MaxAccelDown = 250.0;
	MaxAccelForward = MaxAccelReverse = 500.0;
	MaxTorqueYaw = MaxTorquePitch = MaxTorqueRoll = 1.0;
	G_Dampener = Camera_Restraint = 1.0; 
}
//////////////////////////////////////////////////////////////////////////

void FlightCharacteristics::WriteToBitStream(RakNet::BitStream& bs)
{
	bs.Write(MaxAccelLeft);
	bs.Write(MaxAccelRight);
	bs.Write(MaxAccelUp);
	bs.Write(MaxAccelDown);
	bs.Write(MaxAccelForward);
	bs.Write(MaxAccelReverse);
	bs.Write(MaxTorqueYaw);
	bs.Write(MaxTorquePitch);
	bs.Write(MaxTorqueRoll);
	bs.Write(G_Dampener);
	bs.Write(Camera_Restraint);
}
//////////////////////////////////////////////////////////////////////////

void FlightCharacteristics::LoadFromBitStream(RakNet::BitStream& bs)
{
	bs.Read(MaxAccelLeft);
	bs.Read(MaxAccelRight);
	bs.Read(MaxAccelUp);
	bs.Read(MaxAccelDown);
	bs.Read(MaxAccelForward);
	bs.Read(MaxAccelReverse);
	bs.Read(MaxTorqueYaw);
	bs.Read(MaxTorquePitch);
	bs.Read(MaxTorqueRoll);
	bs.Read(G_Dampener);
	bs.Read(Camera_Restraint);
}
//////////////////////////////////////////////////////////////////////////

void FlightCharacteristics::LoadFromScript(GG_Framework::Logic::Scripting::Script& script, double defAccel, double defBrake, double defStrafe)
{
	// No need to worry about errors because we have good defaults
	MaxAccelLeft = MaxAccelRight = MaxAccelUp = MaxAccelDown = defStrafe;
	MaxAccelForward = defAccel;
	MaxAccelReverse = defBrake;
	
	script.GetField("MaxAccelLeft", NULL, NULL, &MaxAccelLeft);
	script.GetField("MaxAccelRight", NULL, NULL, &MaxAccelRight);
	script.GetField("MaxAccelUp", NULL, NULL, &MaxAccelUp);
	script.GetField("MaxAccelDown", NULL, NULL, &MaxAccelDown);
	script.GetField("MaxAccelForward", NULL, NULL, &MaxAccelForward);
	script.GetField("MaxAccelReverse", NULL, NULL, &MaxAccelReverse);
	script.GetField("MaxTorqueYaw", NULL, NULL, &MaxTorqueYaw);
	script.GetField("MaxTorquePitch", NULL, NULL, &MaxTorquePitch);
	script.GetField("MaxTorqueRoll", NULL, NULL, &MaxTorqueRoll);
	script.GetField("G_Dampener", NULL, NULL, &G_Dampener);
	script.GetField("Camera_Restraint", NULL, NULL, &Camera_Restraint);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from the LUA Script
void ThrustShip2_TransmittedEntity::LoadFromScript(
	GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
	GG_Framework::Logic::ITransmittedEntityOwner* entityOwner)
{
	const char* err;

	// We will need the node name for the sake of the Cannons (I do not like this here, but it has to be done before I read myself)
	err = script.GetField("ID", &NAME);
	ASSERT_MSG(!err, err);

	CanUserPilot = true; // Optional
	script.GetField("UserCanPilot", NULL, &CanUserPilot, NULL);

	err = script.GetFieldTable("Ship");
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("Mass", NULL, NULL, &Mass);
		ASSERT_MSG(!err, err);
		
		err = script.GetField("dHeading", NULL, NULL, &dHeading);
		ASSERT_MSG(!err, err);
		err = script.GetField("dPitch", NULL, NULL, &dPitch);
		ASSERT_MSG(!err, err);
		err = script.GetField("dRoll", NULL, NULL, &dRoll);
		ASSERT_MSG(!err, err);

		err = script.GetField("ACCEL", NULL, NULL, &ACCEL);
		ASSERT_MSG(!err, err);
		err = script.GetField("BRAKE", NULL, NULL, &BRAKE);
		ASSERT_MSG(!err, err);
		err = script.GetField("STRAFE", NULL, NULL, &STRAFE);
		if (err)
			STRAFE=BRAKE; 		//Give strafe the default of the brake

		AFTERBURNER_ACCEL=ACCEL;  //Give afterburner acceleration rate the same as thrusters for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &AFTERBURNER_ACCEL);

		AFTERBURNER_BRAKE=BRAKE;  //Give afterburner brake rate the same as brakes for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &AFTERBURNER_BRAKE);


		// James, What can I change to make these work without Jitter?  I do NOT
		// want us to force these values so low, because it makes the fighter feel far to "slushy".  
		// What you are saying here is that engines always take 1 second to ramp up or ramp down.  
		// We need to be far more responsive than that for fighters.
		// Can we figure out how to fix the jitter properly rather than forcing some limit?
		// The RAMP_UP_DUR, RAMP_DOWN_DUR logic is sound for this third derivative.
		// To be honest, with recent fixes in other areas, I do not find your Camera jitters so problematic.  Are you concerned
		// with something else, like the Gforce YO-YOing?  I forgot how to duplicate that.


		//You can reproduce by using JamesJacks and simply accelerate using the w key
		//I like the idea of using duration times as this is more intuitive for content developers, so we can keep these in script and remove the other
		//entries (TODO cleanup).  For the *default* workaround, we should use a value that will not break other ships.  I'll need to figure out
		//a solution to properly support higher ramp up rates if problem persists.  For now please tweak the durations on the ships script, so
		//that we can ensure we are not breaking other ships.  I can verify the ships are working properly once this is done.  Note: I'm going
		//verify by looking at numbers.  If we get those cases working then I can hold off on making changes.
		//  [10/5/2009 James]

		// By defaults, we want all of the engines to ramp up to full in this amount of time
		double RAMP_UP_DUR = 1.0;
		double RAMP_DOWN_DUR = 1.0;

		err = script.GetField("RAMP_UP_DUR", NULL, NULL, &RAMP_UP_DUR);
		err = script.GetField("RAMP_DOWN_DUR", NULL, NULL, &RAMP_DOWN_DUR);
		const double MIN_RAMP = 0.00001;	// Avoid stupid numbers, div by 0 errors, etc.
		if (RAMP_UP_DUR < MIN_RAMP) RAMP_UP_DUR = MIN_RAMP;	
		if (RAMP_DOWN_DUR < MIN_RAMP) RAMP_DOWN_DUR = MIN_RAMP;

		err = script.GetField("EngineDeceleration", NULL, NULL, &EngineDeceleration);
		if (err) EngineDeceleration= ACCEL/RAMP_DOWN_DUR;
		err = script.GetField("EngineRampStrafe", NULL, NULL, &EngineRampStrafe);
		if (err) EngineRampStrafe= STRAFE/RAMP_UP_DUR;
		err = script.GetField("EngineRampForward", NULL, NULL, &EngineRampForward);
		if (err) EngineRampForward= ACCEL/RAMP_UP_DUR;
		err = script.GetField("EngineRampReverse", NULL, NULL, &EngineRampReverse);
		if (err) EngineRampReverse= BRAKE/RAMP_UP_DUR;
		err = script.GetField("EngineRampAfterBurner", NULL, NULL, &EngineRampAfterBurner);
		if (err) EngineRampAfterBurner= AFTERBURNER_ACCEL/RAMP_UP_DUR;
		
		err = script.GetField("StructuralDmgGLimit", NULL, NULL, &StructuralDmgGLimit);
		if (err) StructuralDmgGLimit=10.0;
		err = script.GetField("PilotGLimit", NULL, NULL, &PilotGLimit);
		if (err) PilotGLimit=6.5;
		err = script.GetField("PilotTimeToPassOut", NULL, NULL, &PilotTimeToPassOut);
		if (err) PilotTimeToPassOut=5.0;
		err = script.GetField("PilotTimeToRecover", NULL, NULL, &PilotTimeToRecover);
		if (err) PilotTimeToRecover=1.0;
		err = script.GetField("PilotMaxTimeToRecover", NULL, NULL, &PilotMaxTimeToRecover);
		if (err) PilotMaxTimeToRecover=PilotTimeToRecover*20.0;

		err = script.GetField("MAX_SPEED", NULL, NULL, &MAX_SPEED);
		ASSERT_MSG(!err, err);
		err = script.GetField("ENGAGED_MAX_SPEED", NULL, NULL, &ENGAGED_MAX_SPEED);
		if (err)
			ENGAGED_MAX_SPEED=MAX_SPEED;


		err = script.GetFieldTable("NormalFlight_Characteristics");
		if (err) // The old scripts did not break it up
			NormalFlight_Characteristics.LoadFromScript(script, ACCEL, BRAKE, STRAFE);
		else
		{
			NormalFlight_Characteristics.LoadFromScript(script, ACCEL, BRAKE, STRAFE);
			script.Pop();
		}

		err = script.GetFieldTable("Afterburner_Characteristics");
		if (err) // Just copy Normal, but use AFTERBURNER_ACCEL values
		{
			Afterburner_Characteristics=NormalFlight_Characteristics;
			Afterburner_Characteristics.MaxAccelForward = AFTERBURNER_ACCEL;
			Afterburner_Characteristics.MaxAccelReverse = AFTERBURNER_BRAKE;
		}
		else
		{
			Afterburner_Characteristics.LoadFromScript(script, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE, STRAFE);
			script.Pop();
		}

		err = script.GetFieldTable("Braking_Characteristics");
		if (err) // Just copy Normal
			Braking_Characteristics=NormalFlight_Characteristics;
		else
		{
			Braking_Characteristics.LoadFromScript(script, ACCEL, BRAKE, STRAFE);
			script.Pop();
		}
			

		err = script.GetField("LEAD_RET_OSGV", &LEAD_RET_OSGV, NULL, NULL);
		ASSERT_MSG(!err, err);
		err = script.GetField("INRANGE_LEAD_RET_OSGV", &INRANGE_LEAD_RET_OSGV, NULL, NULL);
		ASSERT_MSG(!err, err);
		err = script.GetField("FWD_RET_OSGV", &FWD_RET_OSGV, NULL, NULL);
		ASSERT_MSG(!err, err);

		// Get all of the Cannons (optional)
		err = script.GetFieldTable("Cannons");
		if (!err)
		{
			int index = 1;
			while (!(err = script.GetIndexTable(index)))
			{
				DEBUG_CANNON_FIRE("ThrustShip2_TransmittedEntity::LoadFromScript(%s) -- In Cannon Loop(%i)\n", NAME.c_str(), index);

				CannonDesc cannon;
				cannon.LoadFromScript(NAME, index, script, controllingPlayerID, entityOwner);

				Cannons.push_back(cannon);
				script.Pop();
				++index;
			}
			script.Pop();	// Pop the error one too we get off NOt getting an index
			script.Pop();   // This one is only for the Cannons, only need to do it if there was one
		}
	}
	script.Pop();

	// Get all of the Way Points (optional)
	err = script.GetFieldTable("WayPointLoop");
	if (!err)
	{
		int index = 1;
		while (!(err = script.GetIndexTable(index++)))
		{
			WayPoint wp;
			double x, y, z;
			err = script.GetField("X", NULL, NULL, &x);
			ASSERT_MSG(!err, err);
			err = script.GetField("Y", NULL, NULL, &y);
			ASSERT_MSG(!err, err);
			err = script.GetField("Z", NULL, NULL, &z);
			ASSERT_MSG(!err, err);
			err = script.GetField("Power", NULL, NULL, &wp.Power);
			ASSERT_MSG(!err, err);

			// This one is optional, leave it at its default if not there
			err = script.GetField("TurnSpeedScaler", NULL, NULL, &wp.TurnSpeedScaler);

			wp.Position = FromLW_Pos(x,y,z);
			WayPoints.push_back(wp);
			script.Pop();
		}
		script.Pop();	// Pop the error one too we get off NOt getting an index
		script.Pop();   // This one is only for the WayPointLoop, only need to do it if there was one
	}

	// Or perhaps we are just following a ship
	err = script.GetFieldTable("FollowShip");
	if (!err) 
	{
		double x, y, z;
		err = script.GetField("X", NULL, NULL, &x);
		ASSERT_MSG(!err, err);
		err = script.GetField("Y", NULL, NULL, &y);
		ASSERT_MSG(!err, err);
		err = script.GetField("Z", NULL, NULL, &z);
		ASSERT_MSG(!err, err);

		ShipToFollow.RelPosition = FromLW_Pos(x,y,z);

		err = script.GetField("ShipID", &ShipToFollow.ShipID, NULL, NULL);
		ASSERT_MSG(!err, err);

		script.Pop();   // This one is only for the FollowShip, only need to do it if there was one
	}

	// Targeting another ship is optional
	TARGET_SHIP = "";
	script.GetField("TARGET_SHIP", &TARGET_SHIP, NULL, NULL);

	// Let the base class finish things up
	__super::LoadFromScript(script, controllingPlayerID, entityOwner);
}
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from a received Packet data
//! Apply this AFTER any packet identifiers
void ThrustShip2_TransmittedEntity::LoadFromBitStream(RakNet::BitStream& bs)
{
	__super::LoadFromBitStream(bs);

	NormalFlight_Characteristics.LoadFromBitStream(bs);
	Afterburner_Characteristics.LoadFromBitStream(bs);
	Braking_Characteristics.LoadFromBitStream(bs);
	
	bs.Read(dHeading);
	bs.Read(dPitch);
	bs.Read(dRoll);

	bs.Read(EngineDeceleration);
	bs.Read(EngineRampStrafe);
	bs.Read(EngineRampForward);
	bs.Read(EngineRampReverse);
	bs.Read(EngineRampAfterBurner);

	bs.Read(MAX_SPEED);
	bs.Read(ENGAGED_MAX_SPEED);
	bs.Read(ACCEL);
	bs.Read(BRAKE);
	bs.Read(STRAFE);
	bs.Read(AFTERBURNER_ACCEL);
	bs.Read(AFTERBURNER_BRAKE);

	bs.Read(CanUserPilot);
	bs.Read(StructuralDmgGLimit);
	bs.Read(PilotGLimit);
	bs.Read(PilotTimeToPassOut);
	bs.Read(PilotTimeToRecover);
	bs.Read(PilotMaxTimeToRecover);

	{
		unsigned numWayPoints;
		bs.Read(numWayPoints);
		for (unsigned i = 0; i < numWayPoints; ++i)
		{
			WayPoint wp;
			bs.Read(wp.Position[0]);
			bs.Read(wp.Position[1]);
			bs.Read(wp.Position[2]);
			bs.Read(wp.Power);
			bs.Read(wp.TurnSpeedScaler);
			WayPoints.push_back(wp);
		}
	}

	GG_Framework::Logic::Network::ReadString(bs, ShipToFollow.ShipID);
	bs.Read(ShipToFollow.RelPosition[0]);
	bs.Read(ShipToFollow.RelPosition[1]);
	bs.Read(ShipToFollow.RelPosition[2]);

	GG_Framework::Logic::Network::ReadString(bs, TARGET_SHIP);
	GG_Framework::Logic::Network::ReadString(bs, LEAD_RET_OSGV);
	GG_Framework::Logic::Network::ReadString(bs, INRANGE_LEAD_RET_OSGV);
	GG_Framework::Logic::Network::ReadString(bs, FWD_RET_OSGV);

	{
		unsigned numCannons;
		bs.Read(numCannons);
		for (unsigned i = 0; i < numCannons; ++i)
		{
			DEBUG_CANNON_FIRE("ThrustShip2_TransmittedEntity::LoadFromBitStream(%s) -- Cannon Loop\n", NAME.c_str());
			CannonDesc cannon;
			cannon.LoadFromBitStream(bs);
			Cannons.push_back(cannon);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

//! Write the entity into the BitStream to be sent, WITHOUT the header
void ThrustShip2_TransmittedEntity::WriteToBitStream(RakNet::BitStream& bs)
{
	__super::WriteToBitStream(bs);

	NormalFlight_Characteristics.WriteToBitStream(bs);
	Afterburner_Characteristics.WriteToBitStream(bs);
	Braking_Characteristics.WriteToBitStream(bs);
	
	bs.Write(dHeading);
	bs.Write(dPitch);
	bs.Write(dRoll);

	bs.Write(EngineDeceleration);
	bs.Write(EngineRampStrafe);
	bs.Write(EngineRampForward);
	bs.Write(EngineRampReverse);
	bs.Write(EngineRampAfterBurner);

	bs.Write(MAX_SPEED);
	bs.Write(ENGAGED_MAX_SPEED);
	bs.Write(ACCEL);
	bs.Write(BRAKE);
	bs.Write(STRAFE);
	bs.Write(AFTERBURNER_ACCEL);
	bs.Write(AFTERBURNER_BRAKE);

	bs.Write(CanUserPilot);
	bs.Write(StructuralDmgGLimit);
	bs.Write(PilotGLimit);
	bs.Write(PilotTimeToPassOut);
	bs.Write(PilotTimeToRecover);
	bs.Write(PilotMaxTimeToRecover);

	{
		unsigned numWayPoints = WayPoints.size();
		bs.Write(numWayPoints);
		for (unsigned i = 0; i < numWayPoints; ++i)
		{
			bs.Write(WayPoints[i].Position[0]);
			bs.Write(WayPoints[i].Position[1]);
			bs.Write(WayPoints[i].Position[2]);
			bs.Write(WayPoints[i].Power);
			bs.Write(WayPoints[i].TurnSpeedScaler);
		}
	}

	GG_Framework::Logic::Network::WriteString(bs, ShipToFollow.ShipID);
	bs.Write(ShipToFollow.RelPosition[0]);
	bs.Write(ShipToFollow.RelPosition[1]);
	bs.Write(ShipToFollow.RelPosition[2]);

	GG_Framework::Logic::Network::WriteString(bs, TARGET_SHIP);
	GG_Framework::Logic::Network::WriteString(bs, LEAD_RET_OSGV);
	GG_Framework::Logic::Network::WriteString(bs, INRANGE_LEAD_RET_OSGV);
	GG_Framework::Logic::Network::WriteString(bs, FWD_RET_OSGV);

	{
		unsigned numCannons = Cannons.size();
		bs.Write(numCannons);
		for (unsigned i = 0; i < numCannons; ++i)
		{
			Cannons[i].WriteToBitStream(bs);
		}
	}
}
//////////////////////////////////////////////////////////////////////////