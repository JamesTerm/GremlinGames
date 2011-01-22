// Interfaces.h
#pragma once
#include <osg/NodeCallback>
#include <osgViewer/Viewer>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
			class ICamera;
			class ICameraManipulator
			{
			public:
				//! This is called once per frame when the camera is active
				//! use it to update the matrix of the camera (or anything else)
				//! you can also bounce out and do nothing if there are no changes.
				virtual void UpdateCamera(ICamera* activeCamera, double dTime_s) = 0;
			};

			class FRAMEWORK_UI_OSG_API ICamera
			{
				public:
					ICamera() : m_camManip(NULL) {}
					virtual void setClearColor(const osg::Vec4& c) = 0;
					virtual void getClearColor(osg::Vec4& c) = 0;

					virtual void addPostDrawCallback(osg::Camera::DrawCallback& cb) = 0;
					virtual void removePostDrawCallback(osg::Camera::DrawCallback& cb) = 0;

					virtual void setFinalDrawCallback(osg::Camera::DrawCallback* cb) = 0;

					virtual osg::Matrix GetCameraMatrix() const =0;
					virtual void SetMatrix(const osg::Matrix& camMatrix, float distortionAmt) = 0;
					virtual osg::Node* GetSceneNode() = 0;
					virtual void SetSceneNode(osg::Node* node, float distortionPCNT) = 0;

					virtual void SetCameraManipulator(ICameraManipulator* cameraManip);
					Event3<ICamera*, ICameraManipulator*, ICameraManipulator*> CamManipChanged; // (this, old, new)
					ICameraManipulator* GetCameraManipulator(){return m_camManip;}

					virtual void Update(double dTime_s);
					Event1<const osg::Matrix&> MatrixUpdate;

				protected:
					ICameraManipulator* m_camManip;
			};
		}
	}	//end namespace OSG
}	//end namespace FrameWork
