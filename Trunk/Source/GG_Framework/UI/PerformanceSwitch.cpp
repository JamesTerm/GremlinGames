// GG_Framework.UI PerformanceSwitch.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;

PerformanceSwitch::PerformanceSwitch(int minPerf, int maxPerf) : m_minPerf(minPerf), m_maxPerf(maxPerf), m_showing(false)
{
	MainWindow::GetMainWindow()->PerformanceIndexChange.Subscribe(ehl, *this, &PerformanceSwitch::PerformanceIndexChanged);
	int currIndex = MainWindow::GetMainWindow()->GetPerformanceIndex();
}
//////////////////////////////////////////////////////////////////////////

void PerformanceSwitch::PerformanceIndexChanged(int oldVal, int newVal)
{
	bool showingNow = ((newVal >= m_minPerf) && (newVal <= m_maxPerf));
	if ((oldVal==newVal) || (showingNow!=m_showing))	// Always make this happen the first time (oldVal==newVal)
	{
		m_showing = showingNow;
		osg::Switch* switchParent = dynamic_cast<osg::Switch*>(getParent(0));
		ASSERT(switchParent);
		switchParent->setChildValue(this, m_showing);
	}
}
//////////////////////////////////////////////////////////////////////////