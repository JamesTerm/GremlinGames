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

class DistortionSubgraph
{
private:
	osg::ref_ptr<osg::Vec2Array> m_textCoords[NUM_DISTORTION_LEVELS];
	unsigned m_currIndex;
	osg::ref_ptr<osg::Geometry> m_polyGeom;

public:
	osg::Node* createDistortionSubgraph(osg::Node* subgraph, const osg::Vec4& clearColour, float amount)
	{
		osg::Group* distortionNode = new osg::Group;

		osg::Texture2D* texture = new osg::Texture2D;
		texture->setTextureSize(tex_width, tex_height);
		texture->setInternalFormat(GL_RGBA);
		texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
		texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

		// set up the render to texture camera.
		{
			osg::CameraNode* camera = new osg::CameraNode;

			// set clear the color and depth buffer
			camera->setClearColor(clearColour);
			camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

			// just inherit the main cameras view
			camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
			camera->setProjectionMatrix(osg::Matrixd::identity());
			camera->setViewMatrix(osg::Matrixd::identity());

			// set viewport
			camera->setViewport(0,0,tex_width,tex_height);

			// set the camera to render before the main camera.
			camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

			// tell the camera to use OpenGL frame buffer object where supported.
			camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

			// attach the texture and use it as the color buffer.
			camera->attach(osg::CameraNode::COLOR_BUFFER, texture);

			// add subgraph to render
			camera->addChild(subgraph);

			distortionNode->addChild(camera);
		}

		// set up the render to texture camera.
		{
			// create the quad to visualize.
			m_polyGeom = new osg::Geometry();

			m_polyGeom->setSupportsDisplayList(false);

			osg::Vec3 origin(0.0f,0.0f,0.0f);
			osg::Vec3 xAxis(1.0f,0.0f,0.0f);
			osg::Vec3 yAxis(0.0f,1.0f,0.0f);
			osg::Vec3 zAxis(0.0f,0.0f,1.0f);
			float height = 1024.0f;
			float width = 1280.0f;
			int noSteps = 50;

			osg::Vec3Array* vertices = new osg::Vec3Array;
			osg::Vec4Array* colors = new osg::Vec4Array;

			for (unsigned tc = 0; tc < NUM_DISTORTION_LEVELS; ++tc)
				m_textCoords[tc] = new osg::Vec2Array;

			osg::Vec3 bottom = origin;
			osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
			osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));

			osg::Vec2 bottom_texcoord(0.0f,0.0f);
			osg::Vec2 dx_texcoord(1.0f/(float)(noSteps-1),0.0f);
			osg::Vec2 dy_texcoord(0.0f,1.0f/(float)(noSteps-1));

			osg::Vec3 cursor = bottom;
			osg::Vec2 texcoord = bottom_texcoord;
			int i,j;
			for(i=0;i<noSteps;++i)
			{
				osg::Vec3 cursor = bottom+dy*(float)i;
				osg::Vec2 texcoord = bottom_texcoord+dy_texcoord*(float)i;
				for(j=0;j<noSteps;++j)
				{
					vertices->push_back(cursor);
					osg::Vec2 distort = osg::Vec2((sin(texcoord.x()*osg::PI-osg::PI*0.5)+1.0f)*0.5f,(sin(texcoord.y()*osg::PI-osg::PI*0.5)+1.0f)*0.5f);

					for (unsigned tc = 0; tc < NUM_DISTORTION_LEVELS; ++tc)
					{
						double blendAmt = amount * (double)tc / (double)(NUM_DISTORTION_LEVELS-1);
						m_textCoords[tc]->push_back((distort*blendAmt)+(texcoord*(1.0f-blendAmt)));
					}
					
					colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
					cursor += dx;
					texcoord += dx_texcoord;
				}
			}

			// pass the created vertex array to the points geometry object.
			m_polyGeom->setVertexArray(vertices);

			m_polyGeom->setColorArray(colors);
			m_polyGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

			m_currIndex = NUM_DISTORTION_LEVELS-1;
			m_polyGeom->setTexCoordArray(0,m_textCoords[m_currIndex].get());


