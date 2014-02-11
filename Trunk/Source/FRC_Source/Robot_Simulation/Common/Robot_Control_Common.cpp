#include "stdafx.h"
#include "Common.h"
namespace Robot_Tester
{
#include "Robot_Control_Interface.h"
}

#ifdef Robot_TesterCode
using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;
#else
using namespace Framework::Base;
using namespace std;
#endif


  /***********************************************************************************************************************************/
 /*													Control_Assignment_Properties													*/
/***********************************************************************************************************************************/

static void LoadControlElement_1C_Internal(Scripting::Script& script,Control_Assignment_Properties::Controls_1C &Output)
{
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"id_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Control_Element_1C newElement;
			{
				double fTest;
				err = script.GetField("channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.Channel=(size_t)fTest;
				err = script.GetField("name",&newElement.name,NULL,NULL);
				assert(!err);
				err = script.GetField("module",NULL,NULL,&fTest);
				newElement.Module=(err)?1:(size_t)fTest;
				assert(newElement.Module!=0);  //sanity check... this is cardinal
			}
			Output.push_back(newElement);
			script.Pop();
			index++;
		}
	} while (!fieldtable_err);

}

static void LoadControlElement_2C_Internal(Scripting::Script& script,Control_Assignment_Properties::Controls_2C &Output)
{
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	const char* err=NULL;
	const char* fieldtable_err=NULL;
	char Buffer[128];
	size_t index=1;  //keep the lists cardinal in LUA
	do 
	{
		sprintf_s(Buffer,128,"id_%d",index);
		fieldtable_err = script.GetFieldTable(Buffer);
		if (!fieldtable_err)
		{
			Control_Element_2C newElement;
			{
				double fTest;
				err = script.GetField("forward_channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.ForwardChannel=(size_t)fTest;
				err = script.GetField("reverse_channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.ReverseChannel=(size_t)fTest;
				err = script.GetField("name",&newElement.name,NULL,NULL);
				assert(!err);
				err = script.GetField("module",NULL,NULL,&fTest);
				newElement.Module=(err)?1:(size_t)fTest;
				assert(newElement.Module!=0);  //sanity check... this is cardinal
			}
			Output.push_back(newElement);
			script.Pop();
			index++;
		}
	} while (!fieldtable_err);
}

void Control_Assignment_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("control_assignments");
	if (!err) 
	{
		err = script.GetFieldTable("victor");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Victors);
			script.Pop();
		}
		err = script.GetFieldTable("digital_input");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Digital_Inputs);
			script.Pop();
		}
		err = script.GetFieldTable("double_solenoid");
		if (!err)
		{
			LoadControlElement_2C_Internal(script,m_Double_Solenoids);
			script.Pop();
		}
		script.Pop();
	}
}
