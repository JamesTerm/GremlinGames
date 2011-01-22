// GG_Framework.UI LoadStatus.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

// The singleton instance
GG_Framework::UI::OSG::LoadStatus GG_Framework::UI::OSG::LoadStatus::Instance;

/// This function is called several times during the load process
void GG_Framework::UI::OSG::LoadStatus::TaskComplete()
{
	// This all has to be thread safe
	RefMutexWrapper rmw(m_mutex);

	// We have already been marked as complete, do not fire more events
	if (m_complete)
		return;

	// Get the current time
	float currTime = m_OSG_timer.delta_s(m_initialTimer,m_OSG_timer.tick());
	m_actualLoadTimes.push_back(currTime);

	// Find the remaining time
	float remTime = -1.0f;	// Defaults to -1.0 s if there are no better predictions
	size_t index = m_actualLoadTimes.size()-1;
	if (m_predictedLoadTimes.size() > (index+1))
	{
		float compTime = m_predictedLoadTimes[m_predictedLoadTimes.size()-1];
		float predTime = m_predictedLoadTimes[index];
		if ((predTime > 0.0f) && (compTime >= predTime))
			remTime = ((currTime/predTime)*compTime) - currTime;
	}

	// Fire the event to show the update
	LoadStatusUpdate.Fire(currTime, remTime);
}
//////////////////////////////////////////////////////////////////////////

/// This function is called only one time, when the load is REALLY complete
/// Writes the completion times to a file
void GG_Framework::UI::OSG::LoadStatus::LoadComplete()
{
	// This all has to be thread safe
	RefMutexWrapper rmw(m_mutex);

	// We have already been marked as complete, do not fire more events
	if (m_complete)
		return;

	// No longer take any messages
	m_complete = true;

	// Get the current time
	float currTime = m_OSG_timer.delta_s(m_initialTimer,m_OSG_timer.tick());
	m_actualLoadTimes.push_back(currTime);

	// Write the times out to a file
	FILE* outFile = fopen("LOAD_TIMES.txt", "w");
	if (outFile)
	{
		// Write the times in a way easy to paste to LUA
		fprintf(outFile, "LOAD_TIMES = {\n");
		for (size_t i = 0; i < m_actualLoadTimes.size(); ++i)
			fprintf(outFile, "\t%f,\n", m_actualLoadTimes[i]);
		fprintf(outFile, "}\n");
		fclose(outFile);
		outFile = NULL;
	}

	// Fire the event to say the load is complete
	LoadStatusUpdate.Fire(currTime, 0.0f);
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::UI::OSG::LoadStatus::SetPredictedLoadTimes(std::vector<float>& predLoad)
{
	// This all has to be thread safe
	RefMutexWrapper rmw(m_mutex);

	// We have already been marked as complete, do not fire more events
	if (m_complete)
		return;

	// If there are no items, it does not help us
	if (predLoad.empty())
		return;

	// We want to keep this list of predicted times
	m_predictedLoadTimes = predLoad;

	// Fire the event to let everyone know about the remaining time
	LoadStatusUpdate.Fire(0.0f, m_predictedLoadTimes[m_predictedLoadTimes.size()-1]);
}
//////////////////////////////////////////////////////////////////////////