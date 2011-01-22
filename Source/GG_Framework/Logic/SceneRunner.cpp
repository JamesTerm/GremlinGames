// GG_Framework.Logic SceneRunner.h
#include "stdafx.h"
#include "GG_Framework.Logic.h"

using namespace GG_Framework::Logic;

void GameSceneRunner::FireStart()
{
	ISceneRunner::FireStart();
	for (unsigned i = 0; i < (unsigned)m_gameScene->MapList.size(); ++i)
		m_gameScene->MapList[i]->Event_Map["START"].Fire();
}
//////////////////////////////////////////////////////////////////////////

// Only call this after the Thread exits all loaded
void GameSceneRunner::MyRunScene(UI_GameScene& gameScene,
								 GG_Framework::UI::OSG::ICamera& cam, 
								 GG_Framework::UI::OSG::IKeyboardMouse& kbm)
{
	m_gameScene = &gameScene;
	// Connect the real Camera to the Proxy
	m_gameScene->UI_CamProxy.SetCamera(&cam);
	osg::Node* centeredNode = m_gameScene->UI_ActorScene.CenteredNode;
	if (centeredNode == NULL)
		centeredNode = m_gameScene->UI_ActorScene.GetScene()->getChild(0);
	RunScene(m_gameScene->UI_ActorScene.GetScene(), centeredNode, cam, kbm);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

LogoSceneRunner::LogoSceneRunner(std::string gameName, ThreadedClientGameLoader& threadConn, ServerManager* threadAI) : 
	GG_Framework::UI::OSG_Timer_SceneRunner(), m_threadConn(threadConn), m_gameName(gameName), m_threadAI(threadAI) {}
//////////////////////////////////////////////////////////////////////////

void LogoSceneRunner::MyRunScene(const char* fn, GG_Framework::UI::OSG::ICamera& cam, GG_Framework::UI::OSG::IKeyboardMouse& kbm)
{
	GG_Framework::UI::ActorScene actorScene(GetTimerEvent(), &cam);
	osg::Group* mainGroup = actorScene.AddActorFile(globalEventMap, fn);
	RunScene(actorScene.GetScene(), mainGroup, cam, kbm);
}
//////////////////////////////////////////////////////////////////////////

void LogoSceneRunner::FireStart()
{
	// Wait to start the thread connection thread until much later
	m_threadConn.start();
	if (m_threadAI)
		m_threadAI->start();
	GG_Framework::UI::OSG_Timer_SceneRunner::FireStart();
}
//////////////////////////////////////////////////////////////////////////

bool LogoSceneRunner::LoopUpdate(GG_Framework::UI::OSG::ICamera& cam)
{
	// Watch for a Title Update
	std::string statusUpdate = m_threadConn.GetStatusUpdate();
	if (!statusUpdate.empty())
		cam.SetWindowText(GG_Framework::Base::BuildString("%s: %s", m_gameName.c_str(), statusUpdate.c_str()).c_str());
	if  (m_threadConn.IsLoadComplete())
		return false;
	else
		return GG_Framework::UI::ISceneRunner::LoopUpdate(cam);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////