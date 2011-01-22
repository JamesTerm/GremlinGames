// Camera.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

const unsigned int NUM_DISTORTION_LEVELS = 120;
const unsigned int tex_width = 1024;
const unsigned int tex_height = 1024;

using namespace GG_Framework;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

#undef DISABLE_ALL_OPTIMIZATIONS
//////////////////////////////////////////////////////////////////////////

// This constructor happens BEFORE the realize
Camera::Camera(osgViewer::Viewer* camGroup) : m_camGroup(camGroup)
{
	ASSERT(m_camGroup.valid());
	
	m_compositePostDrawCallback = new Camera::CompositeDrawCallback;
	m_cameraViewMatrixCallback = new Camera::CameraViewMatrixCallback(m_camGroup.get());

	// Work with a larger near/far ratio
	m_camGroup->getCamera()->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
	m_camGroup->getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
	m_camGroup->getCamera()->setNearFarRatio(0.00001f);
	m_camGroup->getCamera()->setFinalDrawCallback(m_compositePostDrawCallback.get());
	m_camGroup->getCamera()->setUpdateCallback(m_cameraViewMatrixCallback.get());
}

//////////////////////////////////////////////////////////////////////////

void Camera::setFinalDrawCallback(osg::Camera::DrawCallback* cb)
{
	m_camGroup->getCamera()->setFinalDrawCallback(cb);
}
//////////////////////////////////////////////////////////////////////////


void Camera::addPostDrawCallback(osg::Camera::DrawCallback& cb)
{
	GG_Framework::Base::RefMutexWrapper rmw(m_compositePostDrawCallback->MyMutex);
	osg::ref_ptr<osg::Camera::DrawCallback>* rp = new osg::ref_ptr<osg::Camera::DrawCallback>(&cb);
	m_compositePostDrawCallback->CallbackList.push_back(rp);
}
void Camera::removePostDrawCallback(osg::Camera::DrawCallback& cb)
{
	GG_Framework::Base::RefMutexWrapper rmw(m_compositePostDrawCallback->MyMutex);
	for (Ref_List_impl::iterator i=m_compositePostDrawCallback->CallbackList.begin(); i!=m_compositePostDrawCallback->CallbackList.end(); ++i) 
	{
		osg::ref_ptr<osg::Camera::DrawCallback>* ptr = (*i);
		if (ptr->get() == &cb)
		{
			delete ptr;
			m_compositePostDrawCallback->CallbackList.erase(i);
			return;
		}
	}
}

void Camera::CompositeDrawCallback::operator () (const osg::Camera& camera) const
{
	GG_Framework::Base::RefMutexWrapper rmw(MyMutex);
	for (Ref_List_impl::const_iterator i=CallbackList.begin(); i!=CallbackList.end(); ++i) 
	{
		osg::ref_ptr<osg::Camera::DrawCallback>* ptr = (*i);
		ptr->get()->operator ()(camera);
	}
}

Camera::CompositeDrawCallback::~CompositeDrawCallback()
{
	GG_Framework::Base::RefMutexWrapper rmw(MyMutex);
	for (Ref_List_impl::iterator i=CallbackList.begin(); i!=CallbackList.end(); ++i) 
	{
		osg::ref_ptr<osg::Camera::DrawCallback>* ptr = (*i);
		delete ptr;
	}
}


osg::Node* Camera::GetSceneNode()
{
	return m_camGroup->getSceneData();
}
void Camera::SetSceneNode(osg::Node* node)
{
	// Be sure this happens before the realize
	ASSERT_MSG(!m_camGroup->isRealized(), "Scene Set in Camera::SetSceneNode after camera realized.");

#ifndef DISABLE_ALL_OPTIMIZATIONS
	// optimize the scene graph
	osgUtil::Optimizer optimizer;

	//TODO determine what new optimizations are stable; otherwise the original ones should be fine
	//  [5/15/2009 JamesK]
#if 1
	unsigned optOptions =	// We do not want all options, because it kills some of our camera point at effects
//		osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
//		osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
		osgUtil::Optimizer::REMOVE_LOADED_PROXY_NODES |
		osgUtil::Optimizer::COMBINE_ADJACENT_LODS |
		osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
		osgUtil::Optimizer::MERGE_GEOMETRY |
		osgUtil::Optimizer::CHECK_GEOMETRY |
		osgUtil::Optimizer::OPTIMIZE_TEXTURE_SETTINGS |
		osgUtil::Optimizer::STATIC_OBJECT_DETECTION;
#else
	unsigned optOptions = osgUtil::Optimizer::ALL_OPTIMIZATIONS
	// We do not want all options, because it kills some of our camera point at effects
	& ~(
				osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
				osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
				osgUtil::Optimizer::TRISTRIP_GEOMETRY
		);
#endif
	optimizer.optimize(node, optOptions);
#endif

	m_camGroup->setSceneData(node);

	// this might be a task that could take some time, lets fire a task complete to keep the times updated
	GG_Framework::UI::OSG::LoadStatus::Instance.TaskComplete();
}

void Camera::SetMatrix(const osg::Matrix& camMatrix)
{
	m_cameraViewMatrixCallback->m_camMatrix = camMatrix;
	MatrixUpdate.Fire(camMatrix);
}
//////////////////////////////////////////////////////////////////////////

// Perhaps this getter is safe?
osg::Matrix Camera::GetCameraMatrix() const
{
	return m_cameraViewMatrixCallback->m_camMatrix;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
