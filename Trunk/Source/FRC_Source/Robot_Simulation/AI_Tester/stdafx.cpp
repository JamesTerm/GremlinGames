// stdafx.cpp : source file that includes just the standard includes
// AI_Tester.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

#ifdef _DEBUG
#pragma comment (lib,"osgd")
#pragma comment (lib,"osgDBd")
#pragma comment (lib,"osgGAd")
#pragma comment (lib,"osgTextd")
#pragma comment (lib,"osgUtild")
#pragma comment (lib,"osgViewerd")
#pragma comment (lib,"OpenThreadsd")
#else
#pragma comment (lib,"osg")
#pragma comment (lib,"osgDB")
#pragma comment (lib,"osgGA")
#pragma comment (lib,"osgText")
#pragma comment (lib,"osgUtil")
#pragma comment (lib,"osgViewer")
#pragma comment (lib,"OpenThreads")
#endif
