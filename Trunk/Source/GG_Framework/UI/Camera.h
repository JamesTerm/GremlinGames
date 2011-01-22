// Camera.h
#pragma once

#include <osg/NodeCallback>
#include <osgViewer/Viewer>
typedef std::list<osg::ref_ptr<osg::Camera::DrawCallback>* > Ref_List_impl;


namespace GG_Framework
{
	namespace UI
	{


class Camera : public GG_Framework::UI::OSG::ICamera
{
private:

	osg::ref_ptr<osgViewer::Viewer> m_camGroup;
	class CompositeDrawCallback : public osg::Camera::DrawCallback
	{		
	protected:
		virtual ~CompositeDrawCallback();
	public:
		Ref_List_impl CallbackList;
		mutable OpenThreads::Mutex MyMutex;
		virtual void operator () (const osg::Camera& camera) const;
	};

	class CameraViewMatrixCallback : public osg::NodeCallback
	{
	public:
		// Use our atomic class to make our camera matrix thread save
		GG_Framework::Base::AtomicT<osg::Matrix> m_camMatrix;
		osgViewer::Viewer* m_camGroup; // Not ref counted since we will be assigned as a child of this cam group

		CameraViewMatrixCallback(osgViewer::Viewer* camGroup) : m_camGroup(camGroup)
		{
			m_camMatrix = m_camGroup->getCamera()->getViewMatrix();
		};
		
		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
		{
			m_camGroup->getCamera()->setViewMatrix(m_camMatrix);
			osg::NodeCallback::operator ()(node, nv);
		};
	};

	osg::ref_ptr<CompositeDrawCallback> m_compositePostDrawCallback;
	osg::ref_ptr<CameraViewMatrixCallback> m_cameraViewMatrixCallback;

public:

	Camera(osgViewer::Viewer* camGroup);

	virtual void addPostDrawCallback(osg::Camera::DrawCallback& cb);
	virtual void removePostDrawCallback(osg::Camera::DrawCallback& cb);

	virtual void setFinalDrawCallback(osg::Camera::DrawCallback* cb);

	virtual void SetMatrix(const osg::Matrix& camMatrix);
	virtual osg::Matrix GetCameraMatrix() const;

	// These should be ok, the getter just gets and the setter happens before realize
	virtual osg::Node* GetSceneNode();
	virtual void SetSceneNode(osg::Node* node);
};
	}
}