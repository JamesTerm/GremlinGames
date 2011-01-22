// GG_Framework.UI Impulse.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

class Actor;
//! an Impulse handles an Event and places an Action on the QUE for an Actor
class FRAMEWORK_UI_API Impulse : public IEffect
{
public:
	Impulse(Actor& parent);
	virtual void LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt);

	// Called by the Actor to indicate this Impulse is still active.  Only one can be active at a time
	// Used so the impulse knows it can complete the last action
	void ActiveImpulse(bool active){m_active = active;}

protected:
	virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);

private:
	// Just used for debugging
	std::string m_debugDesc;

	bool m_active;
	Actor& m_parent;
	std::vector<Action*> m_actionList;
	Action* m_finalAction;
};

	}
}