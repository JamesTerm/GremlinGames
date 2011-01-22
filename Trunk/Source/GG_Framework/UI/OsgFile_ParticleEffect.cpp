// GG_Framework.UI OsgFile_ParticleEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

void OsgFile_ParticleEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	INamedEffect::ReadParameters(localMap, remLineFromFile);
	m_osgFileNode = GG_Framework::UI::OSG::readNodeFile(remLineFromFile);
	remLineFromFile = NULL;
}

void OsgFile_ParticleEffect::LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt)
{
	if (start && !m_addedAlready)
	{
		m_addedAlready = true;
		osg::Group* topGroup = m_osgFileNode->asGroup();
		osg::ref_ptr<osg::Node> child0 = topGroup->getChild(0);
		osg::ref_ptr<osg::Node> child1 = topGroup->getChild(1);
		osg::ref_ptr<osg::Node> child2 = topGroup->getChild(2);
		topGroup->removeChildren(1,2);
		m_actorScene->AddChildNextUpdate(topGroup, GetParentNode()->asGroup());

		// The other 2 go higher up
		m_actorScene->AddChildNextUpdate(child1.get());
		m_actorScene->AddChildNextUpdate(child2.get());
	}
}