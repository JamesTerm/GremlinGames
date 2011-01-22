// Misc.cpp
#include "stdafx.h"
#include "../../../CompilerSettings.h"
#include "../FrameWork_OSG.h"

namespace FrameWork
{
	namespace OSG
	{
		namespace Tester
		{

SUITE(MiscFuncs_Tester)
{
	TEST(FillNodePath)
	{
		//! \todo We can add a test here if we find a problem
	}	

	TEST(GetNodeMatrix)
	{
		//! \todo We can add a test here if we find a problem
	}	

	TEST(GetNodePosition)
	{
		//! \todo We can add a test here if we find a problem
	}	

	TEST(FindChildNode)
	{
		//! \todo We can add a test here if we find a problem
	}	

	TEST(GetDistance)
	{
		CHECK_EQUAL(5.0, FrameWork::OSG::GetDistance(osg::Vec3(1,1,1), osg::Vec3(4,5,1)));
	}	

	TEST(GetMaxAnimTime)
	{
		//! \todo We can add a test here if we find a problem
	}	
}
		}
	}
}