// GG_Framework.UI Action.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

/// An Action is a mapping of time from beginning to end from one pose to another
/// Once an action starts, it will play through to the end
class FRAMEWORK_UI_API Action
{
public:
	Action(GG_Framework::UI::EventMap& eventMap, ActorScene& actorScene, const char* lineFromFile);
	const std::string& GetName(){return m_name;}
	const std::string& GetStartPose(){return m_startPose;}
	const std::string& GetEndPose(){return m_endPose;}
	int GetStartFrame(){return m_startFrame;}
	int GetEndFrame(){return m_endFrame;}
	int GetFrameLength(){return m_endFrame>m_startFrame ? m_endFrame-m_startFrame : m_startFrame-m_endFrame;}
	void LaunchDelayedEvents(double launchTime);

	double GetActionTime_s(double offsetFromStart);

private:
	class DelayedEvent
	{
	public:
		DelayedEvent(GG_Framework::UI::EventMap& eventMap, 
			ActorScene& actorScene, const char* eventName, 
			int startFramesAfterLaunch, int endFramesAfterLaunch, IEffect::Mode mode);
		void Launch(int launchFrame);

	private:
		std::string m_eventName;
		int m_startFramesAfterLaunch;
		int m_endFramesAfterLaunch;
		int m_launchFrame;
		bool m_started;
		IEffect::Mode m_mode;

		void GlobalTimeChangedCallback(double newTime_s);
		IEvent::HandlerList ehl;
		ActorScene* const m_actorScene;
		GG_Framework::UI::EventMap* const m_eventMap;
	};

	std::string m_name;
	std::string m_startPose;
	std::string m_endPose;
	int m_startFrame;
	int m_endFrame;
	std::list<DelayedEvent> m_delayedEvents;
};
	}
}