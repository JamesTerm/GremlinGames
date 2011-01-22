// GG_Framework.UI ActorScene.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

ActorScene::ActorScene(GG_Framework::Base::Timer& timer, const char* contentDirLW, bool ignoreKeybindings) :
	m_timer(&timer), m_fileReader(NULL), m_contentDirLW(contentDirLW), m_ignoreKeybindings(ignoreKeybindings)
{
	_scene = new osg::Group();
	_scene->setName("root");

	if (MainWindow::GetMainWindow())
		CreateReverseCameraAngleNode(MainWindow::GetMainWindow()->GetMainCamera());
}
//////////////////////////////////////////////////////////////////////////

//! ALWAYS returns at least an empty Group with warning, fn must not be NULL
//! This function is Mutex Blocked b/c we are using the CWD and a static buffer.
ActorTransform* ActorScene::ReadActorFile(GG_Framework::UI::EventMap& localEventMap, const char* fn)
{
	ASSERT(fn);
	ActorTransform* ret = NULL;
	{
		FileLineReader fileReader(m_fileReader);
		ret = ReadFromSceneFile(localEventMap, fn);
	}
	ASSERT(ret);

	// Fire a 0 time change to pass along a 0 time, to keep the AnimationPath callbacks from
	// Having the silly offset value
	osgUtil::UpdateVisitor update;
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp();
	update.setFrameStamp(frameStamp.get());
#ifndef __Use_OSG_Svn__
	frameStamp->setReferenceTime(0.0);
#else
	frameStamp->setSimulationTime(0.0);
#endif
	frameStamp->setFrameNumber(0);
	ret->accept(update);

	// TODO: Is this the best way to normalize?
	ret->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
	return ret;
}
//////////////////////////////////////////////////////////////////////////

void ActorScene::AddActor(ActorTransform* newActor)
{
	ASSERT(_scene.valid());
	_scene->addChild(newActor);
	_scene->dirtyBound();
}
//////////////////////////////////////////////////////////////////////////

void ActorScene::RemoveActor(ActorTransform* newActor)
{
	ASSERT(_scene.valid());
	_scene->removeChild(newActor);
	_scene->dirtyBound();
}
//////////////////////////////////////////////////////////////////////////

ActorScene::~ActorScene()
{
	std::list<INamedEffect*>::iterator pos;
	for (pos = m_effectList.begin(); pos != m_effectList.end(); ++pos)
	{
		INamedEffect* effect = *pos;
		effect->Delete();
	}
}
//////////////////////////////////////////////////////////////////////////

//! This looks at the first character of the line and returns whether or not 
//! the character might be an indicator (not just the first character of a filename.
//! Always returns the upper-case version.  Returns the first character only IF
//! there is not an alpha-numeric or 
char GetIndicatorCharacter(const char* line)
{
	if (line == NULL) return (char)0;
	else if (line[0] == 0) return (char)0;
	else if (line[1] <= 32)
		return (char)toupper(line[0]);
	else return (char)0;
}
//////////////////////////////////////////////////////////////////////////

ActorScene::FileLine 
ActorScene::SkipChildren(FILE* inFile, unsigned parentTabs)
{
	ActorScene::FileLine fl = m_fileReader->ReadNextLineFromFile(inFile);
	while (fl.line && fl.numTabs >= parentTabs)
		fl = m_fileReader->ReadNextLineFromFile(inFile);
	return fl;
}
//////////////////////////////////////////////////////////////////////////

