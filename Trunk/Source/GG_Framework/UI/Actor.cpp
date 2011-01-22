// GG_Framework.UI Actor.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

Actor::Actor(ActorScene* actorScene, const char* lineFromFile) : IScriptOptionsLineAcceptor(actorScene),
	m_firstAction(NULL), m_currAction(NULL), m_lastAction(NULL), m_debugFile(NULL), m_lastDebugFrameNum(-1)
{
	ASSERT(GetActorScene());
	// Read the rest of the transform
	char actorName[64];
	float X=0.0f, Y=0.0f, Z=0.0f, H=0.0f, P=0.0f, R=0.0f, Sx=1.0f, Sy=1.0f, Sz=1.0f;
	sscanf(lineFromFile, "^ %63s %f %f %f %f %f %f %f %f %f", actorName,
		&X, &Y, &Z, &H, &P, &R, &Sx, &Sy, &Sz);

	// Watch out for LightWave style orientations
	this->setPosition(FromLW_Pos(X,Y,Z));
	this->setAttitude(FromLW_Rot(H,P,R));
	this->setScale(osg::Vec3d(Sx, Sy, Sz));
	this->setName(actorName);

	// Hold on to an update and a framestamp to pass along
	m_frameStamp = new osg::FrameStamp;
	m_update.setFrameStamp( m_frameStamp.get() );

	// Fire a 0 time change to pass along a 0 time, to keep the AnimationPath callbacks from
	// Having the silly offset value
	m_frameStamp->setSimulationTime(0.0);
	m_frameStamp->setFrameNumber(0);
	accept(m_update);

	// Listen for changes to the time
	GetActorScene()->GetTimer()->CurrTimeChanged.Subscribe(ehl, *this, &Actor::GlobalTimeChangedCallback);
}
//////////////////////////////////////////////////////////////////////////

