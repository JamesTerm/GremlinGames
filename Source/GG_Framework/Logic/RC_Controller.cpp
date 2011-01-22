#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;

//This is for testing purposes.  This is used to confirm how accurate the receiver can interpolate the messages properly
enum SimulationType
{
	eNoSimulation,
	eTest_gp_NoRackNet,
	eTest_gp_RackNet
};

const SimulationType g_RunSimulation=eNoSimulation;
const char *g_GinnyPig="Vexhall";
//const char *g_GinnyPig="Ramora_1";
//const char *g_GinnyPig="Ramora";

#undef __UsePosATT_Only__
#define __USE_PREDICTION_
#undef SEND_EVERY_FRAME

inline const osg::Vec3d Vec3Multiply (const osg::Vec3d &A,const osg::Vec3d &rhs)
{
	return osg::Vec3d(A[0]*rhs._v[0], A[1]*rhs._v[1], A[2]*rhs._v[2]);
}

RC_Controller::SimulateInfo RC_Controller::s_SimulatedTransfer;

RC_Controller::RC_Controller(Entity3D *Parent) : 
	m_pParent(Parent), m_lastTransmittedUpdateTime_s(-1.0), m_resetVis(true), m_ignorePhysics(false)
{
	if (g_RunSimulation==eTest_gp_NoRackNet)
		m_lastTransmittedUpdateTime_s = 0.0;
}

RC_Controller::~RC_Controller()
{
}

void RC_Controller::SetRecvIgnorePhysics(bool ignorePhysics)
{
	m_ignorePhysics = ignorePhysics;
	if (m_ignorePhysics) m_resetVis = !RC_Controller_GetIsSending();
}

void RC_Controller::Initialize( GameClient& gs, TransmittedEntity& te)
{
	Physics::FlightDynamics &Physics=m_pParent->GetPhysics();
	osg::Vec3d RadiusOfConcentratedMass=Physics.GetRadiusOfConcentratedMass();
	m_RadialArmDefault=Vec3Multiply(RadiusOfConcentratedMass,RadiusOfConcentratedMass);

	// The first pass we set position, we may need to reset the visibility
	// If the parent starts off hiding, this will happen later
	if (RC_Controller_GetIsSending())
		m_resetVis = m_pParent->IsShowing();
}

void RC_Controller::SendFinalUpdate(RakNet::BitStream& sendControlBS, bool sameOwner)
{
	// We should still be sending
	ASSERT(!sameOwner || RC_Controller_GetIsSending());

	// Send a regular update
	if (sameOwner)
		PopulateStandardEntityUpdate(sendControlBS);

	// reset for next update we get
	m_resetVis = true;
}
//////////////////////////////////////////////////////////////////////////

void RC_Controller::RecvFinalUpdate(RakNet::BitStream& recvControlBS, double msgTime_s, bool sameOwner)
{
	// This is the last update we will get from the other controller
	// we will start sending from now on if we are the controller or recv from another
	ASSERT(!RC_Controller_GetIsSending());
	
	if (sameOwner)
		ReadStandardEntityUpdate(recvControlBS, msgTime_s);
	m_resetVis = true;
}
//////////////////////////////////////////////////////////////////////////

bool RC_Controller::RC_Controller_GetIsSending() const
{
	if (g_RunSimulation!=eTest_gp_NoRackNet)
	{
		// We are sending messages out if the entity is locally controlled
		return m_pParent->IsLocallyControlled();
	}
	//Hard wire some ship to be my ginny pig
	if (stricmp(m_pParent->GetName().c_str(),g_GinnyPig)==0)
		return false;
	//DebugOutput("ID=%s\n",m_pParent->GetName().c_str());
	return true;
}