			for(i=0;i<noSteps-1;++i)
			{
				osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
				for(j=0;j<noSteps;++j)
				{
					elements->push_back(j+(i+1)*noSteps);
					elements->push_back(j+(i)*noSteps);
				}
				m_polyGeom->addPrimitiveSet(elements);
			}


			// new we need to add the texture to the Drawable, we do so by creating a 
			// StateSet to contain the Texture StateAttribute.
			osg::StateSet* stateset = m_polyGeom->getOrCreateStateSet();
			stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
			stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

			osg::Geode* geode = new osg::Geode();
			geode->addDrawable(m_polyGeom.get());

			// set up the camera to render the textured quad
			osg::CameraNode* camera = new osg::CameraNode;

			// just inherit the main cameras view
			camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			camera->setViewMatrix(osg::Matrix::identity());
			camera->setProjectionMatrixAsOrtho2D(0,1280,0,1024);

			// set the camera to render before the main camera.
			camera->setRenderOrder(osg::CameraNode::NESTED_RENDER);

			// tell the camera to use OpenGL frame buffer object where supported.
			camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

			// attach the texture and use it as the color buffer.
			camera->attach(osg::CameraNode::COLOR_BUFFER, texture);

			// add subgraph to render
			camera->addChild(geode);

			distortionNode->addChild(camera);
		}
		return distortionNode;
	}

	void SetDistortionLevel(float distAmt)
	{
		if (distAmt < 0.0f)
			distAmt = 0.0f;
		else if (distAmt > 1.0f)
			distAmt = 1.0f;
		unsigned index = (unsigned)(NUM_DISTORTION_LEVELS * distAmt);
		if (index >= NUM_DISTORTION_LEVELS)
			index = NUM_DISTORTION_LEVELS-1;
		if (index != m_currIndex)
		{
			m_currIndex = index;
			m_polyGeom->setTexCoordArray(0,m_textCoords[m_currIndex].get());
		}
	};
};
//////////////////////////////////////////////////////////////////////////

Camera::Camera(osgViewer::Viewer* camGroup) : m_camGroup(camGroup)
{
	ASSERT(m_camGroup.valid());
	// Default to a black background
	m_camGroup->getCamera()->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
	m_compositePostDrawCallback = new Camera::CompositeDrawCallback;

	// Work with a larger near/far ratio
	m_camGroup->getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
	m_camGroup->getCamera()->setNearFarRatio(0.00001f);
	m_camGroup->getCamera()->setFinalDrawCallback(m_compositePostDrawCallback.get());
}

//////////////////////////////////////////////////////////////////////////

void Camera::setFinalDrawCallback(osg::Camera::DrawCallback* cb)
{
	m_camGroup->getCamera()->setFinalDrawCallback(cb);
}
//////////////////////////////////////////////////////////////////////////


void Camera::setClearColor(const osg::Vec4& c)
{
	m_camGroup->getCamera()->setClearColor(c);
}

void Camera::getClearColor(osg::Vec4& c)
{
	c = m_camGroup->getCamera()->getClearColor();
}

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
void Camera::SetSceneNode(osg::Node* node, float distortionPCNT)
{
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

	m_distortion = NULL;
	if (distortionPCNT > 0.0)
	{
		m_distortion = new DistortionSubgraph();
		osg::Vec4 color; getClearColor(color);
		m_camGroup->setSceneData(m_distortion->createDistortionSubgraph(node, color, distortionPCNT) );
	}
	else
		m_camGroup->setSceneData(node);

	// this might be a task that could take some time, lets fire a task complete to keep the times updated
	GG_Framework::UI::OSG::LoadStatus::Instance.TaskComplete();
}

void Camera::SetMatrix(const osg::Matrix& camMatrix, float distortionAmt)
{
	{
		m_camGroup->getCamera()->setViewMatrix(camMatrix);
		MatrixUpdate.Fire(camMatrix);
		if (m_distortion)
			m_distortion->SetDistortionLevel(distortionAmt);
	}
}
//////////////////////////////////////////////////////////////////////////

osg::Matrix Camera::GetCameraMatrix() const
{
	return m_camGroup->getCamera()->getViewMatrix();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
