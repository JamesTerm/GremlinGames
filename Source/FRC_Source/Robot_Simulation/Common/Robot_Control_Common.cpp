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


  /***********************************************************************************************************************************/
 /*														RobotControlCommon															*/
/***********************************************************************************************************************************/

RobotControlCommon::~RobotControlCommon()
{

}

void RobotControlCommon::RobotControlCommon_Initialize(const Control_Assignment_Properties &props)
{
	m_Props=props;
	typedef Control_Assignment_Properties::Controls_1C Controls_1C;
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	//create control elements and their LUT's
	{
		//victors
		const Controls_1C &control_props=props.GetVictors();
		for (size_t i=0;i<control_props.size();i++)
		{
			const Control_Element_1C &element=control_props[i];
			//create the new Victor
			#ifdef Robot_TesterCode
			Victor *NewVictor=new Victor(element.Module,element.Channel,element.name.c_str());  //adding name for UI
			#else
			Victor *NewVictor=new Victor(element.Module,element.Channel);
			#endif
			const size_t PopulationIndex=m_Victors.size();  //get the ordinal value before we add it
			m_Victors.push_back(NewVictor);  //add it to our list of victors
			//Now to work out the new LUT
			size_t enumIndex=RobotControlCommon_Get_Victor_EnumValue(element.name.c_str());
			//our LUT is the EnumIndex position set to the value of i... make sure we have the slots created
			assert(enumIndex<10);  //sanity check we have a limit to how many victors we have
			while(m_VictorLUT.size()<=enumIndex)
				m_VictorLUT.push_back(-1);  //fill with -1 as a way to indicate nothing is located for that slot
			m_VictorLUT[enumIndex]=i;
		}
	}
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*														Control_1C_Element_UI														*/
/***********************************************************************************************************************************/

Control_1C_Element_UI::Control_1C_Element_UI(uint8_t moduleNumber, uint32_t channel,const char *name)
{
	m_Name=name;
	char Buffer[4];
	m_Name+="_";
	itoa(channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(moduleNumber,Buffer,10);
	m_Name+=Buffer;
}

void Control_1C_Element_UI::display(double value)
{
	SmartDashboard::PutNumber(m_Name,value);
}

#endif
