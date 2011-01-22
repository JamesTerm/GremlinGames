// VectorDerivativeOverTimeAverager.h
#pragma once
#include <osg/Vec3>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
			/// Use this class to get an averaged out Velocity vector when the velocity due to displacement
			/// is unstable due to rapidly varying frame delta times and/or position vectors.  Use this class
			/// again to smooth out the acceleration (2nd Derivative).
			class FRAMEWORK_UI_OSG_API VectorDerivativeOverTimeAverager
			{
			public:
				VectorDerivativeOverTimeAverager(unsigned numSamples);
				~VectorDerivativeOverTimeAverager();
				osg::Vec3 GetVectorDerivative(osg::Vec3 vec, double dTime_s);
				void Reset(){m_currIndex = (unsigned)-1;}

			private:
				osg::Vec3* m_vecArray;
				double* m_timeArray;
				unsigned m_numSamples;
				unsigned m_currIndex;
				double m_sumTime;
			};
		}
	}
}