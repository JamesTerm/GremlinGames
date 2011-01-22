// Camera.h
#pragma once
#include <osg/NodeCallback>
#include <osgViewer/Viewer>
typedef std::list<osg::ref_ptr<osg::Camera::DrawCallback>* > Ref_List_impl;

class DistortionSubgraph;
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
	osg::ref_ptr<CompositeDrawCallback> m_compositePostDrawCallback;
	DistortionSubgraph* m_distortion;

public:
	Camera(osgViewer::Viewer* camGroup);
	virtual void setClearColor(const osg::Vec4& c);
	virtual void getClearColor(osg::Vec4& c);

	virtual void addPostDrawCallback(osg::Camera::DrawCallback& cb);
	virtual void removePostDrawCallback(osg::Camera::DrawCallback& cb);

	virtual void setFinalDrawCallback(osg::Camera::DrawCallback* cb);

	virtual void SetMatrix(const osg::Matrix& camMatrix, float distortionAmt);
	virtual osg::Matrix GetCameraMatrix() const;
	virtual osg::Node* GetSceneNode();
	virtual void SetSceneNode(osg::Node* node, float distortionPCNT);
};
	}
}