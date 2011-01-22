// FrameWork.OSG Misc.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

osg::Node* FindParent(osg::Node* node, osg::Node* uniqueParent)
{
	if (!node)
		return node;
	if (uniqueParent == node)
		return node;
	
	unsigned numParents = node->getNumParents();
	if (!uniqueParent && numParents)
		return node->getParent(0);	// Always return the first parent if there are none to specify
	for (unsigned i = 0; i < numParents; ++i)
	{
		osg::Node* potentialPArent = FindParent(node->getParent(i), uniqueParent);
		if (potentialPArent)
			return potentialPArent;
	}
	// No parents were found
	return NULL;
}
//////////////////////////////////////////////////////////////////////////

//! Will create a node path and take into account a node that may have multiple parents
//! Use a topParent if you ONLY want the node path to only go so far (like finding a point only to an entity)
void FillNodePath(osg::NodePath& nodePath, osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent)
{
	// Base cases for the recursion
	if (!node) return;
	if (node == topParent) return;
	if (node == uniqueParent)
		uniqueParent = NULL;	// We do not need to remember it

	// Find the only parent
	osg::Node* parent = FindParent(node, uniqueParent);

	// Recursively call on parent
	FillNodePath(nodePath, parent, uniqueParent, topParent);

	// Add my own node
	nodePath.push_back(node);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

class NodePathMap
{
private:
	OpenThreads::Mutex m_mutex;
	std::map<osg::Node*, std::map<osg::Node*, std::map<osg::Node*, osg::NodePath*> > > Map;
public:
	NodePathMap(){}
	~NodePathMap()
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
		std::map<osg::Node*, std::map<osg::Node*, std::map<osg::Node*, osg::NodePath*> > >::iterator it1;
		for (it1 = Map.begin(); it1 != Map.end(); ++it1)
		{
			std::map<osg::Node*, std::map<osg::Node*, osg::NodePath*> >& map2 = (*it1).second;
			std::map<osg::Node*, std::map<osg::Node*, osg::NodePath*> >::iterator it2;
			for (it2 = map2.begin(); it2 != map2.end(); ++it2)
			{
				std::map<osg::Node*, osg::NodePath*>& map3 = (*it2).second;
				std::map<osg::Node*, osg::NodePath*>::iterator it3;
				for (it3 = map3.begin(); it3 != map3.end(); ++it3)
					delete (*it3).second;
			}
		}
	}

	osg::NodePath* FindNodePath(osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent)
	{
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
		osg::NodePath* ret = Map[node][uniqueParent][topParent];
		if (!ret)
		{
			ret = new osg::NodePath;
			FillNodePath(*ret, node, uniqueParent, topParent);
			Map[node][uniqueParent][topParent] = ret;
		}
		return ret;
	}
};

NodePathMap GlobalNodePath;

//! We are going to cache the Node paths, since we use them SOOO often
//! We will use a three dimensional map, <node*, uniqueParent*, topParent*>
//! This map will be global, so we need to block for it

