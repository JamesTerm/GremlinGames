#include "StdAfx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

namespace GG_Framework
{
	namespace UI
	{

ImageNode::ImageNode(char indicator, const char* params)
{ 
	ASSERT(params && params[0]);
	
	float width, height;
	char useAlphaBlend;
	char quadTexture;
	osg::Group* newGroup = new osg::Group;
	bool allIsWell = false;
	if (sscanf(params, "%f %f %c %c", &width, &height, &useAlphaBlend, &quadTexture) == 4)
	{
		const char* firstBlock = strchr(params, '[');
		const char* endBlock = strchr(params, ']');
		if (firstBlock && endBlock && (endBlock>firstBlock))
		{
			char filename[512];
			strncpy(filename, firstBlock+1, endBlock-firstBlock);
			filename[endBlock-firstBlock-1] = 0;
			osg::ref_ptr<osg::Image> image = GG_Framework::UI::OSG::readImageFile(filename);
			if (image.valid())
			{
				osg::ref_ptr<osg::Geode> geode;
				if (indicator == 'B')
				{	// Make a Billboard version
					osg::Billboard* bb = new osg::Billboard();
					bb->setMode(osg::Billboard::POINT_ROT_EYE);
					geode = bb;
				}
				else
					geode = new osg::Geode;

				if (quadTexture=='q')
				{
					geode->addDrawable(GG_Framework::UI::OSG::CreateSquare(
						osg::Vec3(0, 0, 0),
						osg::Vec3(width/2.0, 0, 0),
						osg::Vec3(0, 0, height/2.0),
						image.get()));
					geode->addDrawable(GG_Framework::UI::OSG::CreateSquare(
						osg::Vec3(0, 0, 0),
						osg::Vec3(-width/2.0, 0, 0),
						osg::Vec3(0, 0, height/2.0),
						image.get()));
					geode->addDrawable(GG_Framework::UI::OSG::CreateSquare(
						osg::Vec3(0, 0, 0),
						osg::Vec3(width/2.0, 0, 0),
						osg::Vec3(0, 0, -height/2.0),
						image.get()));
					geode->addDrawable(GG_Framework::UI::OSG::CreateSquare(
						osg::Vec3(0, 0, 0),
						osg::Vec3(-width/2.0, 0, 0),
						osg::Vec3(0, 0, -height/2.0),
						image.get()));
				}
				else
				{
					geode->addDrawable(GG_Framework::UI::OSG::CreateSquare(
						osg::Vec3(width/2, 0, height/2),
						osg::Vec3(-width, 0, 0),
						osg::Vec3(0, 0, -height),
						image.get()));
				}
				geode->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
				geode->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

				if ((toupper(useAlphaBlend) == 'T') || (toupper(useAlphaBlend) == 'A'))
				{
					osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
					osg::BlendFunc::BlendFuncMode destMode = (toupper(useAlphaBlend) == 'A') ? 
						osg::BlendFunc::ONE : osg::BlendFunc::ONE_MINUS_SRC_ALPHA;
					bf->setFunction(osg::BlendFunc::SRC_ALPHA, destMode);
					osg::StateSet* ss = geode->getOrCreateStateSet();
					ss->setAttributeAndModes(bf.get());
					ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
				}

				this->addChild(geode.get());
				allIsWell = true;
			}
			else
				osg::notify(osg::FATAL) << "Failed To Read Image: " << filename << std::endl;
		}
	}

	if (!allIsWell)
		DebugOutput("Error Reading I Line: %s", params);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

PointAtNode::PointAtNode(ActorScene* actorScene, std::string params) : m_actorScene(actorScene)
{
	ASSERT(params.length() > 2);

	// Do we want to align with the camera's up vector?
	m_useCameraUp = GG_Framework::Base::ParseBooleanFromChar(params[0]);

	// The name of the target we want to point at
	m_targetNodeName = std::string(params.substr(2));

	// We are going to perform our point at when the camera is updated
	if (GG_Framework::UI::MainWindow::GetMainWindow())
		GG_Framework::UI::MainWindow::GetMainWindow()->GetMainCamera()->MatrixUpdate.Subscribe(ehl, *this, &PointAtNode::CameraUpdateCallback);

	// We do not know for sure that the targetNodeName is in the scene
	// Wait until the first callback to get it
	m_targetNode = NULL;
}
//////////////////////////////////////////////////////////////////////////

void PointAtNode::CameraUpdateCallback(const osg::Matrix& camMatrix)
{
	// The first time this is called, we can find the node, IF it is not "CAMERA"
	if (m_targetNodeName != "")
	{
		if (m_targetNodeName != "CAMERA")
		{
			// Find the node in the scene graph
			m_targetNode = GG_Framework::UI::OSG::FindChildNode(
				m_actorScene->GetScene(), m_targetNodeName);
			if (m_targetNode.valid())
				DebugOutput("PointAtNode found node %s", m_targetNodeName.c_str());
			else
			{
				osg::notify(osg::WARN) << "WARNING: PointAtNode could not find node - " << m_targetNodeName << std::endl;
				osg::notify(osg::WARN) << "     Will Point at CAMERA by default." << std::endl;
			}
		}
		else
			m_targetNode = NULL;
		m_targetNodeName = "";
	}

	// Take the camera's position and up vector
	osg::Vec3d camPos, camCenter, camUp;
	camMatrix.getLookAt(camPos, camCenter, camUp);

	// Find the GLOBAL matrix of the target, or the camera
	osg::Vec3d targetPos;
	if (m_targetNode.valid())
		targetPos = GG_Framework::UI::OSG::GetNodePosition(m_targetNode.get(), NULL, NULL);
	else
		targetPos = camPos;

	// Find the GLOBAL matrix of THIS node (from the Parent's perspective)
	osg::Matrix parentMat = GG_Framework::UI::OSG::GetNodeMatrix(this->getParent(0), NULL, NULL);
	osg::Vec3d myPos = osg::Vec3d(0,0,0) * parentMat;
	osg::Matrix parentMatInverse = osg::Matrix::inverse(parentMat);
	targetPos = targetPos * parentMatInverse;
	
	// Set my new attitude to the rotation quaternion to point at the target
	if (targetPos.length() != 0.0)
	{
		if (m_useCameraUp)	// Use the camera's up vector
			this->setMatrix(GG_Framework::UI::OSG::MimicCameraRot(targetPos, (camUp+myPos) * parentMatInverse));
		else	// Just let it spin around a bit, generally pointing in +Z
			this->setMatrix(GG_Framework::UI::OSG::CreateRot(targetPos));
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

OrientWithNode::OrientWithNode(ActorScene* actorScene, std::string params) : m_actorScene(actorScene)
{
	ASSERT(params.length() > 2);

	// Do we want to align with the camera's up vector?
	m_flipHeading = GG_Framework::Base::ParseBooleanFromChar(params[0]);

	// The name of the target we want to orient with
	m_targetNodeName = std::string(params.substr(2));

	// We are going to perform our orient with when the camera is updated
	if (GG_Framework::UI::MainWindow::GetMainWindow())
		GG_Framework::UI::MainWindow::GetMainWindow()->GetMainCamera()->MatrixUpdate.Subscribe(ehl, *this, &OrientWithNode::CameraUpdateCallback);

	// We do not know for sure that the targetNodeName is in the scene
	// Wait until the first callback to get it
	m_targetNode = NULL;
}
//////////////////////////////////////////////////////////////////////////

void OrientWithNode::CameraUpdateCallback(const osg::Matrix& camMatrix)
{
	// The first time this is called, we can find the node, IF it is not "CAMERA"
	if (m_targetNodeName != "")
	{
		if (m_targetNodeName != "CAMERA")
		{
			// Find the node in the scene graph
			m_targetNode = GG_Framework::UI::OSG::FindChildNode(
				m_actorScene->GetScene(), m_targetNodeName);
			if (m_targetNode.valid())
				DebugOutput("PointAtNode found node %s", m_targetNodeName.c_str());
			else
			{
				osg::notify(osg::WARN) << "WARNING: PointAtNode could not find node - " << m_targetNodeName << std::endl;
				osg::notify(osg::WARN) << "     Will Point at CAMERA by default." << std::endl;
			}
		}
		else
			m_targetNode = NULL;
		m_targetNodeName = "";
	}

	// Find the GLOBAL matrix of the target, or the camera
	osg::Matrix targetMat;
	if (m_targetNode.valid())
		targetMat = GG_Framework::UI::OSG::GetNodeMatrix(m_targetNode.get(), NULL, NULL);
	else
	{
		osg::Vec3d camPos, camCenter, camUp;
		camMatrix.getLookAt(camPos, camCenter, camUp);
		targetMat = GG_Framework::UI::OSG::MimicCameraRot(camCenter-camPos, camUp-camPos);
	}


	// Find the GLOBAL matrix of THIS node (from the Parent's perspective)
	osg::Matrix parentMat = GG_Framework::UI::OSG::GetNodeMatrix(this->getParent(0), NULL, NULL);
	osg::Matrix inverseMat = osg::Matrix::inverse(parentMat);

	// Find the global positions of the target and where it is pointing
	osg::Vec3d tgt0 = osg::Vec3d(0,0,0) * targetMat;
	osg::Vec3d tgt1 = osg::Vec3d(0,1,0) * targetMat;
	osg::Vec3d tgt2 = osg::Vec3d(0,0,1) * targetMat;

	// Convert them to local space
	tgt0 = tgt0 * inverseMat;
	tgt1 = tgt1 * inverseMat;
	tgt2 = tgt2 * inverseMat;

	this->setMatrix(GG_Framework::UI::OSG::MimicCameraRot(tgt1-tgt0, tgt2-tgt0));
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
	}
}
