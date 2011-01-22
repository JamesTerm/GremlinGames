// GG_Framework.Logic ThreadedClientGameLoader.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"

using namespace GG_Framework::Logic;

ThreadedClientGameLoader::ThreadedClientGameLoader(GG_Framework::Logic::UI_GameClient& gameClient) : 
		m_gameClient(gameClient), m_loadComplete(false), m_status(NULL)
{
	m_gameClient.LoadStatusUpdate.Subscribe(ehl, *this, &ThreadedClientGameLoader::UpdateStatus);
	setCancelModeAsynchronous();
	this->setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_HIGH);

}
//////////////////////////////////////////////////////////////////////////

void ThreadedClientGameLoader::UpdateStatus(const char* status)
{
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	m_status = status;
}
//////////////////////////////////////////////////////////////////////////

void ThreadedClientGameLoader::tryRun()
{
	// Have the GameClient load from the Server, which connects first
	bool loadOK = m_gameClient.LoadInitialGameData();
	if (!loadOK)
		UpdateStatus("Failed to Connect To Server");

	// All Done
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	m_loadComplete = loadOK;
}
//////////////////////////////////////////////////////////////////////////

bool ThreadedClientGameLoader::IsLoadComplete()
{
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	return m_loadComplete;
}
//////////////////////////////////////////////////////////////////////////

std::string ThreadedClientGameLoader::GetStatusUpdate()
{
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	std::string ret(m_status ? m_status : "");
	m_status = NULL;
	return ret;
}
//////////////////////////////////////////////////////////////////////////