//////////////////////////////////////////////////////////////////////////

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{

osg::Matrix GetNodeMatrix(osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent)
{
	osg::NodePath* nodePath = GlobalNodePath.FindNodePath(node, uniqueParent, topParent);
	return computeLocalToWorld(*nodePath, true);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//! This helper function finds the current position of a Node
osg::Vec3 GetNodePosition(osg::Node* node, osg::Node* uniqueParent, osg::Node* topParent)
{
	osg::Matrix M = GetNodeMatrix(node, uniqueParent, topParent);
	return M.getTrans();
}
///////////////////////////////////////////////////////////////////////////////////////////////////

//! This helper function is used to grab the PYTHAGOREAN distance between two points
double GetDistance(const osg::Vec3& A, const osg::Vec3& B)
{
	double dX = A[0] - B[0];
	double dY = A[1] - B[1];
	double dZ = A[2] - B[2];

	return sqrt(dX*dX + dY*dY + dZ*dZ);
}
///////////////////////////////////////////////////////////////////////////////////////////////////

osg::Node* FindChildNode(osg::Node* parent, std::string name)
{
	if (!parent) return NULL;

	// Watch for finding the parent of a specific node if the first char is a '^'
	if (name[0] == '^')
	{
		osg::Node* retChild = FindChildNode(parent, name.substr(1));
		return retChild ? retChild->getParent(0) : NULL;
	}
	if (parent->getName() == name)
		return parent;
	osg::Group* parentGroup = parent->asGroup();
	if (parentGroup)
	{
		unsigned numChildren = parentGroup->getNumChildren();
		for (unsigned up = 0; up < numChildren; ++up)
		{
			osg::Node* foundChild = FindChildNode(parentGroup->getChild(up), name);
			if (foundChild)
				return foundChild;
		}
	}
	return NULL;
}
//////////////////////////////////////////////////////////////////////////

void GetMaxAnimTime(osg::Node* node, double& maxTime)
{
	osg::NodeCallback* ncb = node ? node->getUpdateCallback() : NULL;
	osg::AnimationPathCallback* apcb = dynamic_cast<osg::AnimationPathCallback*>(ncb);
	osg::AnimationPath* ap = apcb ? apcb->getAnimationPath() : NULL;
	if (ap)
	{
		double lt = ap->getLastTime();
		if (maxTime < lt) maxTime = lt;
	}
	osg::Group* group = node ? node->asGroup() : NULL;
	if (group)
	{
		for (unsigned up = 0; up < group->getNumChildren(); ++up)
			GetMaxAnimTime(group->getChild(up), maxTime);
	}
}
//////////////////////////////////////////////////////////////////////////

osg::Matrix MimicCameraRot(osg::Vec3 lookCenter, osg::Vec3 up)
{
	double cLen = lookCenter.normalize();
	double uLen = up.normalize();
	if ((cLen == 0.0) || (uLen == 0.0))	// Watch for a 0 length
	{
		lookCenter = osg::Vec3(0,1,0);
		up = osg::Vec3(0,0,1);
	}

	osg::Vec3 rt(lookCenter^up);
	rt.normalize();
	up = rt^lookCenter;

	osg::Matrix 
		M = osg::Matrix(	rt[0],		rt[1],		rt[2],		0,
							lookCenter[0],	lookCenter[1],	lookCenter[2],	0,
							up[0],		up[1],		up[2],		0,
							0,			0,			0,			1);
	return M;
}
//////////////////////////////////////////////////////////////////////////

// Properly rotates the Y axis towards the vector V
osg::Matrix CreateRot(osg::Vec3 lookCenter)
{
	// This works by finding the 'up' vector based on the look at vector,
	// then passing to MimicCameraRot
	double cLen = lookCenter.normalize();
	if (cLen == 0.0)	// Watch for a 0 length
		lookCenter = osg::Vec3(0,1,0);

	// We are trying to make the 'up' vector basically point +Z
	osg::Vec3 up;
	if (lookCenter[2] == 0.0)	// Watch for simple case of being in XY plane
		up = osg::Vec3(0,0,1);		// 'up' is just +Z
	else if ((lookCenter[0] == 0.0) && (lookCenter[1] == 0.0)) // Watch for pointing straight up or down
		up = osg::Vec3(1,0,0);	// Arbitrarily point to +X
	else // Find the rt vector along the xy plane and cross with that
	{
		double x = lookCenter[0];
		double y = lookCenter[1];
		osg::Vec3 rt(y, -x, 0.0);	// Will be normalized in MimicCameraRot
		up = rt^lookCenter;
	}
	return MimicCameraRot(lookCenter, up);
}
//////////////////////////////////////////////////////////////////////////

osg::Geometry* CreateSquare(const osg::Vec3& corner,const osg::Vec3& width,const osg::Vec3& height, osg::Image* image)
{
	// Watch for there being no image
	if (!image)
		return NULL;

	// set up the Geometry.
	osg::Geometry* geom = osg::createTexturedQuadGeometry(corner,width,height);
	osg::Texture2D *tex = new osg::Texture2D;
	tex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::MIRROR );
	tex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::MIRROR );
	tex->setImage(image);
	geom->getOrCreateStateSet()->setTextureAttributeAndModes(0,tex,osg::StateAttribute::ON);

	return geom;
}
//////////////////////////////////////////////////////////////////////////
//I have temporarily left my caching technique here as it is currently the only point of reference in case we want to do any higher
//level caching elsewhere.  It turns out the osg will cache these images implicitly (provided that it is working correctly)
//I may want to remove this example if there can be another place that can use it.
//  [3/20/2009 James]
#undef __TestOurCache__

#ifdef __TestOurCache__
#include "../../Base/FileSharedPointer.h"
typedef osg::ref_ptr<osg::Image> Image_ptr;
static UtilityCode::FileSharedPointer<osg::Image> s_ImageDataBase;
#endif

OpenThreads::Mutex OSGDB_Mutex;

