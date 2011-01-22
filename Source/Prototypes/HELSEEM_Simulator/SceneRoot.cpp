// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/SceneRoot.cpp,v 1.5 2004/05/21 21:18:13 pingrri Exp $
////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "SceneRoot.h"
#include "Utility.h"

#include <osg/MatrixTransform>
#include <osgUtil/Optimizer>
#include <osgDB/ReadFile>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/Billboard>
#include <osgUtil/DisplayListVisitor>
#include <osg/BlendFunc>
#include <osgSim/LightPointNode>
#include <osgSim/LightPoint>

#include <fstream>
#include <iostream>
using namespace std;


std::vector<osg::PositionAttitudeTransform*>	SceneRoot::s_transList;
std::vector<std::string>						SceneRoot::s_fileNameList;
osg::Vec3										SceneRoot::s_cameraPosition(0,0,1);
std::string										SceneRoot::s_targetConfigFile;

osg::Geometry* CreateSquareGeomery(const osg::Vec3& center, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image, float intensity)
{
	osg::Geometry* geom = new osg::Geometry;

	osg::Vec3Array* coords = new osg::Vec3Array(4);

	(*coords)[0] = center - (width / 2.0f) - (height / 2.0f);
	(*coords)[1] = center + (width / 2.0f) - (height / 2.0f);
	(*coords)[2] = center + (width / 2.0f) + (height / 2.0f);
	(*coords)[3] = center - (width / 2.0f) + (height / 2.0f);

    geom->setVertexArray(coords);

    osg::Vec3Array* norms = new osg::Vec3Array(1);
    (*norms)[0] = width ^ height;
    (*norms)[0].normalize();
    
    geom->setNormalArray(norms);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec2Array* tcoords = new osg::Vec2Array(4);
    (*tcoords)[0].set(0.0f, 0.0f);
    (*tcoords)[1].set(1.0f, 0.0f);
    (*tcoords)[2].set(1.0f, 1.0f);
    (*tcoords)[3].set(0.0f, 1.0f);
    geom->setTexCoordArray(0, tcoords);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));
	
    if(image) {
        osg::StateSet* stateset = new osg::StateSet;
        osg::Texture2D* texture = new osg::Texture2D;

        texture->setImage(image);
        stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);

		stateset->setTextureAttribute(0, new osg::TexEnv);
		stateset->setAttributeAndModes(new osg::BlendFunc, osg::StateAttribute::ON);

        osg::Material* material = new osg::Material;
		material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		material->setAlpha(osg::Material::FRONT_AND_BACK, intensity);
        stateset->setAttribute(material);    

        geom->setStateSet(stateset);
    }
	return geom;
}
////////////////////////////////////////////////////////////////////////////////////////

// Create quad at specified position
osg::Geode* createSquare(const osg::Vec3& center, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image, float intensity)
{
    // Set up the geometry
	osg::Geode* squareGeode = new osg::Geode;
	squareGeode->addDrawable(CreateSquareGeomery(center, width, height, image, intensity));
    return squareGeode;
}
////////////////////////////////////////////////////////////////////////////////////////

osg::Billboard* createBillboard(const osg::Vec3& center, const osg::Vec3& width, const osg::Vec3& height, osg::Image* image, float intensity)
{
    // Set up the geometry
	osg::Billboard* bb = new osg::Billboard();
	bb->setMode(osg::Billboard::POINT_ROT_EYE);
	bb->addDrawable(CreateSquareGeomery(center, width, height, image, intensity));
	return bb;
}
////////////////////////////////////////////////////////////////////////////////////////

