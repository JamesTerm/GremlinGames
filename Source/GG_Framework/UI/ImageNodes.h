// GG_Framework.UI ImageNodes.h
#pragma once
#include <osg/MatrixTransform>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API ImageNode : public osg::Group
		{
		public:
			//! Pass in the full line from the file, including the indicator
			ImageNode(char indicator, const char* params);
		};
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////


		//! This parent node is used to point scene graphs at an object (hopefully not
		//! a child node in the scene graph, this would cause some strange effects).
		//! The target may be the "CAMERA".
		class FRAMEWORK_UI_API PointAtNode : public osg::MatrixTransform
		{
		public:
			//! Pass in "T' or "F" for orienting with camera up (typically true), and then 
			//! the name of the node to look for, (or "CAMERA")
			PointAtNode(ActorScene* actorScene, std::string params);

			//! This callback is where the update happens to point at the target (or the CAMERA)
			void CameraUpdateCallback(const osg::Matrix& camMatrix);

		private:
			//! m_targetNodeName holds the name of the target only until the first callback
			std::string m_targetNodeName;

			//! m_targetNode gets assigned during the first callback, or stays NULL 
			//! if m_targetNodeName is assigned to "CAMERA" in the c'tor
			osg::ref_ptr<osg::Node> m_targetNode;

			//! The handler list for event handling
			IEvent::HandlerList ehl;

			//! Do we want to alight the 'up' vector with the camera's 'UP'?
			bool m_useCameraUp;
			ActorScene* const m_actorScene;
		};
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////

		//! This parent node is used to orient an object's attitude in line with another object.
		//! Typically this would be the camera, and should be good for 
		//! Images used as transparencies, rather than using PointAtNode, which would cause 
		//! the images to overlap.
		class FRAMEWORK_UI_API OrientWithNode : public osg::MatrixTransform
		{
		public:
			//! Pass in "T' or "F" for flipping back (180 heading, typically true), and then 
			//! the name of the node to look for, (or "CAMERA")
			OrientWithNode(ActorScene* actorScene, std::string params);

			//! This callback is where the update happens to point at the target (or the CAMERA)
			void CameraUpdateCallback(const osg::Matrix& camMatrix);

		private:
			//! m_targetNodeName holds the name of the target only until the first callback
			std::string m_targetNodeName;

			//! m_targetNode gets assigned during the first callback, or stays NULL 
			//! if m_targetNodeName is assigned to "CAMERA" in the c'tor
			osg::ref_ptr<osg::Node> m_targetNode;

			//! The handler list for event handling
			IEvent::HandlerList ehl;

			//! Do we want to alight the 'up' vector with the camera's 'UP'?
			bool m_flipHeading;
			ActorScene* const m_actorScene;
		};
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
	}
}