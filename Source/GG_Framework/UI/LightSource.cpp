// GG_Framework.UI LightSource.cpp
#include "StdAfx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

LightSource::LightSource(ActorScene* actorScene, const char* params) : IScriptOptionsLineAcceptor(actorScene)
{ 
	ASSERT(params && params[0]);
	//For easy reference this is the light's defaults in the constructor
	//_ambient.set(0.05f,0.05f,0.05f,1.0f);
	//_diffuse.set(0.8f,0.8f,0.8f,1.0f);
	//_specular.set(0.05f,0.05f,0.05f,1.0f);
	//_position.set(0.0f,0.0f,1.0f,0.0f);
	//_direction.set(0.0f,0.0f,-1.0f);
	//_spot_exponent = 0.0f;
	//_spot_cutoff = 180.0f;
	//_constant_attenuation = 1.0f;
	//_linear_attenuation = 0.0f;
	//_quadratic_attenuation = 0.0f;
	int LightNumber=0;
	float x=0,y=0,z=0;
	// Rick, we should allow for user to enter no parms... aside from this you must not use asserts for code that has functionality, because in release
		//that code may be ignored - James
	sscanf(params, "%i %f %f %f",&LightNumber,&x,&y,&z);

	this->getLight()->setLightNum(LightNumber);
	this->getLight()->setPosition(osg::Vec4(x,y,z,1.0f));

	//TODO see if I need this
	//float modelSize = 100.0f;
	//this->getLight()->setLinearAttenuation(2.0f/modelSize);
	//this->getLight()->setQuadraticAttenuation(2.0f/osg::square(modelSize));
	
	//Turn on the lights!
	setLocalStateSetModes(osg::StateAttribute::ON);
	//This light source needs the root node's state set, which by default is not created.  
	//This way all children know that this light source will affect them
	osg::Group *RootNode=GetActorScene()->GetScene();
	osg::StateSet *ss=RootNode->getOrCreateStateSet();
	this->setStateSetModes(*ss,osg::StateAttribute::ON);
}
//////////////////////////////////////////////////////////////////////////

bool LightSource::AcceptScriptLine(
		GG_Framework::UI::EventMap& eventMap,
		char indicator,	//!< If there is an indicator character, the caps version is here, or 0 
		const char* lineFromFile)
{
	// All of the lighting controls are functions with parentheses
	const char* firstParen = strchr(lineFromFile, '(');
	if (!firstParen) return false;
	if (strncmp(lineFromFile, "setDirection", 12) == 0)
	{
		float x=0, y=0, z=0;
		if (sscanf(firstParen, "(%f,%f,%f)", &x, &y, &z) == 3)
		{
			getLight()->setDirection(osg::Vec3(x,y,z));
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setDiffuse", 10) == 0)
	{
		float x=0, y=0, z=0, a=0;
		if (sscanf(firstParen, "(%f,%f,%f,%f)", &x, &y, &z, &a) == 4)
		{
			getLight()->setDiffuse(osg::Vec4(x,y,z,a));
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setSpecular", 11) == 0)
	{
		float x=0, y=0, z=0, a=0;
		if (sscanf(firstParen, "(%f,%f,%f,%f)", &x, &y, &z, &a) == 4)
		{
			getLight()->setSpecular(osg::Vec4(x,y,z,a));
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setAmbient", 10) == 0)
	{
		float x=0, y=0, z=0, a=0;
		if (sscanf(firstParen, "(%f,%f,%f,%f)", &x, &y, &z, &a) == 4)
		{
			getLight()->setAmbient(osg::Vec4(x,y,z,a));
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setConstantAttenuation", 22) == 0)
	{
		float x=0;
		if (sscanf(firstParen, "(%f)", &x) == 1)
		{
			getLight()->setConstantAttenuation(x);
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setLinearAttenuation", 20) == 0)
	{
		float x=0;
		if (sscanf(firstParen, "(%f)", &x) == 1)
		{
			getLight()->setLinearAttenuation(x);
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setQuadraticAttenuation", 23) == 0)
	{
		float x=0;
		if (sscanf(firstParen, "(%f)", &x) == 1)
		{
			getLight()->setQuadraticAttenuation(x);
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setSpotExponent", 15) == 0)
	{
		float x=0;
		if (sscanf(firstParen, "(%f)", &x) == 1)
		{
			getLight()->setSpotExponent(x);
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}
	else if (strncmp(lineFromFile, "setSpotCutoff", 13) == 0)
	{
		float x=0;
		if (sscanf(firstParen, "(%f)", &x) == 1)
		{
			getLight()->setSpotCutoff(x);
		}
		else
		{
			DebugOutput("Error in line format: %s", lineFromFile);
		}
		return true;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