void Prediction(const RC_Controller::PosAttUpdate &_, double lag_s, 
				osg::Vec3d& PredictPOS,	osg::Vec3d& PredictLinearVel, osg::Vec3d& PredictAngularVel, osg::Quat& PredictATT)
{
	double dT_s= (lag_s<0.1) ? lag_s : _.DeltaTime_s;
	PredictPOS=_.POS + (_.LinearVelocity * lag_s) + ( _.LinearAcceleration * (0.5 * dT_s * dT_s));
	osg::Vec3d ATTDelta( (_.AngularVelocity * lag_s)+ (_.AngularAcceleration * (0.5 * dT_s * dT_s)));
	PredictATT=FromLW_Rot_Radians(ATTDelta[0],ATTDelta[1],ATTDelta[2]) * _.ATT;
	PredictLinearVel=_.LinearVelocity + (_.LinearAcceleration * dT_s);
	PredictAngularVel=_.AngularVelocity + (_.AngularAcceleration * dT_s);
}

//#define DOUT4(x,...) DebugOutput(x,__VA_ARGS__)
void RC_Controller::TimeChange_pre(double dTime_s)
{
	if (g_RunSimulation==eTest_gp_NoRackNet)
		m_lastTransmittedUpdateTime_s=s_SimulatedTransfer.lastUpdateTime;
	// Are we receiving?
	if (!RC_Controller_GetIsSending() && (m_lastTransmittedUpdateTime_s>-1.0))
	{
		double currTime_s = m_pParent->GetGameClient()->GetTimer().GetCurrTime_s();
		double timeSinceUpdate_s = currTime_s - m_lastTransmittedUpdateTime_s;

		// Watch for no updates being sent yet
		if (timeSinceUpdate_s >= 0.0)
		{
			if (g_RunSimulation==eTest_gp_NoRackNet)
				m_NetworkPosAtt=s_SimulatedTransfer.posatt;

			// Perform extrapolation based on the difference in time here
			osg::Vec3d PredictPOS=m_NetworkPosAtt.POS;
			osg::Vec3d PredictLinearVel=m_NetworkPosAtt.LinearVelocity;
			osg::Vec3d PredictAngularVel=m_NetworkPosAtt.AngularVelocity;
			osg::Quat PredictATT=m_NetworkPosAtt.ATT;

			m_PredictedPosAtt=m_NetworkPosAtt;

#ifdef __USE_PREDICTION_
			//from past to present
			Prediction(m_NetworkPosAtt, timeSinceUpdate_s, PredictPOS, PredictLinearVel, PredictAngularVel, PredictATT);
			//from present to future
			m_PredictedPosAtt.POS=PredictPOS;
			m_PredictedPosAtt.LinearVelocity=PredictLinearVel;
			m_PredictedPosAtt.AngularVelocity=PredictAngularVel;
			m_PredictedPosAtt.ATT=PredictATT;
			Prediction(m_PredictedPosAtt, dTime_s, PredictPOS, PredictLinearVel, PredictAngularVel, PredictATT);

			//Testing to see how prediction is working
			//m_pParent->SetPosAtt(PredictPOS, PredictATT);
			//return;
#endif

			// if we are about to reset vis anyway, no need to even apply physics
			if (!m_resetVis)
			{
				osg::Vec3d GlobalForce,TorqueToApply;
				Physics::FlightDynamics &Physics=m_pParent->GetPhysics();
				{
					//It seems the lower the max force the smoother it will be... may need trial and error to find the ideal value
					const double MaxForce=Physics.GetMass() * 10.0;
					const osg::Vec3d Restraint(MaxForce,MaxForce,MaxForce);
					osg::Vec3d VectorOffset=PredictPOS-m_pParent->GetPos_m();
					osg::Vec3d LocalVectorOffset(m_pParent->GetAtt_quat().conj() * VectorOffset);
					osg::Vec3d LocalPredictLinearVel(m_pParent->GetAtt_quat().conj() * PredictLinearVel);
					osg::Vec3d LocalVelocity=Physics.GetVelocityFromDistance_Linear_v1(LocalVectorOffset,Restraint,Restraint,dTime_s,LocalPredictLinearVel);
					//DebugOutput("%f\n",LocalVelocity[1]);
					//LocalVelocity is the velocity vector in line with the entity
					//GlobalVelocity rotates that back out using the entity's current attitude.
					osg::Vec3d GlobalVelocity(m_pParent->GetAtt_quat() * LocalVelocity);
					//explicitly call PhysicsEntity method as some entity's are using the flight dynamics with ramping (we do not use that here)
					GlobalForce=Physics.PhysicsEntity::GetForceFromVelocity(GlobalVelocity,dTime_s);
					//DebugOutput("Acc=%f\n",(GlobalForce[1]/Physics.GetMass())/dTime_s);
					//Should not need to apply restraint force here since it was precomputed (don't want any surprises)

				}
				{
					//This will extract the yaw pitch and roll against 2 quats
					osg::Vec3d Offset=-Physics.ComputeAngularDistance(PredictATT);
					//DOUT4("%f %f %f\n",Offset[0],Offset[1],Offset[2]);
					const double MaxTorque=Physics.GetMass() * 1.0;  
					//const double MaxTorque=-1.0;
					const osg::Vec3d TorqueRestraint(MaxTorque,MaxTorque,MaxTorque);
					osg::Vec3d angularvelocity(Physics.GetVelocityFromDistance_Angular_v2(Offset,TorqueRestraint,dTime_s,PredictAngularVel));
					//DOUT4("%f %f %f",angularvelocity[0],angularvelocity[1],angularvelocity[2]);
					//DOUT4("%f %f",Offset[0],angularvelocity[0]);
					TorqueToApply=Physics.PhysicsEntity::GetTorqueFromVelocity(angularvelocity,dTime_s);
					//Should not need to apply restraint torque here since it was precomputed (don't want any surprises)
				}
				Physics.ApplyFractionalForce(GlobalForce,dTime_s);
				Physics.ApplyFractionalTorque(TorqueToApply,dTime_s,m_RadialArmDefault);
			}
		}
	}
}

