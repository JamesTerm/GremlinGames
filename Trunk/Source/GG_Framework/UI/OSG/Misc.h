// FrameWork.OSG Misc.h
#pragma once

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Matrix>
#include <osg/Vec3>
#include <osg/Image>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
			//! This helper function finds the current position of a Node
			FRAMEWORK_UI_OSG_API osg::Matrix GetNodeMatrix(osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent);
			FRAMEWORK_UI_OSG_API osg::Vec3 GetNodePosition(osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent);

			// Find a child Node of a specific name
			FRAMEWORK_UI_OSG_API osg::Node* FindChildNode(osg::Node* parent, std::string name);

			//! This helper function grabs the PYTHAGOREAN distance between to points
			FRAMEWORK_UI_OSG_API double GetDistance(const osg::Vec3& A, const osg::Vec3& B);

			//! Make a rotation vector based on a look at center and up vector
			//! This function is great for tying something to a camera
			FRAMEWORK_UI_OSG_API osg::Matrix MimicCameraRot(osg::Vec3 lookCenter, osg::Vec3 up);
			FRAMEWORK_UI_OSG_API osg::Matrix CreateRot(osg::Vec3 lookCenter);

			//! Loops through all animation paths recursively to find the maximum animation time node
			FRAMEWORK_UI_OSG_API void GetMaxAnimTime(osg::Node* node, double& maxTime);

			//! Create a polygon with an image mapped on it, watches for blending as well
			FRAMEWORK_UI_OSG_API osg::Geometry* CreateSquare(
				const osg::Vec3& corner,
				const osg::Vec3& width,
				const osg::Vec3& height, 
				osg::Image* image);

			// Use THESE functions to work with osgDB because it is not thread safe
			FRAMEWORK_UI_OSG_API osg::Image* readImageFile(std::string filename);
			FRAMEWORK_UI_OSG_API osg::Node*  readNodeFile(std::string filename);

			FRAMEWORK_UI_OSG_API void SetNodeToDrawOnTop(osg::Node* node);

			//! Use this to log any value on a TimerLogger and write its value
			class FRAMEWORK_UI_OSG_API Vec3Logger
			{
			public:
				Vec3Logger(std::string itemName) : m_itemName(itemName), m_writeIndex(0) {}
				osg::Vec3f V; //! Set this value to write on the next frame
				void SetLogger(GG_Framework::Base::FrameLogger& fl);

			private:
				void WriteTitles(FILE* logFile);
				void TimerEvent(double lastTime, double currTime);
				void WriteValues(FILE* logFile, double lastTime, double currTime);

				IEvent::HandlerList ehl;
				std::string m_itemName;
				std::vector<osg::Vec3f> m_valSet;
				unsigned m_writeIndex;
			};
			//////////////////////////////////////////////////////////////////////////

			__inline bool zeroRotation(const osg::Quat &A,double t=1e-5) { return fabs(A[0])<t && fabs(A[1])<t && fabs(A[2])<t && (fabs(A[3])-1.0)<t; } 
		}
	}
}