Actor::~Actor()
{
	{
		std::list<Impulse*>::iterator pos;
		for (pos = m_impulseList.begin(); pos != m_impulseList.end(); ++pos)
		{
			Impulse* imp = *pos;
			delete imp;
		}
	}
	{
		ActionMap::iterator pos;
		for (pos = m_actionMap.begin(); pos != m_actionMap.end(); ++pos)
		{
			Action* action = pos->second;
			delete action;
		}
	}

	if (m_firstAction)
	{
		m_lastAction = m_currAction = NULL;
		m_firstAction->ClearFollowing();
		delete m_firstAction;
		m_firstAction = NULL;
	}

	if (m_debugFile)
	{
		fclose(m_debugFile);
		m_debugFile = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////

void Actor::GlobalTimeChangedCallback(double newTime)
{
	double actorTime = GetActorTime(newTime);
	m_frameStamp->setSimulationTime(actorTime);
	m_frameStamp->setFrameNumber(TIME_2_FRAME(actorTime));
	DebugFrameNumber(TIME_2_FRAME(actorTime));

	// Pass the accept down to the children
	accept(m_update);
}
//////////////////////////////////////////////////////////////////////////

double Actor::GetActorTime(double sceneTime)
{
	if (m_currAction)
	{
		// See if we have moved to a new node
		ActionQueNode* currAction = m_currAction;
		while (currAction->GetNext() && (sceneTime > currAction->GetNext()->GetGlobalStartTime_s()))
			currAction = currAction->GetNext();
		while ((sceneTime < currAction->GetGlobalStartTime_s()) && currAction->GetPrev())
			currAction = currAction->GetPrev();
		if (m_currAction != currAction)
		{
			if (&(m_currAction->GetImpulse()) != &(currAction->GetImpulse()))
				m_currAction->GetImpulse().ActiveImpulse(false);
			m_currAction = currAction;
			DebugStringLine(m_currAction->GetAction().GetName());
		}
		m_currAction->GetImpulse().ActiveImpulse(true);
		return m_currAction->GetActionTime_s(sceneTime);
	} 
	else 
	{
		// If there are no events happening, fire the default time event and see if there are any takers
		double defTime = 0.0;
		DefaultTime.Fire(defTime);
		return defTime;	
	}
}
//////////////////////////////////////////////////////////////////////////

void Actor::AddAction(GG_Framework::UI::EventMap& eventMap, const char* line)
{
	Action* action = new Action(eventMap, *(GetActorScene()), line);
	m_actionMap[action->GetName()] = action;
	if (action->GetStartPose() != action->GetEndPose())
		m_poseMap[action->GetStartPose()][action->GetEndPose()] = action;
}
//////////////////////////////////////////////////////////////////////////

void Actor::AddImpulse(GG_Framework::UI::EventMap& eventMap, const char* lineFromFile)
{
	Impulse* i = new Impulse(*this);
	m_impulseList.push_back(i);
	i->Initialize(eventMap, lineFromFile);
}
//////////////////////////////////////////////////////////////////////////

void Actor::QueAction(double queueTime_s, Impulse& impulse, Action& action, bool loop)
{
	if (m_lastAction)
	{
		if (queueTime_s < m_firstAction->GetGlobalStartTime_s())
		{
			m_firstAction->ClearFollowing();
			delete m_firstAction;
			m_lastAction = m_currAction = m_firstAction = NULL;
			DebugStringLine("NO ACTION");
		}
		else
		{
			if (m_lastAction->GetAction().GetEndPose() != action.GetStartPose())
			{
				Action* transAction = m_poseMap[m_lastAction->GetAction().GetEndPose()][action.GetStartPose()];
				if (transAction)
					m_lastAction = m_lastAction->QueNext(queueTime_s, impulse, *transAction, false);
			}
			m_lastAction = m_lastAction->QueNext(queueTime_s, impulse, action, loop);
			return;
		}
	}

	m_firstAction = m_currAction = m_lastAction = 
		new ActionQueNode(queueTime_s, impulse, action, loop);
	DebugStringLine(m_currAction->GetAction().GetName());
}
//////////////////////////////////////////////////////////////////////////

void Actor::ClearActionQue()
{
	if (m_currAction)
	{
		if (GetActorScene()->GetTimer()->GetCurrTime_s() < m_firstAction->GetGlobalStartTime_s())
		{
			m_firstAction->ClearFollowing();
			delete m_firstAction;
			m_lastAction = m_currAction = m_firstAction = NULL;
			DebugStringLine("NO ACTION");
		}
		else
		{
			m_lastAction = m_currAction;
			m_currAction->ClearFollowing();
			DebugStringLine(m_currAction->GetAction().GetName());
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void Actor::DebugOutputActionQue()
{
#ifdef DEBUG_ACTOR_ACTIONS
	DEBUG_ACTOR_ACTIONS("Start Actor::DebugOutputActionQue(%i)\n", TIME_2_FRAME(GetActorScene()->GetTimer()->GetCurrTime_s()));
	ActionQueNode* node = m_firstAction;
	while (node)
	{
		DEBUG_ACTOR_ACTIONS("\t%s (%s-%i)-(%s-%i) %s [%i-%i] %s\n", 
			node->GetAction().GetName().c_str(),
			node->GetAction().GetStartPose().c_str(),
			node->GetAction().GetStartFrame(),
			node->GetAction().GetEndPose().c_str(),
			node->GetAction().GetEndFrame(),
			node->IsLooping() ? "loop" : "single",
			TIME_2_FRAME(node->GetGlobalStartTime_s()),
			TIME_2_FRAME(node->GetGlobalEndTime_s()),
			node==m_currAction ? "<<<<" : "");
		node = node->GetNext();
	}
	DEBUG_ACTOR_ACTIONS("End Actor::DebugOutputActionQue()\n");
#endif DEBUG_ACTOR_ACTIONS
}
//////////////////////////////////////////////////////////////////////////

void Actor::traverse(osg::NodeVisitor& nv)
{
	// Block the messages if frame stamped and coming from something besides me
	if ((nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR) && (nv.getFrameStamp() != GetFrameStamp()))
		return;

	// Pass the message down
	osg::PositionAttitudeTransform::traverse(nv);
}
//////////////////////////////////////////////////////////////////////////

bool Actor::AcceptScriptLine(
	GG_Framework::UI::EventMap& eventMap,
	char indicator,	//!< If there is an indicator character, the caps version is here, or 0 
	const char* lineFromFile)
{
	if (indicator == '@')	// An Action
	{
		AddAction(eventMap, lineFromFile);
		return true;
	}
	else if (indicator == '~')	// An Impulse
	{
		AddImpulse(eventMap, lineFromFile+2);
		return true;
	}
	else if (indicator == 'D')
	{
		// Create a debug file for this actor
		ASSERT(!m_debugFile);
		m_debugFile = fopen(lineFromFile+2, "w");
		ASSERT(m_debugFile);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////

void Actor::DebugStringLine(std::string debugStr)
{
	if (m_debugFile)
	{
		if (m_lastDebugFrameNum == -1)
			fprintf(m_debugFile, "%s\n", debugStr.c_str());
		else
		{
			fprintf(m_debugFile, "\n%s\n", debugStr.c_str());
			m_lastDebugFrameNum = -1;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void Actor::DebugFrameNumber(int frameNum)
{
	if (m_debugFile)
	{
		if (m_lastDebugFrameNum == frameNum)
			fprintf(m_debugFile, ".");	// Just a dot for being the same as last time
		else
		{
			fprintf(m_debugFile, " %i", frameNum);
			m_lastDebugFrameNum = frameNum;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

Actor::ActionQueNode::ActionQueNode
	(double globalStartTime, Impulse& impulse, Action& action, bool looping, ActionQueNode* prev) :
m_prev(prev), m_next(NULL), m_globalStartTime_s(globalStartTime), 
	m_globalEndTime_s(globalStartTime+FRAME_2_TIME(action.GetFrameLength())),
	m_impulse(impulse), m_action(action), m_looping(looping), m_launchDelayedEventsTime(m_globalStartTime_s)
{
}
//////////////////////////////////////////////////////////////////////////

void Actor::ActionQueNode::ClearFollowing()
{
	if (m_next)
	{
		m_next->ClearFollowing();
		delete m_next;
		m_next = NULL;
	}
}
//////////////////////////////////////////////////////////////////////////

Actor::ActionQueNode* Actor::ActionQueNode::QueNext(
	double currTime_s, Impulse& impulse, Action& action, bool looping)
{
	ASSERT(!m_next);
	// Get the proper next time
	double time = currTime_s;
	if (time < m_globalEndTime_s)
		time = m_globalEndTime_s;
	else if (m_looping && (m_globalEndTime_s>m_globalStartTime_s))
	{
		double nextLoopTime = m_globalEndTime_s + m_globalEndTime_s - m_globalStartTime_s;
		while (nextLoopTime < time)
			nextLoopTime += (m_globalEndTime_s - m_globalStartTime_s);
		time = nextLoopTime;
	}

	m_next = new ActionQueNode(time, impulse, action, looping, this);
	return m_next;
}
//////////////////////////////////////////////////////////////////////////

double Actor::ActionQueNode::GetActionTime_s(double globalTime_s)
{
	// Launch the delayed events at our first opportunity
	if (globalTime_s >= m_launchDelayedEventsTime)
	{
		m_action.LaunchDelayedEvents(m_launchDelayedEventsTime);
		if (m_looping && (m_globalEndTime_s > m_globalStartTime_s))
			m_launchDelayedEventsTime += (m_globalEndTime_s-m_globalStartTime_s);
		else
			m_launchDelayedEventsTime = DBL_MAX;
	}
	if (globalTime_s > m_globalEndTime_s)
	{
		if (m_looping && (m_globalEndTime_s > m_globalStartTime_s))
		{
			while (globalTime_s > m_globalEndTime_s)
				globalTime_s -= (m_globalEndTime_s-m_globalStartTime_s);
		}
		else
			globalTime_s = m_globalEndTime_s;
	}
	else if (globalTime_s < m_globalStartTime_s)
		globalTime_s = m_globalStartTime_s;

	double ret_s = m_action.GetActionTime_s(globalTime_s - m_globalStartTime_s);
	return ret_s;
}
//////////////////////////////////////////////////////////////////////////