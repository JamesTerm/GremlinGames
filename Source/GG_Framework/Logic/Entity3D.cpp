// GG_Framewor.Logic Entity3D.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

// **** These macros are all about collision detection and external forces ****
// I tweaked with collision points recently, using this means that only the center point is observed
// #define IGNORE_COLLISION_POINTS

using namespace GG_Framework::Logic;

FactoryMapT<Entity3D> Entity3D::ClassFactory;
ClassFactoryT<Entity3D, Entity3D> Entity3D_ClassFactory("Entity3D", Entity3D::ClassFactory);

Entity3D::Entity3D() : m_Physics(m_att_quat), m_isShowing(true), m_showNextFrame(true), m_collisionIndex(0), m_RC_Controller(NULL),
	m_pos_m(0,0,0), m_att_quat(0,0,0,1), m_lastTime_s(-1.0), m_collidedLastFrame(NULL), m_isPlayerControlled(-1)
{
}

Entity3D::~Entity3D()
{
	if (m_RC_Controller)
		delete m_RC_Controller;
}

bool Entity3D::IsLocallyControlled() const 
{
	return m_controllingPlayerID==m_gameClient->GetMyPlayerID();
}

RC_Controller& Entity3D::GetRC_Controller()
{
	if (!m_RC_Controller)
		m_RC_Controller = new RC_Controller(this);
	return *m_RC_Controller;
}

void Entity3D::SetPlayerControlled(bool controlled)
{
	int C = controlled ? 1 : 0;
	if (C != m_isPlayerControlled)
	{
		GetEventMap()->EventOnOff_Map["PLAYER_CONTROLLED"].Fire(controlled);
		m_isPlayerControlled = C;
		GetRC_Controller().ParentPlayerControlled(controlled);
	}
}


double Entity3D::GetBoundRadius()
{
	osg::Group* actor = Get_BB_Actor();
	if (!actor) return 0.0;
	if (actor->getNumChildren() < 1)
		return 0.0;
	osg::Node* child = actor->getChild(0);	// Only the first one, HUD elements might be attached also
	if (!child) return 0.0;
	return child->getBound().radius();
}
//////////////////////////////////////////////////////////////////////////

