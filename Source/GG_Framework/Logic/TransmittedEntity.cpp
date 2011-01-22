// GG_Framework.Logic TransmittedEntity.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"

using namespace GG_Framework::Logic::Network;
using namespace GG_Framework::Logic;

FactoryMapT<TransmittedEntity> TransmittedEntity::ClassFactory;

// The base Entity3D will use the base TransmittedEntity
ClassFactoryT<TransmittedEntity, TransmittedEntity> TransmittedEntity_ClassFactory("Entity3D", TransmittedEntity::ClassFactory);

TransmittedEntity::TransmittedEntity()
{
	Mass = 0.0;
	Dimensions[0] = 0.0; Dimensions[1] = 0.0; Dimensions[2] = 0.0;
	X= Y= Z= Heading= Pitch= Roll = 0.0;
}

TransmittedEntity* TransmittedEntity::CreateFromServerScript(
	GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, ITransmittedEntityOwner* entityOwner)
{
	std::string CPP_CLASS;
	const char* err;
	err = script.GetFieldTable("Entity");
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("CPP_CLASS", &CPP_CLASS);
		ASSERT_MSG(!err, err);
	}
	script.Pop();

	TransmittedEntity* ret = ClassFactory.Create(CPP_CLASS);
	ASSERT_MSG(ret, GG_Framework::Base::BuildString("Could not build TransmittedEntity for type {%s}", CPP_CLASS.c_str()).c_str());
	ret->CPP_CLASS = CPP_CLASS;
	ret->LoadFromScript(script, controllingPlayerID, entityOwner);

	return ret;
}
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from the LUA Script
void TransmittedEntity::LoadFromScript(
	GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, ITransmittedEntityOwner* entityOwner)
{
	const char* err;

	// Not worried about these failing, we have good defaults
	X = Y = Z = Heading = Pitch = Roll = Mass = 0;
	COLLISION_INDEX = 0;

	// We will need the node name
	err = script.GetField("ID", &NAME);
	ASSERT_MSG(!err, err);

	err = script.GetFieldTable("Entity");
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("OSGV", &OSGV);
		ASSERT_MSG(!err, err);

		err = script.GetField("Mass", NULL, NULL, &Mass);
		ASSERT_MSG(!err, err);

		//Get the ship dimensions
		err = script.GetFieldTable("Dimensions");
		if (!err)
		{
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("Length", NULL, NULL,&Dimensions[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("Width", NULL, NULL,&Dimensions[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("Height", NULL, NULL,&Dimensions[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		else
			Dimensions[0]=Dimensions[1]=Dimensions[2]=2.0; //using 2.0 here means 1.0 radius default which is a great default for torque radius of mass

		double d;
		err = script.GetField("COLLISION_INDEX", NULL, NULL, &d);
		ASSERT_MSG(!err, err);
		COLLISION_INDEX = d;
	}
	script.Pop();

	script.GetField("X", NULL, NULL, &X);
	script.GetField("Y", NULL, NULL, &Y);
	script.GetField("Z", NULL, NULL, &Z);
	script.GetField("Heading", NULL, NULL, &Heading);
	script.GetField("Pitch", NULL, NULL, &Pitch);
	script.GetField("Roll", NULL, NULL, &Roll);

	CONTROLLING_PLAYER_ID = controllingPlayerID;
	NETWORK_ID = entityOwner->AddTransmittedEntity(this);
}
//////////////////////////////////////////////////////////////////////////

TransmittedEntity* TransmittedEntity::CreateFromBitStream(RakNet::BitStream& bs)
{
	std::string CPP_CLASS;
	ReadString(bs, CPP_CLASS);

	TransmittedEntity* ret = ClassFactory.Create(CPP_CLASS);
	ret->CPP_CLASS = CPP_CLASS;
	ret->LoadFromBitStream(bs);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from a received Packet data
//! Apply this AFTER any packet identifiers
void TransmittedEntity::LoadFromBitStream(RakNet::BitStream& bs)
{
	// ReadIn each of the strings as NULL terminated
	ReadString(bs, NAME);
	ReadString(bs, OSGV);

	bs.Read(Mass);

	bs.Read(Dimensions[0]);
	bs.Read(Dimensions[1]);
	bs.Read(Dimensions[2]);

	bs.Read(X);
	bs.Read(Y);
	bs.Read(Z);
	bs.Read(Heading);
	bs.Read(Pitch);
	bs.Read(Roll);
	bs.Read(COLLISION_INDEX);
	bs.Read(CONTROLLING_PLAYER_ID);
	bs.Read(NETWORK_ID);
}
//////////////////////////////////////////////////////////////////////////

//! Write the entity into the BitStream to be sent, WITHOUT the header
void TransmittedEntity::WriteToBitStream(RakNet::BitStream& bs)
{
	// Write out each of the strings as NULL terminated
	// The CPP_CLASS is written here, but is read from CreateFromBitStream()
	WriteString(bs, CPP_CLASS);
	WriteString(bs, NAME);
	WriteString(bs, OSGV);

	bs.Write(Mass);

	bs.Write(Dimensions[0]);
	bs.Write(Dimensions[1]);
	bs.Write(Dimensions[2]);

	bs.Write(X);
	bs.Write(Y);
	bs.Write(Z);
	bs.Write(Heading);
	bs.Write(Pitch);
	bs.Write(Roll);
	bs.Write(COLLISION_INDEX);
	bs.Write(CONTROLLING_PLAYER_ID);
	bs.Write(NETWORK_ID);
}
//////////////////////////////////////////////////////////////////////////
