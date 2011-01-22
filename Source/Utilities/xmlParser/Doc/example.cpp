#define XML_NO_WIDE_CHAR
#include "xmlParser.h"
#include "ClassFactory.h"

#include <iostream>
#include <vector>
using namespace std;

class INodeReader
{
public:
	virtual string GetName() = 0;
	virtual void ReadFromNode(XMLNode& node) = 0;
};
FactoryMapT<INodeReader> INodeReader_ClassFactory;
//////////////////////////////////////////////////////////////////////////

class Sphere : public INodeReader
{
public:
	Sphere() { cout << "Sphere c'tor" << endl; }
	virtual string GetName(){return "Sphere";}
	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Sphere::ReadFromNode" << endl;
	}
};
ClassFactoryT<Sphere, INodeReader> Sphere_ClassFactory("Sphere", INodeReader_ClassFactory);
//////////////////////////////////////////////////////////////////////////

class Cube : public INodeReader
{
public:
	Cube() { cout << "Cube c'tor" << endl; }
	virtual string GetName(){return "Cube";}
	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Cube::ReadFromNode" << endl;
	}
};
ClassFactoryT<Cube, INodeReader> Cube_ClassFactory("Cube", INodeReader_ClassFactory);
//////////////////////////////////////////////////////////////////////////

class Camera : public INodeReader
{
public:
	Camera() { cout << "Camera c'tor" << endl; }
	virtual string GetName(){return "Camera";}
	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Camera::ReadFromNode" << endl;
	}
};
//////////////////////////////////////////////////////////////////////////

class Canvas : public INodeReader
{
public:
	Canvas() { cout << "Canvas c'tor" << endl; }
	virtual string GetName(){return "Canvas";}
	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Canvas::ReadFromNode" << endl;
	}
};
//////////////////////////////////////////////////////////////////////////

class Group : public INodeReader
{
public:
	Group() { cout << "Group c'tor" << endl; }
	virtual string GetName(){return "Group";}
	~Group()
	{
		cout << "Group d'tor" << endl;
		std::vector<INodeReader*>::iterator pos;
		for (pos = m_children.begin(); pos != m_children.end(); ++pos)
		{
			INodeReader_ClassFactory.Destroy((*pos)->GetName(), *pos);
		}
	}

	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Group::ReadFromNode" << endl;

		for (int j = 0; j < node.nChildNode(); ++j)
		{
			XMLNode childNode = node.getChildNode(j);
			INodeReader* childNodeReader = INodeReader_ClassFactory.Create(childNode.getName());
			if (childNodeReader)
			{
				childNodeReader->ReadFromNode(childNode);
				m_children.push_back(childNodeReader);
			}
			else
			{
				cout << "Could not find class factory for " << childNode.getName() << endl;
			}
		}
	}

private:
	std::vector<INodeReader*> m_children;

};
//////////////////////////////////////////////////////////////////////////

class Scene : public INodeReader
{
public:
	Scene() { cout << "Scene c'tor" << endl; }
	virtual string GetName(){return "Scene";}
	virtual void ReadFromNode(XMLNode& node)
	{
		cout << "Scene::ReadFromNode" << endl;
		canvas.ReadFromNode(node.getChildNode("Canvas"));
		camera.ReadFromNode(node.getChildNode("Camera"));
		sceneGraph.ReadFromNode(node.getChildNode("Group"));
	}

private:
	Canvas canvas;
	Camera camera;
	Group sceneGraph;
};
//////////////////////////////////////////////////////////////////////////



void DisplayNode(XMLNode& node, unsigned numTabs)
{
	for (unsigned i = 0; i < numTabs; ++i)
		cout << '\t';

	cout << node.getName() << endl;
	for (int j = 0; j < node.nChildNode(); ++j)
		DisplayNode(node.getChildNode(j), numTabs+1);
}

int main(unsigned argc, const char** argv)
{
	// show the arguments coming in
		cout << "argc = " << argc << endl;
		for (unsigned i = 0; i < argc; ++i)
			cout << argv[i] << endl;
		cout << endl;

	// Make sure we have good arguments
		if (argc != 3)
		{
			cout << "Bad arguments:  Use one of these two forms:" << endl;
			cout << "-r filename";
			cout << "-w filename";
			cout << "Use -r to read in a file.  Use -w to write out the file" << endl;
			cout << "EXITING" << endl;
		}

	// Are we reading or writing?
		if (argv[1][1] == 'r')
		{
			// We are creating a node to write out
			cout << "Reading From " << argv[2] << endl;

			// Read in the Scene node
			XMLNode sceneNode = XMLNode::parseFile(argv[2], "Scene");
			DisplayNode(sceneNode, 0);

			// Create a Scene recursively
			Scene scene;
			scene.ReadFromNode(sceneNode);

		}
		else
		{
			// We are creating a node to write out
			cout << "Writing To " << argv[2] << endl;

			// Create a sample
			XMLNode topNode = XMLNode::createXMLTopNode("Scene"); // What does the isDeclare do?
			topNode.addChild("Camera");
			topNode.addChild("Canvas");
			XMLNode sceneGraph = topNode.addChild("Group");
			sceneGraph.addChild("Sphere");
			sceneGraph.addChild("Cube");

			XMLError err = topNode.writeToFile(argv[2]);
			cout << XMLNode::getError(err) << endl;
		}

	cout << endl;
	return 0;
}
//////////////////////////////////////////////////////////////////////////