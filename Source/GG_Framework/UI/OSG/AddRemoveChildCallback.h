// GG_Framework.UI.OSG AddRemoveChildCallback.h
#pragma once
#include <osg/Group>
#include <osg/NodeCallback>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{

// this class is used to safely add actors during the Update Callback
class FRAMEWORK_UI_OSG_API AddRemoveChildCallback : public osg::NodeCallback
{
private:
	class AddRemoveNode
	{
	private:
		osg::ref_ptr<osg::Node> m_childPtr;
		osg::ref_ptr<osg::Group> m_parentPtr;
		bool m_adding;

	public:
		AddRemoveNode(osg::Node* node, bool adding, osg::Group* parent=NULL) : m_childPtr(node), m_adding(adding), m_parentPtr(parent) {}
		osg::Node* GetChildNode(){return m_childPtr.get();}
		bool IsAdding() {return m_adding;}
		osg::Group* GetParentNode(){return m_parentPtr.get();}
	};

	osg::Group* m_sceneParent;
	std::vector<AddRemoveNode> m_nodesToAddRemove;
	bool m_updateNeeded;
	OpenThreads::Mutex m_mutex;

	void AddRemoveChild(osg::Node* child, bool adding, osg::Group* parent);

protected:
	virtual ~AddRemoveChildCallback() {}

public:
	AddRemoveChildCallback(osg::Group* sceneParent);
	void AddChild(osg::Node* child, osg::Group* parent){AddRemoveChild(child, true, parent);}
	void RemoveChild(osg::Node* child, osg::Group* parent){AddRemoveChild(child, false, parent);}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);
};
		}
	}
}