// GG_Framework.UI Actor.h
#pragma once
#include <osg/PositionAttitudeTransform>
#include <osgUtil/UpdateVisitor>
#include <osg/MatrixTransform>

namespace GG_Framework
{
	namespace UI
	{

class ActorScene;

//! This is the node that is returned from ActorScene::AddActorFile.
//! It can listen for events from GG_FRamework::Logic::Entity3D
class FRAMEWORK_UI_API ActorTransform : public osg::PositionAttitudeTransform
{
public:
	void SubscribeToPosAttChange(Event2<const osg::Vec3&, const osg::Quat&>& posAtEvent)
	{
		posAtEvent.Subscribe(ehl, *this, &ActorTransform::UpdatePosAtt);
	}

	void UnsubscribeToPosAttChange(Event2<const osg::Vec3&, const osg::Quat&>& posAtEvent)
	{
		posAtEvent.Remove(*this, &ActorTransform::UpdatePosAtt);
	}

	void UpdatePosAtt(const osg::Vec3& pos, const osg::Quat& att)
	{
		setPosition(pos);
		setAttitude(att);
	}

protected:
	virtual ~ActorTransform(){}
	IEvent::HandlerList ehl;
};

/// An Actor is a PositionAttitudeTransform that listens for changes to the global timer 
/// and passes down a proper representation of the time based on the action it is performing
class FRAMEWORK_UI_API Actor : 
	public osg::PositionAttitudeTransform, public IScriptOptionsLineAcceptor
{
public:
	Actor(ActorScene* actorScene, const char* lineFromFile);
	~Actor();

	virtual double GetActorTime(double sceneTime);
	void GlobalTimeChangedCallback(double newTime_s);
	osg::FrameStamp* GetFrameStamp(){return m_frameStamp.get();}

	void AddImpulse(GG_Framework::UI::EventMap& eventMap, const char* lineFromFile);
	void AddAction(GG_Framework::UI::EventMap& eventMap, const char* line);
	Action* GetAction(const std::string& name){return m_actionMap[name];}

	void QueAction(double queueTime_s, Impulse& impulse, Action& action, bool loop);
	void ClearActionQue();

	char* GetCurrActionName(){return m_currAction ? m_currAction->GetAction().GetName().c_str() : "NONE";}

	void DebugOutputActionQue();

	// We do not want to pass messages to children except those we originate ourselves
	virtual void traverse(osg::NodeVisitor& nv);

	virtual bool AcceptScriptLine(
		GG_Framework::UI::EventMap& eventMap, 
		char indicator,	//!< If there is an indicator character, the caps version is here, or 0 
		const char* lineFromFile);

	// These are all used for debugging the frames passing through the actor
	void DebugStringLine(std::string debugStr);
	void DebugFrameNumber(int frameNum);

	Event1<double&>	DefaultTime;

private:
	FILE* m_debugFile;
	int m_lastDebugFrameNum;
	IEvent::HandlerList ehl;
	osgUtil::UpdateVisitor m_update;
	osg::ref_ptr<osg::FrameStamp> m_frameStamp;

	class ActionQueNode
	{
	public:
		ActionQueNode(double globalStartTime_s, Impulse& impulse, Action& action, bool looping, ActionQueNode* prev = NULL);
		ActionQueNode* GetPrev() const {return m_prev;}
		ActionQueNode* GetNext() const {return m_next;}
		void ClearFollowing();
		ActionQueNode* QueNext(double currTime_s, Impulse& impulse, Action& action, bool looping);
		double GetActionTime_s(double globalTime_s);
		double GetGlobalStartTime_s() const {return m_globalStartTime_s;}
		double GetGlobalEndTime_s() const {return m_globalEndTime_s;}
		Impulse& GetImpulse(){return m_impulse;}
		Action& GetAction(){return m_action;}
		bool IsLooping(){return m_looping;}

	private:
		ActionQueNode* m_prev;
		ActionQueNode* m_next;
		double m_globalStartTime_s;
		double m_globalEndTime_s;
		Impulse& m_impulse;
		Action& m_action;
		bool m_looping;
		double m_launchDelayedEventsTime;
	};

	ActionQueNode* m_firstAction;
	ActionQueNode* m_lastAction;
	ActionQueNode* m_currAction;

	std::list<Impulse*> m_impulseList;
	typedef std::map< const std::string, Action*, std::greater<std::string> > ActionMap;
	ActionMap m_actionMap;
	std::map< const std::string, ActionMap, std::greater<std::string> > m_poseMap;
};
	}
}