void RC_Controller::TimeChange_post(double dTime_s)
{
	const double currTime_s = m_pParent->GetGameClient()->GetTimer().GetCurrTime_s();
	double timeSinceUpdate_s = currTime_s - m_lastTransmittedUpdateTime_s;

	// This block sets the position directly, circumventing all physics.  Normally, the message is acted on
	// in the TimeChange_pre function, but if we want to just set pos att directly, either because of our macro
	// or if m_resetVis is true, or because the physics generated position is too far off, then we should do it here
	if (!RC_Controller_GetIsSending() && (m_lastTransmittedUpdateTime_s>-1.0))
	{
		if (!m_resetVis)
		{
			// See if the physics generated position for this point is too far from the predicted position
			osg::Vec3d physPos = m_pParent->GetPos_m();

			// How far off do we want to allow?  This does it at one second.
			double linVelDist2 = m_pParent->GetPhysics().GetLinearVelocity().length2();

			// Reset the vis if we are too far away
			m_resetVis = ((m_PredictedPosAtt.POS-physPos).length2() > linVelDist2);
		}

		// If we are wanting to reset or if we are too far away from where we want to be ...
		if (m_resetVis)
		{
			// Watch for no updates being sent yet
			if (timeSinceUpdate_s >= 0.0)
			{
				//if (stricmp(m_pParent->GetName().c_str(),g_GinnyPig)==0)
				//	DOUT2("%f %f %f",PredictPOS[0],PredictPOS[1],PredictPOS[2]);

				m_pParent->SetPosAtt(m_PredictedPosAtt.POS, m_PredictedPosAtt.ATT);

				m_pParent->GetPhysics().SetLinearVelocity(m_PredictedPosAtt.LinearVelocity);
				m_pParent->GetPhysics().SetAngularVelocity(m_PredictedPosAtt.AngularVelocity);

#ifndef __UsePosATT_Only__ // Always leave m_resetVis true if using __UsePosATT_Only__
				m_resetVis = m_ignorePhysics;	// If we are ignoring physics, we are ALWAYS resetting vis
#endif
			}
		}
	}
	
	#if 0
	if (stricmp(m_pParent->GetName().c_str(),g_GinnyPig)==0)
	{
		osg::Vec3d pos=m_pParent->GetPos_m();
		DOUT2("%f %f %f",pos[0],pos[1],pos[2]);
	}
	#endif

	// Here is where we post messages back to the server about where we are now.
	// We do not need to send if the entity is not visible
	// TODO: This will send every frame, we may want to place other limits
	if (RC_Controller_GetIsSending() && m_pParent->IsShowing())
	{
		osg::Vec3d pos = m_pParent->GetPos_m();
		osg::Quat att = m_pParent->GetAtt_quat();

		// We do not need to send if there was no change in position
#ifndef SEND_EVERY_FRAME
		if ( m_resetVis ||
			(m_lastTransmittedUpdateTime_s <= 0.0) ||				// If it has never been sent OR
			(timeSinceUpdate_s > 1.0) ||	// At least every second  OR
			((pos-m_NetworkPosAtt.POS).length2() > 0.01) || // Position has changed OR
			((att-m_NetworkPosAtt.ATT).length2() > 0.0001))	// Attitude has changed
#endif
		{
			// Populate the m_NetworkPosAtt
			Physics::FlightDynamics &Physics=m_pParent->GetPhysics();
			m_NetworkPosAtt.DeltaTime_s=dTime_s;
			assert((dTime_s >= 0.0) && (dTime_s < 100));
			m_NetworkPosAtt.POS = pos;
			m_NetworkPosAtt.ATT = att;
			osg::Vec3d Current_LinearVelocity=Physics.GetLinearVelocity();
			//Note acceleration times represent delta velocity for delta time of last update
			m_NetworkPosAtt.LinearAcceleration=Current_LinearVelocity-m_NetworkPosAtt.LinearVelocity;
			m_NetworkPosAtt.LinearVelocity= Current_LinearVelocity;
			osg::Vec3d Current_AngularVelocity=Physics.GetAngularVelocity();
			m_NetworkPosAtt.AngularAcceleration=Current_AngularVelocity-m_NetworkPosAtt.AngularVelocity;
			m_NetworkPosAtt.AngularVelocity=Current_AngularVelocity;

			// We will want to post the next time we are asked
			m_wantsEntityUpdate = true;
			m_resetVis = m_ignorePhysics;

			if (g_RunSimulation!=eNoSimulation)
			{
				if ((strcmp(m_pParent->GetName().c_str(),"Q33_2")==0)&&(m_pParent->IsLocallyControlled()))
				{
#if 0
					static size_t SimulateLag=0;
					const size_t LagThreshold=30;
					if (SimulateLag++>LagThreshold)
					{
						SimulateLag=0;
						s_SimulatedTransfer=m_PreviousSimulatedValue;
						m_PreviousSimulatedValue.posatt=m_lastPosAtt;
						m_PreviousSimulatedValue.lastUpdateTime=currTime_s;
					}
#else
					//for testing ginny pig follow with racknet we set it here as before... and simply transfer these updates when
					//calling PopulateStandardEntityUpdate
					s_SimulatedTransfer.posatt=m_NetworkPosAtt;
					s_SimulatedTransfer.lastUpdateTime=currTime_s;
#endif
					//DebugOutput("%p=%f %f %f\n",this,m_NetworkPosAtt.POS[0],m_NetworkPosAtt.POS[1],m_NetworkPosAtt.POS[2]);
				}
			}
		}
	}
}

