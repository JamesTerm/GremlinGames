#pragma once

#define __2011_TestCamera__

class FRC_2012_CameraProcessing
{
	public:
		FRC_2012_CameraProcessing();
		~FRC_2012_CameraProcessing();
		void CameraProcessing_TimeChange(double dTime_s);

	private:
		AxisCamera *m_Camera;  //This is a singleton, but treated as a member that is optional
		double m_LastTime;  //Keep track of frame rate
};