ActorScene::FileLine 
ActorScene::ReadSceneGroup(GG_Framework::UI::EventMap& localEventMap, 
FILE* inFile, osg::Group* parent, unsigned parentTabs, int fileV)
{
	ASSERT_MSG(inFile, "ActorScene::ReadSceneGroup(inFile==null)");
	ASSERT_MSG(parent, "ActorScene::ReadSceneGroup(parent==null)");

	ActorScene::FileLine fl = m_fileReader->ReadNextLineFromFile(inFile);
	while (true)
	{
		// These are the things that would cause us to return.  They are things that 
		// can be a parent object themselves.
		char indicator = GetIndicatorCharacter(fl.line);
		if (!fl.line) return fl;
		else if (fl.numTabs < parentTabs) return fl;
		else if (indicator == '^')	// An actor
		{
			Actor* actor = new Actor(this, fl.line);
			parent->addChild(actor);
			fl = ReadSceneGroup(localEventMap, inFile, actor, fl.numTabs+1, fileV);
		}
		else if (indicator == '$') // A SUb-Object in the current parent
		{
			osg::ref_ptr<osg::Node> childNode = GG_Framework::UI::OSG::FindChildNode(parent, fl.line+2);
			if (!childNode.valid())
			{
				osg::notify(osg::FATAL) << "Could not find the sub-node, \"" << fl.line+2 << "\"\n";
				fl = ReadSceneGroup(localEventMap, inFile, parent, fl.numTabs+1, fileV);
			}
			else
			{
				osg::ref_ptr<Actor> newActor = new Actor(this, GG_Framework::Base::BuildString("^ %s", fl.line+2).c_str());
				childNode->getParent(0)->addChild(newActor.get());
				childNode->getParent(0)->removeChild(childNode.get());
				newActor->addChild(childNode.get());
				fl = ReadSceneGroup(localEventMap, inFile, newActor.get(), fl.numTabs+1, fileV);
			}
		}
		else if (indicator == 'S') // A Scaled Transform, used for large things like planets
		{
			if (MainWindow::GetMainWindow() && IsVisibleScene())
			{
				ScaledTransform* st = new ScaledTransform(*MainWindow::GetMainWindow()->GetMainCamera(), atof(fl.line+2));
				parent->addChild(st);
				fl = ReadSceneGroup(localEventMap, inFile, st, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == 'U')  // A Light Bloom
		{
			if (MainWindow::GetMainWindow() && IsVisibleScene())
			{
				float nomDist, farDist;
				if ((sscanf(fl.line, "U %f %f", &nomDist, &farDist) != 2) || (nomDist <= 0.0f) || (farDist <= nomDist))
				{
					printf("Error in Light Bloom Node: %s\n", fl.line);
					throw std::exception("Error in Light Bloom Node");
				}

				LightBloomTransform* st = new LightBloomTransform(*MainWindow::GetMainWindow()->GetMainCamera(), nomDist, farDist);
				parent->addChild(st);
				fl = ReadSceneGroup(localEventMap, inFile, st, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == 'C')  // An item parented to the Camera
		{
			if (MainWindow::GetMainWindow() && IsVisibleScene())
			{
				CameraParentedTransform* ct = new CameraParentedTransform(MainWindow::GetMainWindow()->GetMainCamera());
				parent->addChild(ct);
				fl = ReadSceneGroup(localEventMap, inFile, ct, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == 'L') // A Light Source
		{
			if (IsVisibleScene())
			{
				LightSource* st = new LightSource(this, fl.line+2);
				parent->addChild(st);
				fl = ReadSceneGroup(localEventMap, inFile, st, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if ((indicator == 'I') || (indicator == 'B')) // An Image or Billboard node
		{
			if (IsVisibleScene())
			{
				ImageNode* imageNode = new ImageNode(indicator, fl.line+2);
				parent->addChild(imageNode);
				fl = ReadSceneGroup(localEventMap, inFile, imageNode, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == 'P') // This node points all of its children at something else
		{
			//! \todo There may be times when a non visible scene wants a Point at, but we are now basing this on the
			//! Camera position.  We ned to change the event structure to a node visitor to make it work better
			if (IsVisibleScene())
			{
				PointAtNode* pnt = new PointAtNode(this, fl.line+2);
				parent->addChild(pnt);
				fl = ReadSceneGroup(localEventMap, inFile, pnt, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == 'O') // This kind of node can orient itself with another node
		{
			//! \todo There may be times when a non visible scene wants a Point at, but we are now basing this on the
			//! Camera position.  We ned to change the event structure to a node visitor to make it work better
			if (IsVisibleScene())
			{
				OrientWithNode* own = new OrientWithNode(this, fl.line+2);
				parent->addChild(own);
				fl = ReadSceneGroup(localEventMap, inFile, own, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == '-') // Performance switch nodes
		{
			// We only care about these if there is a visible scene
			if (IsVisibleScene())
			{
				// Make sure the parent is a Switch node
				osg::Switch* parentSwitch = dynamic_cast<osg::Switch*>(parent);
				if (!parentSwitch)
				{
					parentSwitch = new osg::Switch;
					parent->addChild(parentSwitch);
				}

				// Find the display ranges
				int minPerf, maxPerf;
				if (sscanf(fl.line, "- %i %i", &minPerf, &maxPerf) != 2)
				{
					printf("Error in Performance Switch node: %s\n", fl.line);
					throw std::exception("Error in Performance Switch Node");
				}
				if (maxPerf < minPerf)
				{
					printf("Error in Performance Switch node: %s\n", fl.line);
					throw std::exception("Error in Performance Switch Node");
				}

				// Create a new group to add for this Performance Switch
				PerformanceSwitch* perfGroup = new PerformanceSwitch(minPerf, maxPerf);
				parentSwitch->addChild(perfGroup);
				int initIndex = GG_Framework::UI::MainWindow::PERFORMANCE_INIT_INDEX;
				perfGroup->PerformanceIndexChanged(initIndex,initIndex);	//! Call the first time to show/hide properly

				// Keep reading the children
				fl = ReadSceneGroup(localEventMap, inFile, perfGroup, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else if (indicator == '+') // LOD nodes
		{
			// We only care about these if there is a visible scene
			if (IsVisibleScene())
			{
				// Make sure the parent is a LOD node
				osg::LOD* parentLOD = dynamic_cast<osg::LOD*>(parent);
				if (!parentLOD)
				{
					parentLOD = new osg::LOD;
					parent->addChild(parentLOD);
				}

				// Find the display ranges
				float minRange, maxRange;
				if (sscanf(fl.line, "+ %f %f", &minRange, &maxRange) != 2)
				{
					printf("Error in LOD node: %s\n", fl.line);
					throw std::exception("Error in LOD Node");
				}
				if ((minRange < 0) || (maxRange < minRange))
				{
					printf("Error in LOD node: %s\n", fl.line);
					throw std::exception("Error in LOD Node");
				}

				// Create a new group to add for this LOD
				osg::Group* lodGroup = new osg::Group;
				parentLOD->addChild(lodGroup, minRange, maxRange);

				// Keep reading the children
				fl = ReadSceneGroup(localEventMap, inFile, lodGroup, fl.numTabs+1, fileV);
			}
			else
				fl = SkipChildren(inFile, fl.numTabs+1);
		}
		else
		{
			// These cannot be parents, but are rather modifiers to the existing parent,
			// OR Global Modifiers, like key bindings,
			// OR a simple file read in as a node, like a LWS or LWO or another OSGV file
			IScriptOptionsLineAcceptor* scriptAcc =  dynamic_cast<IScriptOptionsLineAcceptor*>(parent);
			if ((scriptAcc != NULL) && (scriptAcc->AcceptScriptLine(localEventMap, indicator, fl.line)))
			{
				// Nothing else to do, the script was accepted
			}		
			else if (indicator == '%')	// A Key Binding, only used for testing in OSGV
			{
				if (!m_ignoreKeybindings)
				{
					char eventType[16];
					char eventName[64];
					char key;
					double minRange=0.0, maxRange=0.0;
					
					// OOH!  Watch for the '%'
					if (sscanf(fl.line+2, "%15s %c %63s %lf %lf", eventType, &key, eventName, &minRange, &maxRange) < 3)
					{
						printf("Error in Key Binding: %s\n", fl.line);
						throw std::exception("Error in Key Binding");
					}

					// Watch for Range kinds of events that fire off secondary events on a timer
					if (maxRange > minRange)
					{
						// We are working with testing Mapped events.  See MapFramesEffect.txt
						MapFramesEffect::TestKeyBinding::Add(key, localEventMap, GetTimer(), eventName, minRange, maxRange);
					}
					else
					{
						// The default handler
						MainWindow::GetMainWindow()->GetKeyboard_Mouse().AddKeyBinding(key, eventName, ((eventType[0]=='w')||(eventType[0]=='W')));
					}
				}
			}
			else if (indicator == '!')
			{
				// Effects only with a visible scene, Unless the script says otherwise with !!
				if (IsVisibleScene() || (fl.line[2] == '!'))
				{
					INamedEffect* effect = INamedEffectClassFactory::CreateEffect(this, localEventMap, *parent, fl.line + ((fl.line[2] == '!')?3:2));
					if (effect)
						m_effectList.push_back(effect);
				}
			}
			else
			{
				parent->addChild(ReadFromSceneFile(localEventMap, fl.line));
			}

			// Read the next line for the next loop
			fl = m_fileReader->ReadNextLineFromFile(inFile);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

//! ALWAYS returns at least an empty Group with warning, inFile must not be NULL
ActorTransform* ActorScene::ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, FILE* inFile, const char* rootName)
{
	ASSERT_MSG(inFile, "ActorScene::ReadFromSceneFile(inFile==null)");
	ActorScene::FileLine fl = m_fileReader->ReadNextLineFromFile(inFile);

	if (fl.line[0] == '~')
	{
		int version = 0;
		sscanf(fl.line, "~osgv version %i", &version);
		ActorTransform* root = new ActorTransform;
		ASSERT(rootName);
		root->setName(rootName);
		ReadSceneGroup(localEventMap, inFile, root, 0, version);
		return root;
	}
	else
		return ReadFromSceneFile(localEventMap, fl.line);
}
//////////////////////////////////////////////////////////////////////////

ActorScene::FileLine ActorScene::FileLineReader::ReadNextLineFromFile(FILE* inFile)
{
	ASSERT_MSG(inFile, "ActorScene::m_fileReader->ReadNextLineFromFile(inFile==null)");

	// Read in to a static buffer
	unsigned numTabs = 0;
	char* ret = fgets(buff, 1024, inFile);
	if (!ret) return ActorScene::FileLine(ret, numTabs);

	// Strip the end of line characters and comments and trailing whitespace
	StripCommentsAndTrailingWhiteSpace(ret);

	// Count the number of tabs
	while (ret[0] == '\t')
		++ret;
	numTabs = (unsigned)(ret-buff);

	// Skip over other whitespace
	while (ret[0] == ' ')
		++ret;

	if (ret[0]) return ActorScene::FileLine(ret, numTabs);
	else return ReadNextLineFromFile(inFile);
}
//////////////////////////////////////////////////////////////////////////
#undef DEBUG_FILE_LOADING
#ifdef DEBUG_FILE_LOADING
#define DEBUG_CWD printf
#else
#define DEBUG_CWD osg::notify(osg::NOTICE) << GG_Framework::Base::BuildString
#endif
//! This class sets the CWD, but remembers the old one so it gets set back automatically
//! It sets the CWD from a file using SetCWD_FromSceneFile.
class CWD_Wrapper
{
public:
	CWD_Wrapper(const char* newCD)
	{
		s_mutex.lock();
		_getcwd(m_prevCurrWD, 512);
		std::string newCWDCS;
		if (strrchr(newCD, ':') == NULL)
		{
			// Local path, append it to the current CWD
			newCWDCS = BuildString("%s\\%s", m_prevCurrWD, newCD);
			newCD = newCWDCS.c_str();
		}
		_chdir(newCD);
		DEBUG_CWD("CTOR _chdir: \"%s\"\n", newCD);
	}
	~CWD_Wrapper()
	{
		_chdir(m_prevCurrWD);
		DEBUG_CWD("DTOR _chdir: \"%s\"\n", m_prevCurrWD);
		s_mutex.unlock();
	}
private:
	char m_prevCurrWD[512];
	static OpenThreads::Mutex s_mutex;
};
OpenThreads::Mutex CWD_Wrapper::s_mutex;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


//! ALWAYS returns at least an empty Group with warning, fn must not be NULL
ActorTransform* ActorScene::ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, const char* fn)
{
	static OpenThreads::Mutex ReadFromSceneFile_Mutex;
	GG_Framework::Base::RefMutexWrapper m(ReadFromSceneFile_Mutex);

	// Use Notification to show what is being loaded
	ASSERT(fn);
#ifdef DEBUG_FILE_LOADING
	printf("ActorScene::ReadFromSceneFile(%s)\n", fn);
#endif

	// See what kind of file this thing is
	const char* ext = strrchr(fn, '.');
	if ((ext != NULL) && ((!stricmp(ext, ".txt")) || (!stricmp(ext, ".osgv"))))
	{
		// Open the file before setting the CWD
		FILE* inFile = fopen(fn, "r");

		if (inFile)
		{
			osg::notify(osg::NOTICE) << GG_Framework::Base::BuildString(
				"Reading OSGV file \"%s\"\n", fn);
		}
		else
		{
			char CurrWD[512];
			_getcwd(CurrWD, 512);
			osg::notify(osg::FATAL) << GG_Framework::Base::BuildString(
				"UI::ActorScene::ReadFromSceneFile unable to open file \"%s\\%s\", using empty GROUP\n", CurrWD, fn);
			return new ActorTransform;
		}

		// Set the CWD based on this sceneFile.  CWD_Wrapper wraps a static mutex
		// So this section is all Thread safe from here down.
		std::string parentDir = osgDB::getFilePath(fn);
		CWD_Wrapper cwd(parentDir.c_str());

		ActorTransform* ret = ReadFromSceneFile(localEventMap, inFile, fn);
		fclose(inFile);
			
		ASSERT(ret);
		return ret;
	}
	else
	{
		// We want to get the FULL path for the node file, based on the CWD, which is where the current OSGV file is
		std::string fullPath(fn);
		if (fn[1] != ':')	// Watch for a relative path
		{
			char prevCurrWD[512];
			_getcwd(prevCurrWD, 512);
			fullPath = std::string(prevCurrWD) + std::string("/") + fullPath;
		}


		// Then temporarily set the CWD back to the original CWD, so when LWS reads, it can see the proper CWD
		CWD_Wrapper cwdWrapper(m_contentDirLW);

		osg::notify(osg::NOTICE) << GG_Framework::Base::BuildString("Reading \"%s\"\n", fn);

		osg::Node* nodeFromFile = GG_Framework::UI::OSG::readNodeFile(fullPath);
		if (nodeFromFile)
		{
			// Fire a 0 time change to pass along a 0 time, to keep the AnimationPath callbacks from
			// Having the silly offset value
			osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
			osgUtil::UpdateVisitor update;
			update.setFrameStamp( frameStamp.get() );
			frameStamp->setSimulationTime(0.0);
			frameStamp->setFrameNumber(0);
			nodeFromFile->accept(update);

			ActorTransform* actorParent = new ActorTransform;
			actorParent->addChild(nodeFromFile);
			actorParent->setName(fn);
			return actorParent;
		}
		else
		{
			osg::notify(osg::WARN) << GG_Framework::Base::BuildString(
				"UI::ActorScene::ReadFromSceneFile unable to load \"%s\", using empty ActorTransform\n", fn);
			return new ActorTransform;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void ActorScene::CreateReverseCameraAngleNode(GG_Framework::UI::OSG::ICamera* cam)
{
	CameraParentedTransform* ct = new CameraParentedTransform(cam);
	_scene->addChild(ct);
	Actor* camPtrOffset = new Actor(this, "^ CAM_PTR_OFFSET 0 0 1");
	ct->addChild(camPtrOffset);
	PointAtNode* pnt = new PointAtNode(this, "T CAMERA");
	camPtrOffset->addChild(pnt);
	Actor* camPtr = new Actor(this, "^ CAM_POINTER");
	pnt->addChild(camPtr);
}
//////////////////////////////////////////////////////////////////////////