void RC_Controller::ResetLastPosAtt()
{
	// Populate the m_NetworkPosAtt
	osg::Vec3d zero;
	m_NetworkPosAtt.DeltaTime_s=0.0;
	m_NetworkPosAtt.POS = m_pParent->GetPos_m();
	m_NetworkPosAtt.ATT = m_pParent->GetAtt_quat();
	m_NetworkPosAtt.LinearAcceleration=zero;
	m_NetworkPosAtt.LinearVelocity= m_pParent->GetPhysics().GetLinearVelocity();
	m_NetworkPosAtt.AngularAcceleration=zero;
	m_NetworkPosAtt.AngularVelocity=m_pParent->GetPhysics().GetAngularVelocity();
	m_wantsEntityUpdate = true;
	m_resetVis = true;
}

void RC_Controller::ShowParent(bool showing)
{
	if (m_pParent->IsShowing() != showing)
	{
		if (showing)
			m_resetVis = true;
		else
		{
			m_wantsEntityUpdate = false;
			m_lastTransmittedUpdateTime_s = -1.0;	// We do not want to handle messages until we get a good update
		}
		m_pParent->Show(showing);
	}
}

void RC_Controller::PopulateStandardEntityUpdate(RakNet::BitStream& entityUpdateBS)
{
	if (g_RunSimulation==eTest_gp_RackNet)
	{
		if (stricmp(m_pParent->GetName().c_str(),g_GinnyPig)==0)
			m_NetworkPosAtt=s_SimulatedTransfer.posatt;
	}

	// Write our update
	m_NetworkPosAtt.WriteToBS(entityUpdateBS);

	// We have said all we need to say for now
	m_wantsEntityUpdate = false;
}

