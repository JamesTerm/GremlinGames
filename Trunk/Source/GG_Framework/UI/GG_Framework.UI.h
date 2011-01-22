// GG_Framework.UI.h
#pragma once

#ifdef FRAMEWORK_UI_EXPORTS
#define FRAMEWORK_UI_API __declspec(dllexport)
#else
#define FRAMEWORK_UI_API __declspec(dllimport)
#endif

///////////////////
// Useful Constants
const double FRAMES_PER_SEC = 30.0;
#define TIME_2_FRAME(time) ((int)(((time)*FRAMES_PER_SEC)+0.5))
#define FRAME_2_TIME(frame) ((double)((double)(frame)/FRAMES_PER_SEC))

#include <osg\Vec3>
#include <osg\Quat>
#include "..\Base\GG_Framework.Base.h"
static osg::Vec3 FromLW_Pos(double X, double Y, double Z){return osg::Vec3(X, Z, Y);}
static osg::Quat FromLW_Rot(double H, double P, double R)
{
	return osg::Quat(
		DEG_2_RAD(H)	, osg::Vec3d(0,0,1),
		DEG_2_RAD(-P)	, osg::Vec3d(1,0,0),
		DEG_2_RAD(-R)	, osg::Vec3d(0,1,0));
}
static osg::Quat FromLW_Rot_Radians(double H, double P, double R)
{
	return osg::Quat(
		H	, osg::Vec3d(0,0,1),
		-P	, osg::Vec3d(1,0,0),
		-R	, osg::Vec3d(0,1,0));
}

//////////////////
// Other Libraries
#include "..\..\CompilerSettings.h"
#include "..\Base\GG_Framework.Base.h"
#include "OSG\GG_Framework.UI.OSG.h"
#include "Audio\GG_Framework.UI.Audio.h"

///////////////////////
// Fringe.UI Includes
#include "EventMap.h"
#include "Text_PDCB.h"
#include "Interfaces.h"
#include "Effect.h"
#include "NamedEffect.h"
#include "Action.h"
#include "Impulse.h"
#include "Actor.h"
#include "ParticleEffect.h"
#include "ActorScene.h"
#include "SoundEffect.h"
#include "CameraParentedTransforms.h"
#include "LightSource.h"
#include "ImageNodes.h"
#include "Camera.h"
#include "ConfigurationManager.h"
#include "JoystickBinder.h"
#include "KeyboardMouse_CB.h"
#include "MainWindow.h"
#include "CenteredTrackball_CamManipulator.h"
#include "Framerate_PDCB.h"
#include "DebugOut_PDCB.h"
#include "ArgumentParser.h"
#include "PerformanceSwitch.h"
#include "ScreenCaptureTool.h"
#include "MapFramesEffect.h"
#include "OsgFile_ParticleEffect.h"
#include "CustomParticleEffect.h"


