// GG_Framework.UI.OSG ICamera.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

void ICamera::SetCameraManipulator(ICameraManipulator* cameraManip)
{
	if (cameraManip != m_camManip)
	{
		ICameraManipulator* old = m_camManip;
		m_camManip = cameraManip;
		CamManipChanged.Fire(this, old, m_camManip);
	}
}

void ICamera::Update(double dTime_s)
{
	if (m_camManip)
		m_camManip->UpdateCamera(this, dTime_s);
}