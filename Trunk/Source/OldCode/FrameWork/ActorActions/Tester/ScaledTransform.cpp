// FrameWork.ScaledTransformActions.Tester ScaledTransform.cpp
#include "stdafx.h"
#include "../../../CompilerSettings.h"
#include "../FrameWork_ActorActions.h"

using namespace FrameWork::ActorActions;

namespace FrameWork
{
	namespace ActorActions
	{
		namespace Tester
		{

class MockCamera : public FrameWork::OSG::ICamera
{
public:
	virtual void setClearColor(const osg::Vec4& c){}
	virtual void getClearColor(osg::Vec4& c){}
	virtual void addPostDrawCallback(Producer::Camera::Callback& cb){}
	virtual void setWindowRectangle(int x, int y, int w, int h, bool resize){}
	virtual void fullScreen(bool fs){}
	virtual bool isFullScreen(){return false;}
	virtual void SetWindowText(const char* windowTitle){}
	virtual bool UpdateWithMatrix(const osg::Matrix& camMatrix)
	{
		m_mat = camMatrix; 
		MatrixUpdate.Fire(camMatrix);
		return true;
	}
	virtual osg::Matrix GetCameraMatrix() const {return m_mat;}

private:
	osg::Matrix m_mat;
};


SUITE(ScaledTransform_Tester)
{
	TEST(CheckOffset0)
	{
		MockCamera cam;
		osg::ref_ptr<ScaledTransform> st0 = new ScaledTransform(0.0);
		osg::Matrix camMat;
		osg::Vec3d eye(1000.0, 5000, 20.0);
		osg::Vec3d center(0,0,0);
		osg::Vec3d up(0,1,0);
		camMat.makeLookAt(eye, center, up);
		cam.UpdateWithMatrix(camMat);	// Fires the event that updates

		// Make sure the position is right
		osg::Vec3d newPos = st0->getPosition();
		CHECK_CLOSE(eye[0], newPos[0], 0.00001);
		CHECK_CLOSE(eye[1], newPos[1], 0.00001);
		CHECK_CLOSE(eye[2], newPos[2], 0.00001);

		// Check the renderbin
		CHECK_EQUAL(-2, st0->getOrCreateStateSet()->getBinNumber());
	}

	TEST(CheckOffset1)
	{
		MockCamera cam;
		osg::ref_ptr<ScaledTransform> st0 = new ScaledTransform(0.1);
		osg::Matrix camMat;
		osg::Vec3d eye(1000.0, 5000.0, 200.0);
		osg::Vec3d expEye(900.0, 4500.0, 180.0);
		osg::Vec3d center(0,0,0);
		osg::Vec3d up(0,1,0);
		camMat.makeLookAt(eye, center, up);
		cam.UpdateWithMatrix(camMat);	// Fires the event that updates

		// Make sure the position is right
		osg::Vec3d newPos = st0->getPosition();
		CHECK_CLOSE(expEye[0], newPos[0], 0.00001);
		CHECK_CLOSE(expEye[1], newPos[1], 0.00001);
		CHECK_CLOSE(expEye[2], newPos[2], 0.00001);

		// Check the renderbin
		CHECK_EQUAL(-1, st0->getOrCreateStateSet()->getBinNumber());
	}
}

		}
	}
}