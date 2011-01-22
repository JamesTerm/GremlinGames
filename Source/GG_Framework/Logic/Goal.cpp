#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "Goal.h"

using namespace GG_Framework::Logic;

Goal::Goal_Status CompositeGoal::ProcessSubgoals()
{
	Goal_Status StatusOfSubGoals;
	//Remove all completed and failed goals from the front of the subgoal list
	while (!m_SubGoals.empty() && (m_SubGoals.front()->GetStatus()==eCompleted || m_SubGoals.front()->GetStatus()==eFailed))
	{
		m_SubGoals.front()->Terminate();
		delete m_SubGoals.front();
		m_SubGoals.pop_front();
	}
	//If any subgoals remain, process the one at the front of the list
	if (!m_SubGoals.empty())
	{
		//grab the status of the front-most subgoal
		StatusOfSubGoals = m_SubGoals.front()->Process();

		//we have to test for the special case where the front-most subgoal reports "completed" and the subgoal list contains additional goals.
		//When this is the case, to ensure the parent keeps processing its subgoal list, the "active" status is returned.
		if (StatusOfSubGoals == eCompleted && m_SubGoals.size() > 1)
			StatusOfSubGoals=eActive;
	}
	else
		StatusOfSubGoals=eCompleted;
	return StatusOfSubGoals;
}

void CompositeGoal::RemoveAllSubgoals()
{
	for (SubgoalList::iterator it = m_SubGoals.begin(); it!=m_SubGoals.end(); ++it)
	{
		(*it)->Terminate();
		delete *it;
	}
	m_SubGoals.clear();
}