osg::Node* CreateTargetFlag()
{
	// Create the Geode, Geometry
		osg::Geode* markerGeode = new osg::Geode();
		osg::Geometry* markerLines = new osg::Geometry;

	// Make the lines to the axis points
		osg::Vec3Array* makerLineVerts = new osg::Vec3Array;
		makerLineVerts->push_back(osg::Vec3(0.0f,0.0f,-1.0f));
		makerLineVerts->push_back(osg::Vec3(0.0f, 0.0f, 5.0f));
		markerLines->setVertexArray(makerLineVerts);
	
	// Create the RGB Colors for the 3 axis lines
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
		markerLines->setColorArray(colors);
		markerLines->setColorBinding(osg::Geometry::BIND_OVERALL);

	// set the normal in the same way color.
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
		markerLines->setNormalArray(normals);
		markerLines->setNormalBinding(osg::Geometry::BIND_OVERALL);

	// Add the primitive
		markerLines->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

	// add the points geomtry to the pathGeode.
		markerGeode->addDrawable(markerLines);
		return markerGeode;
}
////////////////////////////////////////////////////////////////////////////////////////

osg::Node* CreateTargetCrosshair()
{
	// Create the Geode, Geometry
		osg::Geode* markerGeode = new osg::Geode();
		osg::Geometry* markerLines = new osg::Geometry;

	// Make the lines to the axis points
		osg::Vec3Array* makerLineVerts = new osg::Vec3Array;
		makerLineVerts->push_back(osg::Vec3(-1.0f,0.0f,0.0f));
		makerLineVerts->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
		makerLineVerts->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
		makerLineVerts->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));
		makerLineVerts->push_back(osg::Vec3(0.0f,0.0f,-1.0f));
		makerLineVerts->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
		markerLines->setVertexArray(makerLineVerts);
	
	// Create the RGB Colors for the 3 axis lines
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
		colors->push_back(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
		colors->push_back(osg::Vec4(0.0f,0.0f,1.0f,1.0f));
		markerLines->setColorArray(colors);
		markerLines->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);

	// set the normal in the same way color.
		osg::Vec3Array* normals = new osg::Vec3Array;
		normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
		markerLines->setNormalArray(normals);
		markerLines->setNormalBinding(osg::Geometry::BIND_OVERALL);

	// Add the primitive
		markerLines->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6));

	// add the points geomtry to the pathGeode.
		markerGeode->addDrawable(markerLines);
		return markerGeode;
}
////////////////////////////////////////////////////////////////////////////////////////