void RC_Controller::ReadStandardEntityUpdate(RakNet::BitStream& entityUpdateBS, double msgTime_s)
{
	m_lastTransmittedUpdateTime_s = msgTime_s;
	//DebugOutput("u time=%f",m_lastTransmittedUpdateTime_s);
	if (g_RunSimulation!=eTest_gp_NoRackNet)
		m_NetworkPosAtt.ReadFromBS(entityUpdateBS);
	else
	{
		// Read it into a stub, this only happens when James is running his simulation
		PosAttUpdate temp;
		temp.ReadFromBS(entityUpdateBS);
	}
}

void RC_Controller::ParentPlayerControlled(bool c)
{
	// Send the message to the network when the visibility changes
	if (RC_Controller_GetIsSending())
	{
		// Post the message with our new visibility
		RakNet::BitStream visBS;
		m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
			visBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_CriticalEntityUpdates, m_pParent->GetNetworkID(), 
			c ? ENTITY_PLAYER_CONTROLLED : ENTITY_NONPLAYER_CONTROLLED);

		// Send the message, in order, and make sure it gets there, on OC_CriticalEntityUpdates channel
		m_pParent->GetGameClient()->PostEntityUpdateToServer(visBS);
	}
}

void RC_Controller::FireNetwork_Collision(Entity3D& otherEntity, const osg::Vec3d& localCollPt, double impulseTime)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents, m_pParent->GetNetworkID(), ENTITY_COLLISION);

	// Populate the message
	bs.Write(otherEntity.GetNetworkID());
	bs.Write(localCollPt[0]); bs.Write(localCollPt[1]); bs.Write(localCollPt[2]);
	bs.Write(impulseTime);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the event
	m_pParent->GetEventMap()->Collision.Fire(otherEntity, localCollPt, impulseTime);
}

void RC_Controller::RecvNetwork_Collision(RakNet::BitStream& bs, double msgTime_s)
{
	unsigned netID;
	osg::Vec3d localCollPt;
	double impulseTime;

	// Read the message
	bs.Read(netID); Entity3D* otherEntity = m_pParent->GetGameClient()->FindEntity(netID);
	bs.Read(localCollPt[0]); bs.Read(localCollPt[1]); bs.Read(localCollPt[2]);
	bs.Read(impulseTime);

	// Fire the event
	if (otherEntity)
	{
		m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
		m_pParent->GetEventMap()->Collision.Fire(*otherEntity, localCollPt, impulseTime);
		m_pParent->GetEventMap()->AlternateEventTime = NULL;
	}
}

void RC_Controller::FireNetwork_ExternalForce(const osg::Vec3d &force, const osg::Vec3d &point,double FrameDuration )
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents, m_pParent->GetNetworkID(), ENTITY_ExternalForce);

	// Populate the message
	bs.Write(force[0]); bs.Write(force[1]); bs.Write(force[2]);
	bs.Write(point[0]); bs.Write(point[1]); bs.Write(point[2]);
	bs.Write(FrameDuration);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Only move the ship if locally controlled, the effects of the move are propagated
	if (m_pParent->IsLocallyControlled())
	{
		m_pParent->GetPhysics().ApplyFractionalForce(force, point, FrameDuration);
	}
}

