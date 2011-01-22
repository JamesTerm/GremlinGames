// GG_Framework.UI CameraParentedTransforms.cpp
#include "StdAfx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::Base;

namespace GG_Framework
{
	namespace UI
	{
		CameraParentedTransform::CameraParentedTransform(GG_Framework::UI::OSG::ICamera* cam)
		{
			this->setName("CameraParentedTransform");
			if (cam)
				cam->MatrixUpdate.Subscribe(ehl, *this, &CameraParentedTransform::CameraUpdateCallback);
		}
		//////////////////////////////////////////////////////////////////////////

		void CameraParentedTransform::CameraUpdateCallback(const osg::Matrix& camMatrix)
		{
			// Find the out where the camera is and where it is pointing
			osg::Vec3d eye, center, up;
			camMatrix.getLookAt(eye, center, up);
			this->setPosition(eye);
			this->setAttitude(GG_Framework::UI::OSG::MimicCameraRot(center-eye, up).getRotate());
		}
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		void ScaledTransform::CameraUpdateCallback(const osg::Matrix& camMatrix)
		{
			osg::Vec3d eye, center, up;
			camMatrix.getLookAt(eye, center, up);
			if (m_adjScale > 0.0)
				this->setPosition(eye - (eye*m_adjScale));
			else
				this->setPosition(eye);
		}
		//////////////////////////////////////////////////////////////////////////

		ScaledTransform::ScaledTransform(GG_Framework::UI::OSG::ICamera& cam, double adjScale) : m_adjScale(adjScale)
		{
			ASSERT(adjScale >= 0.0);
			this->setName((m_adjScale > 0.0) ? "Large-ScaledTransform" : "Super-ScaledTransform");
			cam.MatrixUpdate.Subscribe(ehl, *this, &ScaledTransform::CameraUpdateCallback);

			// clear the depth to the far plane and set the render bin
			osg::StateSet* dstate = this->getOrCreateStateSet();
			osg::Depth* depth = new osg::Depth;
			depth->setFunction(osg::Depth::ALWAYS);
			depth->setRange(1.0,1.0);   
			dstate->setAttributeAndModes(depth,osg::StateAttribute::ON );
			dstate->setRenderBinDetails((m_adjScale > 0.0) ? -1 : -2,"DepthSortedBin");
		}
		//////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

		void LightBloomTransform::CameraUpdateCallback(const osg::Matrix& camMatrix)
		{
			// Find out where the camera is GLOBALLY
			osg::Vec3d eye, center, up;
			camMatrix.getLookAt(eye, center, up);

			// Find out where we are GLOBALLY
			osg::Vec3d pos = GG_Framework::UI::OSG::GetNodePosition(this, NULL, NULL);

			// And the distance
			double dist = (pos-eye).length();

			// Find the scale we need to apply
			double scaleToApply = 1.0;
			if (dist > m_farDist)
				scaleToApply = m_farDist/m_nomDist;
			else if (dist > m_nomDist)
				scaleToApply = dist/m_nomDist;

			// DEBUGGING
			// DebugOut_PDCB::TEXT = GG_Framework::Base::BuildString("nomDist=%f, farDist=%f, dist=%f, scale=%f", m_nomDist, m_farDist, dist, scaleToApply);

			// Apply the scale
			this->setScale(osg::Vec3d(scaleToApply,scaleToApply,scaleToApply));
		}
		//////////////////////////////////////////////////////////////////////////

		LightBloomTransform::LightBloomTransform(GG_Framework::UI::OSG::ICamera& cam, double nomDist, double farDist) : 
			m_nomDist(nomDist), m_farDist(farDist)
		{
			ASSERT(m_nomDist > 0.0);
			ASSERT(m_farDist > m_nomDist);
			this->setName("Light Bloom");
			cam.MatrixUpdate.Subscribe(ehl, *this, &LightBloomTransform::CameraUpdateCallback);
		}
		//////////////////////////////////////////////////////////////////////////
	}
}