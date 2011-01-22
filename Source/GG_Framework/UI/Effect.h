// GG_Framework.UI Effect.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
class ActorScene;

//! An IEffect is attached to an Actor and gets fired when an event happens
class FRAMEWORK_UI_API IEffect
{
public:
	enum Mode{ON = 'O', WHILE = 'W', TOGGLE = 'T', LOOP = 'L'};

	IEffect(ActorScene* actorScene) : m_actorScene(actorScene), m_pressed(false), m_mode(ON), m_constant(false), m_subscribedEventMap(NULL) {}
	void Initialize(GG_Framework::UI::EventMap& localMap, const char* lineFromFile);

	virtual void LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt) = 0;

	bool GetPressed(){return m_pressed;}
	bool GetLooping(){return (m_mode!=ON);}
	Mode GetMode(){return m_mode;}

	virtual double GetDefaultIntensity(){return 1.0;}
	virtual osg::Vec3d GetDefaultPoint(){return osg::Vec3d(0,0,0);}

protected:
	// Always call this base class version
	virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);
	ActorScene* const m_actorScene;
	bool m_constant;	// Only true if a Toggle or While START or SPAWN

private:
	void EventCallbackDnUp(bool pressed);
	void EventCallbackIntensity(double intensity);
	void EventCallbackDn();

	void EventCallbackPtDnUp(const osg::Vec3d& pt, bool pressed);
	void EventCallbackPtIntensity(const osg::Vec3d& pt, double intensity);
	void EventCallbackPt(const osg::Vec3d& pt);

	bool m_pressed;
	Mode m_mode;
	IEvent::HandlerList ehl;

	// I will use this to get the network times if there are any
	GG_Framework::UI::EventMap* m_subscribedEventMap;
};
	}
}