void RC_Controller::RecvNetwork_ExternalForce(RakNet::BitStream& bs, double msgTime_s)
{
	osg::Vec3d force;
	osg::Vec3d point;
	double FrameDuration;

	// Read the message
	bs.Read(force[0]); bs.Read(force[1]); bs.Read(force[2]);
	bs.Read(point[0]); bs.Read(point[1]); bs.Read(point[2]);
	bs.Read(FrameDuration);

	// Only move the ship if locally controlled, the effects of the move are propagated
	if (m_pParent->IsLocallyControlled())
	{
		m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
		m_pParent->GetPhysics().ApplyFractionalForce(force, point, FrameDuration);
		m_pParent->GetEventMap()->AlternateEventTime = NULL;
	}
}

void RC_Controller::FireNetwork_Event(std::string msgID, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_Event);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->Event_Map[msgID].Fire();
}

void RC_Controller::RecvNetwork_Event(RakNet::BitStream& bs, double msgTime_s)
{
	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->Event_Map[msgID].Fire();
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

void RC_Controller::FireNetwork_EventOnOff(std::string msgID, bool onOff, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_EventOnOff);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);
	bs.Write(onOff);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->EventOnOff_Map[msgID].Fire(onOff);
}

void RC_Controller::RecvNetwork_EventOnOff(RakNet::BitStream& bs, double msgTime_s)
{
	bool onOff;

	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;
	bs.Read(onOff);

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->EventOnOff_Map[msgID].Fire(onOff);
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

void RC_Controller::FireNetwork_EventValue(std::string msgID, double val, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_EventValue);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);
	bs.Write(val);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->EventValue_Map[msgID].Fire(val);
}

void RC_Controller::RecvNetwork_EventValue(RakNet::BitStream& bs, double msgTime_s)
{
	double val;

	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;
	bs.Read(val);

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->EventValue_Map[msgID].Fire(val);
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

void RC_Controller::FireNetwork_EventPt(std::string msgID, const osg::Vec3d& pt, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_EventPt);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);
	bs.Write(pt[0]);bs.Write(pt[1]);bs.Write(pt[2]);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->EventPt_Map[msgID].Fire(pt);
}

void RC_Controller::RecvNetwork_EventPt(RakNet::BitStream& bs, double msgTime_s)
{
	osg::Vec3d pt;

	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;
	bs.Read(pt[0]);bs.Read(pt[1]);bs.Read(pt[2]);

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->EventPt_Map[msgID].Fire(pt);
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

void RC_Controller::FireNetwork_EventPtOnOff(std::string msgID, const osg::Vec3d& pt, bool onOff, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_EventPtOnOff);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);
	bs.Write(pt[0]);bs.Write(pt[1]);bs.Write(pt[2]);
	bs.Write(onOff);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->EventPtOnOff_Map[msgID].Fire(pt, onOff);
}

void RC_Controller::RecvNetwork_EventPtOnOff(RakNet::BitStream& bs, double msgTime_s)
{
	osg::Vec3d pt;
	bool onOff;

	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;
	bs.Read(pt[0]);bs.Read(pt[1]);bs.Read(pt[2]);
	bs.Read(onOff);

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->EventPtOnOff_Map[msgID].Fire(pt, onOff);
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

void RC_Controller::FireNetwork_EventPtValue(std::string msgID, const osg::Vec3d& pt, double val, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, priority, reliability, orderingChannel, m_pParent->GetNetworkID(), ENTITY_EventPtValue);

	// Populate the message
	unsigned char l = msgID.length();
	bs.Write(l);
	bs.Write(msgID.c_str(), l);
	bs.Write(pt[0]);bs.Write(pt[1]);bs.Write(pt[2]);
	bs.Write(val);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Fire the Event
	m_pParent->GetEventMap()->EventPtValue_Map[msgID].Fire(pt, val);
}