#ifndef __TestOurCache__
osg::Image* readImageFile(std::string filename)
{
	GG_Framework::Base::RefMutexWrapper m(OSGDB_Mutex);
	osg::Image* ret = osgDB::readImageFile(filename);
	LoadStatus::Instance.TaskComplete();
	return ret;
}
//////////////////////////////////////////////////////////////////////////

#else
osg::Image* readImageFile(std::string filename)
{
	Image_ptr MyPointer(s_ImageDataBase.GetSharedPointer(filename.c_str()));
	{	//read (and possibly write) to the shared pointer
		OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(*MyPointer->getRefMutex());
		if (!MyPointer.get()->valid())
			MyPointer = osgDB::readImageFile(filename);
	}
	LoadStatus::Instance.TaskComplete();
	return MyPointer.get();
}
#endif

/// Pass in the Cached Node. This function will clone the parts of the tree that need to be cloned
osg::Node* CloneOrCache(osg::Node* node)
{
	// See if this is a group
	osg::Group* group = node ? node->asGroup() : NULL;
	if (group)
	{
		// Create a clone of this group (Not sure how to do this part
		// We also need to make sure that all of the animation path callbacks and such happen here
		osg::Group *clonedGroup = new osg::Group(*group);

		// Loop through all of the children
		unsigned numChildren = group->getNumChildren();
		for (unsigned i = 0; i < numChildren; ++i)
		{
			// Recursively replace children
			clonedGroup->replaceChild(clonedGroup->getChild(i),CloneOrCache(group->getChild(i)));
		}

		return clonedGroup;
	}
	else
	{
		// We do not want to clone this one, just return what is in the cache
		return node;
	}
}


osg::Node*  readNodeFile(std::string filename)
{
	GG_Framework::Base::RefMutexWrapper m(OSGDB_Mutex);
	osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;

	// These are options the LWO reader gives us.  We need to play with how to apply them
	{
		typedef osgDB::ReaderWriter::Options Options;
		//This is currently a work in progress... once node cloning is working we can use CACHE_ALL
#if 1
		options->setObjectCacheHint(
			(Options::CacheHintOptions)(Options::CACHE_IMAGES|Options::CACHE_ARCHIVES));
#else
		options->setObjectCacheHint((Options::CacheHintOptions)(Options::CACHE_ALL));
#endif
		// If we are using CloneOrCache, we could do a full cache here, including all nodes.
		// As a matter of fact, I hope this is being cached somewhere, because we are not even keeping
		// A reference to any of it anymore.  I hope you trust OSG's caching!  ;)
	}

	options->setOptionString("NO_LIGHTMODEL_ATTRIBUTE USE_OSGFX COMBINE_GEODES");
	osg::Node* ret = osgDB::readNodeFile(filename, options.get());
	LoadStatus::Instance.TaskComplete();
	
	return ret;
	//return CloneOrCache(ret);	// WE are assuming the ret is in the cache now
}
//////////////////////////////////////////////////////////////////////////

void SetNodeToDrawOnTop(osg::Node* node)
{
	ASSERT_MSG(node, "NULL parameter: SetNodeToDrawOnTop(NULL)");
	osg::StateSet* dstate = node->getOrCreateStateSet();
	osg::Depth* depth = new osg::Depth;
	depth->setFunction(osg::Depth::ALWAYS);
	depth->setRange(1.0,1.0);   
	dstate->setAttributeAndModes(depth,osg::StateAttribute::ON );
	dstate->setRenderBinDetails(1,"DepthSortedBin");
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void Vec3Logger::SetLogger(GG_Framework::Base::FrameLogger& fl)
{
	fl.TimerEvent.Subscribe(ehl, *this, &GG_Framework::UI::OSG::Vec3Logger::TimerEvent);
	fl.WriteTitles.Subscribe(ehl, *this, &GG_Framework::UI::OSG::Vec3Logger::WriteTitles);
	fl.WriteValues.Subscribe(ehl, *this, &GG_Framework::UI::OSG::Vec3Logger::WriteValues);
}
//////////////////////////////////////////////////////////////////////////

void Vec3Logger::WriteTitles(FILE* logFile)
{
	// Write our name to the log file
	fprintf(logFile,",%s", m_itemName.c_str());
}

void Vec3Logger::TimerEvent(double lastTime, double currTime)
{
	// Remember our current value
	m_valSet.push_back(V);
}
void Vec3Logger::WriteValues(FILE* logFile, double lastTime, double currTime)
{
	fprintf(logFile,",%0.5f %0.5f %0.5f", m_valSet[m_writeIndex][0], m_valSet[m_writeIndex][1], m_valSet[m_writeIndex][2]);
	++m_writeIndex;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

		}
	}
}