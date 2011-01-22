// DestructionProxyEntity.cpp
#include "stdafx.h"
#include "GG_Framework.AppReuse.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::AppReuse;
using namespace GG_Framework::Logic;

// DestructionProxyEntity will use the base TransmittedEntity
ClassFactoryT<TransmittedEntity, TransmittedEntity> DPE_TransmittedEntity_ClassFactory("DestructionProxyEntity", TransmittedEntity::ClassFactory);

// My own class factory
ClassFactoryT<DestructionProxyEntity, Entity3D> DPE_ClassFactory("DestructionProxyEntity", Entity3D::ClassFactory);

void DestructionProxyEntity::Initialize
	(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te)
{
	__super::Initialize(gs, em, te);
	// Start off HIDDEN
	ShowNextFrame(false);

	// DestructionProxyEntity NEVER allow physics when receiving messages
	GetRC_Controller().SetRecvIgnorePhysics(true);
}
//////////////////////////////////////////////////////////////////////////

void DestructionProxyEntity::AlignWithOrigAndShow(osg::Vec3d origCurrPos_m, osg::Quat origAtt, const GG_Framework::Logic::Physics::PhysicsEntity& origPhysics)
{
	// This should only get called locally
	ASSERT(IsLocallyControlled());

	// These messages should all get sent about properly with the network
	ShowNextFrame(true);	// Set to true so the very next update gets sent properly
	SetPosAtt(origCurrPos_m, origAtt);
	m_Physics.SetLinearVelocity(origPhysics.GetLinearVelocity());
	m_Physics.SetAngularVelocity(origPhysics.GetAngularVelocity());
	osg::Vec3d zero;
	m_Physics.SetTargetAcceleration(zero);
	GetRC_Controller().ResetLastPosAtt();
	// m_Physics.CopyFrom(origPhysics);
}
//////////////////////////////////////////////////////////////////////////
