// GG_Framework.UI ActorScene.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
class FRAMEWORK_UI_API ActorScene
{
public:
	ActorScene(GG_Framework::Base::Timer& timer, const char* contentDirLW, bool ignoreKeyBindings);
	~ActorScene();
	osg::Group* GetScene(){return _scene.get();}
	GG_Framework::Base::Timer* GetTimer(){return m_timer;}

	//! ALWAYS returns at least an empty Group with warning, fn must not be NULL
	ActorTransform* AddActorFile(GG_Framework::UI::EventMap& localEventMap, const char* fn)
	{
		ActorTransform* ret = ReadActorFile(localEventMap, fn);
		AddActor(ret);
		return ret;
	}

	//! Reads but does not add.  This should be pretty thread safe
	//! \todo Walk through this ti make sure it is thread safe
	ActorTransform* ReadActorFile(GG_Framework::UI::EventMap& localEventMap, const char* fn);
	void AddActor(ActorTransform* newActor);
	void RemoveActor(ActorTransform* newActor);

	//! Create a reverse camera object for "O F CAM_POINTER" nodes to point at
	void CreateReverseCameraAngleNode(GG_Framework::UI::OSG::ICamera* cam);

	virtual bool IsVisibleScene(){return true;}

protected:
	//! ALWAYS returns at least an empty Group with warning, fn must not be NULL
	virtual ActorTransform* ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, const char* fn);

private:
	const char* m_contentDirLW;
	struct FileLine
	{
		FileLine(char* l, unsigned t) : line(l), numTabs(t) {}
		char* line;
		unsigned numTabs;
	};

	//! ALWAYS returns at least an empty Group with warning, inFile must not be NULL
	ActorTransform* ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, FILE* inFile, const char* rootName);

	FileLine ReadSceneGroup(GG_Framework::UI::EventMap& localEventMap, 
		FILE* inFile, osg::Group* parent, unsigned parentTabs, int fileV);
	osg::ref_ptr<osg::Group> _scene;

	FileLine SkipChildren(FILE* inFile, unsigned parentTabs);

private:
	class FileLineReader
	{
	public:
		FileLineReader(FileLineReader*& ptr) : m_ptr(ptr) {m_ptr=this;}
		~FileLineReader(){m_ptr = NULL;}
		FileLine ReadNextLineFromFile(FILE* inFile);
	private:
		char buff[1024];
		FileLineReader*& m_ptr;
	};
	
	// Keep one of these around for a single read
	FileLineReader* m_fileReader;

	std::list<INamedEffect*> m_effectList;
	GG_Framework::Base::Timer* const m_timer;
	bool m_ignoreKeybindings;

};	// class ActorScene

	}
}