// Use this function to read in lines from a tviz_lwo file, returns NULL for EOF
// WARNING, uses a static buffer
char* ReadLine(std::ifstream& inDataFile, unsigned& numTabs, char& firstC)
{
	static char buff[1024];
	char* ret = NULL;
	while (!ret && inDataFile.getline(buff, 1023))
	{
		// Remove comments by looking for a # and ending the line there
			char* comment = strchr(buff, '#');
			if (comment) *comment = NULL;

		// How many tabs are there before the first char?
			numTabs = 0;
			while (buff[numTabs] == '\t') ++numTabs;

		// What is the first char?  be sure we are not at end
			firstC = buff[numTabs];
			if (firstC)
			{
				// Read past whitespace to start of line
					ret = &(buff[numTabs+1]);
					while ((*ret == ' ') || (*ret == '\t')) ++ret;

				// Remove blank lines
					if (*ret == NULL) ret = NULL;
			}

		// Strip trailing whitespace
			if (ret)
			{
				char* lastNonWhite = NULL;
				char* c = ret;
				while (*c)
				{
					if ((*c != ' ') && (*c != '\t')) lastNonWhite = c;
					++c;
				}
				if (lastNonWhite)
					*(lastNonWhite+1) = NULL;
				else ret = NULL;
			}
	}
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////

SceneRoot::~SceneRoot()
{
	std::ofstream outDataFile(s_targetConfigFile.c_str());
	if (outDataFile)
	{
		outDataFile << "V\t1" << endl;
		outDataFile << endl;

		for (unsigned i = 0; i < s_transList.size(); ++i)
		{
			osg::Vec3 pos = s_transList[i]->getPosition();
			osg::Quat att = s_transList[i]->getAttitude();

			outDataFile << "T\t" << pos[0] << "\t" << pos[1] << "\t" << pos[2] << "\t" << att[0] << "\t" << att[1] << "\t" << att[2] << "\t" << att[3] << endl;
			outDataFile << "\tM\t" << s_fileNameList[i].c_str() << endl << endl;
		}
		outDataFile.close();
	}
}
////////////////////////////////////////////////////////////////////////////////////////

void SceneRoot::ReadTargets(osg::Group* scene)
{
	// Open up the dataFile
		std::ifstream inDataFile(s_targetConfigFile.c_str());

	// Keep a list of the Groups at the present tier
		std::vector<osg::Group*>	groupList;
		groupList.push_back(scene);

	// Read in lines until the file is done
		char* line;
		char firstC;
		unsigned numTabs;
		int version = 1;
		while (line = ReadLine(inDataFile, numTabs, firstC))
		{
		// Switch based on the first character in the line
			switch (firstC)
			{
			case 'V':	// The version of this file, should be the first non-comment line
				sscanf(line, "%i", &version);
				break;
			case 'M':
			case 'N':	// Adds an NSM file, same code as any model
			case 'L':	// Adds the LWO as a leaf node
				{
					char* filename;
					osg::Node* modelNode = NULL;
					try {
						filename = line;
						modelNode = osgDB::readNodeFile(filename);
						s_fileNameList.push_back(std::string(filename));
					}
					catch(...)
					{
						modelNode = NULL;
						char msgText[512];
						sprintf(msgText, "Caught an exception reading the file %s", filename);
						MessageBox(NULL, msgText, "Error reading file", MB_OK);
					}
					if (modelNode)
					{
						// Set the name of the node so we can do the camera reference later
							char* lastDot = strrchr(line, '.');
							if (lastDot) *lastDot = NULL;
							std::string newName = std::string("~") + std::string(filename);
							modelNode->setName(newName);

						// Create crosshairs with the same name
							osg::Node* crosshair = CreateTargetCrosshair();
							crosshair->setName(newName);

						// Insert the model in the scene
							groupList[numTabs]->addChild(modelNode);
							groupList[numTabs]->addChild(crosshair);
					}
					else
					{
						char msgText[512];
						MessageBox(NULL, msgText, "Error reading file", MB_OK);
					}
				}
				break;
			case 'T':	// Creates a simple transform node
				{
					// Grab the values off the line
						float X, Y, Z, r0, r1, r2, r3;
						sscanf(line, "%f %f %f %f %f %f %f", &X, &Y, &Z, &r0, &r1, &r2, &r3);

					// Create a vector for the position and a Quat from the rotations in order
						osg::Vec3 position(X, Y, Z);
						osg::Quat rotation(r0, r1, r2, r3);

					// Create the new Position
						osg::PositionAttitudeTransform* posATT = new osg::PositionAttitudeTransform();
						posATT->setPosition(position);
						posATT->setAttitude(rotation);
						posATT->setName("#PosAttTrans");
						s_transList.push_back(posATT);

					// Add it to the scene
						groupList[numTabs]->addChild(posATT);

					// Add the posATT transform to the group list
						if (groupList.size() <= (numTabs+1))
							groupList.push_back(posATT);
						else groupList[numTabs+1] = posATT;
				}
				break;
			};
		}

	// All done with that file
		inDataFile.close();
}
////////////////////////////////////////////////////////////////////////////////////////

SceneRoot::SceneRoot()
{
	// For testing, set up something to look at
		osg::MatrixTransform* landRoot = new osg::MatrixTransform;
		landRoot->setMatrix(osg::Matrix::scale(0.5, 0.5, 0.05));
		landRoot->addChild(osgDB::readNodeFile("Land.lwo"));
		landRoot->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
		m_landNode = landRoot;


	// For testing, Make the sun a little brighter
		osg::LightSource* lightSource = new osg::LightSource;
		landRoot->addChild(lightSource);
		osg::Light *sun = dynamic_cast<osg::Light *>(lightSource->getLight());
		sun->setPosition(osg::Vec4(0.0, 0.0, 100.0, 1.0));

		float l1_ambi = 0.18f;
		float l1_spec = 0.45f;
		float l1_diff = 0.85f;
		sun->setAmbient(osg::Vec4(l1_ambi,l1_ambi,l1_ambi,1.0f));
		sun->setSpecular(osg::Vec4(l1_spec,l1_spec,l1_spec,1.0f));
		sun->setDiffuse(osg::Vec4(l1_diff,l1_diff,l1_diff,1.0f));

	// Create my targets
		m_targetGroup = new osg::Group;
		ReadTargets(m_targetGroup.get());

	// Create the flag that shows the selected target
		m_selTargetFlag = new osg::PositionAttitudeTransform;
		m_selTargetFlag->addChild(CreateTargetFlag());
		m_selTargetFlag->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

	// This is the aim point of the laser
		m_selTargetTrans = new osg::PositionAttitudeTransform;
		
	// Playing around with a lens flare image
		float intensity = 1.0f;
		float flareSize = 0.03f;
		float centerX = 0.5f;
		osg::Image* image = osgDB::readImageFile("green01.png");
		// m_selTargetTrans->addChild(createSquare(osg::Vec3(centerX, 0.0f, 0.0f), osg::Vec3(centerX, flareSize, 0.0f), osg::Vec3(centerX, 0.0f, flareSize), image, intensity));
		m_selTargetTrans->addChild(createBillboard(osg::Vec3(0.0, -centerX, 0.0f), osg::Vec3(flareSize, -centerX, 0.0f), osg::Vec3(0.0, -centerX, flareSize), image, intensity));


	// How about placing a light on the target
		float modelSize = 0.1f;
		osg::Light* myLight2 = new osg::Light;
		myLight2->setLightNum(1);
		myLight2->setPosition(osg::Vec4(centerX,0.0,0.0,1.0f));
		myLight2->setAmbient(osg::Vec4(0.0f,0.2f,0.0f,1.0f));
		myLight2->setDiffuse(osg::Vec4(0.0f,0.2f,0.0f,1.0f));
		myLight2->setSpecular(osg::Vec4(0.0f,1.0f,0.0f,1.0f));
		myLight2->setConstantAttenuation(1.0f);
		myLight2->setLinearAttenuation(2.0f/modelSize);
		myLight2->setQuadraticAttenuation(2.0f/osg::square(modelSize));
		osg::LightSource* lightS2 = new osg::LightSource;	
		lightS2->setLight(myLight2);
		lightS2->setLocalStateSetModes(osg::StateAttribute::ON); 
		lightS2->setStateSetModes(*(getOrCreateStateSet()),osg::StateAttribute::ON);
		m_selTargetTrans->addChild(lightS2);

	/* Try adding a light point
		osgSim::LightPointNode* lpn = new osgSim::LightPointNode;
		osgSim::LightPoint lp;
		lp._position.set(0.0f,0.0f,0.0f);
		lp._color.set(0.0f,1.0f,0.0f,1.0f);
		lpn->addLightPoint(lp);
		m_selTargetTrans->addChild(lpn);
		*/

	// We do not want shadows on our laser
		m_selTargetTrans->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

	// Here is the scene, make it my only child, until I have targets
		addChild(landRoot);
		addChild(m_targetGroup.get());
		addChild(m_selTargetTrans.get());
		addChild(m_selTargetFlag.get());

	// Be sure we are not using display lists anywhere
		osgUtil::DisplayListVisitor* dlv = new osgUtil::DisplayListVisitor(osgUtil::DisplayListVisitor::SWITCH_OFF_DISPLAY_LISTS);
		accept(*dlv);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: SceneRoot.cpp,v $
// Revision 1.5  2004/05/21 21:18:13  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.4  2004/05/21 17:54:59  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.3  2004/05/21 17:48:12  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.2  2004/05/21 17:38:11  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.1  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//