double Entity3D::GetBoundRadius2()
{
	osg::Group* actor = Get_BB_Actor();
	if (!actor) return 0.0;
	if (actor->getNumChildren() < 1)
		return 0.0;
	osg::Node* child = actor->getChild(0);	// Only the first one, HUD elements might be attached also
	if (!child) 
		return 0.0;
	return child->getBound().radius2();
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::Initialize(GameClient& gameClient, Entity3D::EventMap& em, TransmittedEntity& te)
{
	m_gameClient = &gameClient;
	m_eventMap = &em;
	m_name = te.NAME;
	m_className = te.CPP_CLASS;
	m_collisionIndex = te.COLLISION_INDEX;
	m_controllingPlayerID = te.CONTROLLING_PLAYER_ID;
	m_networkID = te.NETWORK_ID;

	SetPosAtt(FromLW_Pos(te.X, te.Y, te.Z), FromLW_Rot(te.Heading, te.Pitch, te.Roll));
	m_origPos = GetPos_m();
	m_origAtt = osg::Vec3d(te.Heading, te.Pitch, te.Roll);

	// All of my descendants can see the timer updates
	m_gameClient->GetLogicTimer().CurrTimeChanged.Subscribe(ehl, *this, &Entity3D::GameTimerUpdate);

	// All entities have mass
	m_Physics.SetMass(te.Mass);

	for (size_t i=0;i<3;i++)
	{
		Dimensions[i] = te.Dimensions[i];
		if (Dimensions[i] == 0.0)
			Dimensions[i] = 1.0f;	// I do not like doing it here, but not all entities define this
	}

	//Given the dimensions we can compute the radius of concentrated mass where each radius will average as follows
	{
		//Yaw = l and w
		//Pitch = l and h
		//Roll = w and h

		double l=Dimensions[1]/2.0;
		double w=Dimensions[0]/2.0;
		double h=Dimensions[2]/2.0;

		osg::Vec3d RadiusOfConcentratedMass;
		RadiusOfConcentratedMass[0]= sqrt(l*l + w*w);
		RadiusOfConcentratedMass[1]= sqrt(l*l + h*h);
		RadiusOfConcentratedMass[2]= sqrt(w*w + h*h);
		m_Physics.SetRadiusOfConcentratedMass(RadiusOfConcentratedMass);
	}
	//This must be called after setting up the SetRadiusOfConcentratedMass! (and visibility)
	GetRC_Controller().Initialize(gameClient,te);
	// Listen for the Script telling us to hide (default does nothing)
	em.Event_Map["HIDE"].Subscribe(ehl, *this, &Entity3D::OnOSGV_HIDE);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::SetPosAtt(const osg::Vec3d pos_m, const osg::Quat att)
{

	m_pos_m = pos_m;
	m_att_quat = att;
	PosQuat_Changed.Fire(m_pos_m, m_att_quat);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::SetActor(GG_Framework::UI::ActorTransform* ap, bool bbox)
{
	SetActor(	ap, 
				(bbox? m_BB_Actor : m_UI_Actor), 
				(bbox ? m_gameClient->Get_BB_ActorScene() : m_gameClient->Get_UI_ActorScene()));
#ifndef IGNORE_COLLISION_POINTS
	if (bbox && (GetCollisionIndex()>0))
	{
		// Find all of the nodes representing the different collision points
		int index = 1;
		while(true)
		{
			const char* filter = (index < 10) ? "ColPt_0%i" : "ColPt_%i";
			std::string colPtName = GG_Framework::Base::BuildString(filter, index);
			osg::Node* colPt = GG_Framework::UI::OSG::FindChildNode(ap, colPtName);
			if (colPt)
				m_collisionPoints.push_back(colPt);
			else
				break;
			++index;
		}
	}
#endif
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::Show(bool show)
{
	if (show != m_isShowing)
	{
		m_isShowing = show;
		if (m_isShowing)
		{
			if (m_BB_Actor.valid() && m_gameClient->Get_BB_ActorScene())
				m_gameClient->Get_BB_ActorScene()->AddActor(m_BB_Actor.get());
			if (m_UI_Actor.valid() && m_gameClient->Get_UI_ActorScene())
				m_gameClient->Get_UI_ActorScene()->AddActor(m_UI_Actor.get());
		}
		else
		{
			CancelAllControls();
			if (m_BB_Actor.valid() && m_gameClient->Get_BB_ActorScene())
				m_gameClient->Get_BB_ActorScene()->RemoveActor(m_BB_Actor.get());
			if (m_UI_Actor.valid() && m_gameClient->Get_UI_ActorScene())
				m_gameClient->Get_UI_ActorScene()->RemoveActor(m_UI_Actor.get());
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::OnOSGV_HIDE()
{
	bool allowHide = AllowScriptHide();
	DEBUG_DESTRUCTION("Entity3D::OnOSGV_HIDE(%s)\n", allowHide ? "true" : "false");
	if (allowHide && IsLocallyControlled()) 
		ShowNextFrame(false);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::SetActor(GG_Framework::UI::ActorTransform* ap, 
			  osg::ref_ptr<GG_Framework::UI::ActorTransform>& refPtr, GG_Framework::UI::ActorScene* scene)
{
	// Remove the old one if it is there
	if (refPtr.valid())
	{
		if (scene && m_isShowing)
			scene->RemoveActor(ap);
		refPtr->UnsubscribeToPosAttChange(PosQuat_Changed);
	}

	// Here is the new one
	refPtr = ap;
	if (scene && m_isShowing)
		scene->AddActor(ap);
	ap->setName(m_name.c_str());
	ap->SubscribeToPosAttChange(PosQuat_Changed);
	PosQuat_Changed.Fire(m_pos_m, m_att_quat);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::GameTimerUpdate(double time_s)
{
	// Take care of visibility for this frame
	if (IsLocallyControlled())
		m_RC_Controller->ShowParent(m_showNextFrame);
	else
		m_showNextFrame = IsShowing();

	// Use my current Velocity to update my positions
	if (m_lastTime_s < 0.0)
		m_lastTime_s = time_s;
	else if (time_s > m_lastTime_s)
	{
		double dTime_s = time_s - m_lastTime_s;
		m_lastTime_s = time_s;
		TimeChange(dTime_s);
	}
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::TimeChange(double dTime_s)
{
	GetRC_Controller().TimeChange_pre(dTime_s);

	//Run physics update for displacement
	osg::Vec3d PositionDisplacement;
	osg::Vec3d RotationDisplacement;

	// Now to apply the displacement
	m_Physics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
	TestPosAtt_Delta(PositionDisplacement,RotationDisplacement,dTime_s);

	SetPosAtt(
		GetPos_m()+(PositionDisplacement),
		FromLW_Rot_Radians(RotationDisplacement[0],RotationDisplacement[1],RotationDisplacement[2]) * GetAtt_quat()
		);
	
	GetRC_Controller().TimeChange_post(dTime_s);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::ResetPos()
{
	CancelAllControls();
	m_Physics.ResetVectors();
	SetPosAtt(m_origPos, FromLW_Rot(m_origAtt[0], m_origAtt[1], m_origAtt[2]));
	GetEventMap()->Event_Map["ResetPos"].Fire();
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner)
{
	// We should still be owning this
	ASSERT(!sameOwner || IsLocallyControlled());

	// Write whether or not I am visible
	if (sameOwner)
		sendControlBS.Write(IsShowing());

	// The RC Controller can do the rest
	GetRC_Controller().SendFinalUpdate(sendControlBS, sameOwner);

	// Finally set who is really controlling, this ALWAYS happens
	SetControllingPlayerID(playerThatWantsControl);
}
//////////////////////////////////////////////////////////////////////////

void Entity3D::RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner)
{
	bool isShowing;
	if (sameOwner)
		recvControlBS.Read(isShowing);

	// The RC Controller can do the rest
	GetRC_Controller().RecvFinalUpdate(recvControlBS, msgTime_s, sameOwner);
		
	if (sameOwner)
		GetRC_Controller().ShowParent(isShowing);

	// Finally set who is really controlling, ALWAYS happens
	SetControllingPlayerID(playerThatWantsControl);
}
//////////////////////////////////////////////////////////////////////////

