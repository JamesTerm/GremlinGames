// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/SceneRoot.h,v 1.2 2004/05/21 17:38:11 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __SceneRoot_Included_
#define __SceneRoot_Included_

#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#include <vector>
#include <string>

class SceneRoot : public osg::Group
{
public:
	SceneRoot();
	osg::Group* GetTargetGroup(){return m_targetGroup.get();}
	osg::Node*	GetLandNode(){return m_landNode.get();}

	osg::PositionAttitudeTransform* GetSelTargetTrans(){return m_selTargetTrans.get();}
	osg::PositionAttitudeTransform* GetSelTargetFlag(){return m_selTargetFlag.get();}

	static std::vector<osg::PositionAttitudeTransform*>	s_transList;
	static std::vector<std::string>						s_fileNameList;
	static osg::Vec3									s_cameraPosition;
	static std::string									s_targetConfigFile;

protected:
	virtual ~SceneRoot();

	// Helper functions
	void ReadTargets(osg::Group* scene);

private:
	osg::ref_ptr<osg::Group>						m_targetGroup;
	osg::ref_ptr<osg::PositionAttitudeTransform>	m_selTargetTrans;
	osg::ref_ptr<osg::PositionAttitudeTransform>	m_selTargetFlag;
	osg::ref_ptr<osg::Node>							m_landNode;
};

#endif	// __SceneRoot_Included_
/////////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: SceneRoot.h,v $
// Revision 1.2  2004/05/21 17:38:11  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.1  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//