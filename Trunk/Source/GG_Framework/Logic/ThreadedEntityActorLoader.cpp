// GG_Framewor.Logic Entity3D.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"

using namespace GG_Framework::Logic;


void ThreadedEntityActorLoader::tryRun()
{
	if (m_uiActorScene)
		uiActorTransform = m_uiActorScene->ReadActorFile(*entity3D->GetEventMap(), m_filename.c_str());
	if (m_bbActorScene)
	{
		// Do not even try to load the BB if it is just a bullet
		if (entity3D->GetCollisionIndex() > 0)
			bbActorTransform = m_bbActorScene->ReadActorFile(*entity3D->GetEventMap(), m_filename.c_str());
		else
			bbActorTransform = new GG_Framework::UI::ActorTransform;
	}
	entity3D->OnAsyncLoad();
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	m_state = 1;	// Completed the run
}
//////////////////////////////////////////////////////////////////////////

int ThreadedEntityActorLoader::UpdateState()
{
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	if (m_state == 1)
	{
		m_state = 2;	// We should 
		return 1;
	}
	else
		return m_state;
}
//////////////////////////////////////////////////////////////////////////

Entity3D* ThreadedEntityActorLoader::Complete()
{
	if (uiActorTransform.valid())
		entity3D->SetActor(uiActorTransform.get(), false);
	if (bbActorTransform.valid())
		entity3D->SetActor(bbActorTransform.get(), true);
	entity3D->OnAsyncLoadComplete();
	return entity3D;
}
//////////////////////////////////////////////////////////////////////////