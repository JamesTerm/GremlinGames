// VectorDerivativeOverTimeAverager.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

VectorDerivativeOverTimeAverager::VectorDerivativeOverTimeAverager(unsigned numSamples)
{
	m_numSamples = numSamples;
	ASSERT(m_numSamples > 0);
	m_vecArray = new osg::Vec3[m_numSamples];
	m_timeArray = new double[m_numSamples];
	m_currIndex = (unsigned)-1;
	m_sumTime = 0.0;
}
//////////////////////////////////////////////////////////////////////////

VectorDerivativeOverTimeAverager::~VectorDerivativeOverTimeAverager()
{
	delete[] m_vecArray;
	delete[] m_timeArray;
}
//////////////////////////////////////////////////////////////////////////

osg::Vec3 VectorDerivativeOverTimeAverager::GetVectorDerivative(osg::Vec3 vec, double dTime_s)
{
	// If this is the very FIRST pass, initialize all of the array with this vec (will return 0)
	if (m_currIndex == (unsigned)-1)
	{
		for (unsigned i = 0; i < m_numSamples; ++i)
		{
			m_vecArray[i] = vec;
			m_timeArray[i] = 0.0;
		}
		m_currIndex = 0;
		m_sumTime = 0.0;
	}

	// Find the total time elapsed
	m_sumTime += dTime_s - m_timeArray[m_currIndex];
	m_timeArray[m_currIndex] = dTime_s;

	// Find the derivative
	osg::Vec3 vecDeriv(0,0,0);
	if (m_sumTime > 0.0)
		vecDeriv = (vec - m_vecArray[m_currIndex]) / m_sumTime;
	m_vecArray[m_currIndex] = vec;

	// Get ready for the next pass
	m_currIndex += 1;
	if (m_currIndex == m_numSamples)
		m_currIndex = 0;

	return vecDeriv;
}
//////////////////////////////////////////////////////////////////////////