void RC_Controller::RecvNetwork_EventPtValue(RakNet::BitStream& bs, double msgTime_s)
{
	osg::Vec3d pt;
	double val;

	// Read the message
	unsigned char l;
	bs.Read(l);
	char* msgID = new char[l+1];
	bs.Read(msgID, l);
	msgID[l] = 0;
	bs.Read(pt[0]);bs.Read(pt[1]);bs.Read(pt[2]);
	bs.Read(val);

	// Fire the Event
	m_pParent->GetEventMap()->AlternateEventTime = &msgTime_s;
	m_pParent->GetEventMap()->EventPtValue_Map[msgID].Fire(pt, val);
	m_pParent->GetEventMap()->AlternateEventTime = NULL;
	delete[] msgID;
}

// Receiving messages, USUALLY from the controller of the entity, and we are receiving messages from our RC
// This might be called before or after our own game loop updates
void RC_Controller::SpecialEntityMessageFromServer(unsigned char msg, double msgTime_s, RakNet::BitStream& entityBS)
{
	switch(msg)
	{
	case ENTITY_PLAYER_CONTROLLED:
	case ENTITY_NONPLAYER_CONTROLLED:
		// We do not care about the time and we do not have any more in the message
		m_pParent->SetPlayerControlled(msg==ENTITY_PLAYER_CONTROLLED);
		break;
	case ENTITY_COLLISION:
		RecvNetwork_Collision(entityBS, msgTime_s);
		break;
	case ENTITY_ExternalForce:
		RecvNetwork_ExternalForce(entityBS, msgTime_s);
		break;
	case ENTITY_Event:
		RecvNetwork_Event(entityBS, msgTime_s);
		break;
	case ENTITY_EventOnOff:
		RecvNetwork_EventOnOff(entityBS, msgTime_s);
		break;
	case ENTITY_EventValue:
		RecvNetwork_EventValue(entityBS, msgTime_s);
		break;
	case ENTITY_EventPt:
		RecvNetwork_EventPt(entityBS, msgTime_s);
		break;
	case ENTITY_EventPtOnOff:
		RecvNetwork_EventPtOnOff(entityBS, msgTime_s);
		break;
	case ENTITY_EventPtValue:
		RecvNetwork_EventPtValue(entityBS, msgTime_s);
		break;
	}
}

void RC_Controller::PosAttUpdate::ReadFromBS(RakNet::BitStream& bs)
{
	//Rick, its faster to unroll these (not sure yet how timing critical this will be, but removing branches is a good thing
	bs.Read(POS[0]),bs.Read(POS[1]),bs.Read(POS[2]);
	bs.Read(ATT[0]),bs.Read(ATT[1]),bs.Read(ATT[2]),bs.Read(ATT[3]);
	bs.Read(LinearVelocity[0]),bs.Read(LinearVelocity[1]),bs.Read(LinearVelocity[2]);
	bs.Read(AngularVelocity[0]),bs.Read(AngularVelocity[1]),bs.Read(AngularVelocity[2]);
	bs.Read(LinearAcceleration[0]),bs.Read(LinearAcceleration[1]),bs.Read(LinearAcceleration[2]);
	bs.Read(AngularAcceleration[0]),bs.Read(AngularAcceleration[1]),bs.Read(AngularAcceleration[2]);
	bs.Read(DeltaTime_s);
	assert((DeltaTime_s > -1e6) && (DeltaTime_s < 1e6));
}

void RC_Controller::PosAttUpdate::WriteToBS(RakNet::BitStream& bs)
{
	bs.Write(POS[0]),bs.Write(POS[1]),bs.Write(POS[2]);
	bs.Write(ATT[0]),bs.Write(ATT[1]),bs.Write(ATT[2]),bs.Write(ATT[3]);
	bs.Write(LinearVelocity[0]),bs.Write(LinearVelocity[1]),bs.Write(LinearVelocity[2]);
	bs.Write(AngularVelocity[0]),bs.Write(AngularVelocity[1]),bs.Write(AngularVelocity[2]);
	bs.Write(LinearAcceleration[0]),bs.Write(LinearAcceleration[1]),bs.Write(LinearAcceleration[2]);
	bs.Write(AngularAcceleration[0]),bs.Write(AngularAcceleration[1]),bs.Write(AngularAcceleration[2]);
	bs.Write(DeltaTime_s);
	assert((DeltaTime_s > -1e6) && (DeltaTime_s < 1e6));
}
