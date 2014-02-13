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
				if (err)
					err=script.GetField("a_channel",NULL,NULL,&fTest);
				assert(!err);
				newElement.ForwardChannel=(size_t)fTest;
				err = script.GetField("reverse_channel",NULL,NULL,&fTest);
				if (err)
					err=script.GetField("b_channel",NULL,NULL,&fTest);
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
		err = script.GetFieldTable("relay");
		if (!err)
		{
			LoadControlElement_1C_Internal(script,m_Relays);
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
		err = script.GetFieldTable("digital_input_encoder");
		if (!err)
		{
			LoadControlElement_2C_Internal(script,m_Encoders);
			script.Pop();
		}
		err = script.GetFieldTable("compressor");
		if (!err)
		{
			double fTest;
			err = script.GetField("relay",NULL,NULL,&fTest);
			assert(!err);
			m_Compressor_Relay=(size_t)fTest;
			err = script.GetField("limit",NULL,NULL,&fTest);
			assert(!err);
			m_Compressor_Limit=(size_t)fTest;
			script.Pop();
		}
		else
			m_Compressor_Relay=8,m_Compressor_Limit=14;

		script.Pop();
	}
}


  /***********************************************************************************************************************************/
 /*														RobotControlCommon															*/
/***********************************************************************************************************************************/

RobotControlCommon::~RobotControlCommon()
{

}

template <class T>
__inline void Initialize_1C_LUT(const Control_Assignment_Properties::Controls_1C &control_props,std::vector<T *> &constrols,
								RobotControlCommon::Controls_LUT &control_LUT,RobotControlCommon *instance,size_t (RobotControlCommon::*delegate)(const char *name) const)
{
	typedef Control_Assignment_Properties::Controls_1C Controls_1C;
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	for (size_t i=0;i<control_props.size();i++)
	{
		const Control_Element_1C &element=control_props[i];
		//create the new Control
		#ifdef Robot_TesterCode
		T *NewElement=new T(element.Module,element.Channel,element.name.c_str());  //adding name for UI
		#else
		T *NewElement=new T(element.Module,element.Channel);
		#endif
		const size_t PopulationIndex=constrols.size();  //get the ordinal value before we add it
		constrols.push_back(NewElement);  //add it to our list of victors
		//Now to work out the new LUT
		size_t enumIndex=(instance->*delegate)(element.name.c_str());
		//our LUT is the EnumIndex position set to the value of i... make sure we have the slots created
		assert(enumIndex<10);  //sanity check we have a limit to how many victors we have
		while(control_LUT.size()<=enumIndex)
			control_LUT.push_back(-1);  //fill with -1 as a way to indicate nothing is located for that slot
		control_LUT[enumIndex]=i;
	}
}

template <class T>
__inline void Initialize_2C_LUT(const Control_Assignment_Properties::Controls_2C &control_props,std::vector<T *> &constrols,
								RobotControlCommon::Controls_LUT &control_LUT,RobotControlCommon *instance,size_t (RobotControlCommon::*delegate)(const char *name) const)
{
	typedef Control_Assignment_Properties::Controls_2C Controls_2C;
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	for (size_t i=0;i<control_props.size();i++)
	{
		const Control_Element_2C &element=control_props[i];
		//create the new Control
		#ifdef Robot_TesterCode
		T *NewElement=new T(element.Module,element.ForwardChannel,element.ReverseChannel,element.name.c_str());
		#else
		T *NewElement=new T(element.Module,element.ForwardChannel,element.ReverseChannel);
		#endif
		const size_t PopulationIndex=constrols.size();  //get the ordinal value before we add it
		constrols.push_back(NewElement);  //add it to our list of victors
		//Now to work out the new LUT
		size_t enumIndex=(instance->*delegate)(element.name.c_str());
		//our LUT is the EnumIndex position set to the value of i... make sure we have the slots created
		assert(enumIndex<10);  //sanity check we have a limit to how many victors we have
		while(control_LUT.size()<=enumIndex)
			control_LUT.push_back(-1);  //fill with -1 as a way to indicate nothing is located for that slot
		control_LUT[enumIndex]=i;
	}
}


void RobotControlCommon::RobotControlCommon_Initialize(const Control_Assignment_Properties &props)
{
	m_Props=props;
	typedef Control_Assignment_Properties::Controls_1C Controls_1C;
	typedef Control_Assignment_Properties::Control_Element_1C Control_Element_1C;
	typedef Control_Assignment_Properties::Controls_2C Controls_2C;
	typedef Control_Assignment_Properties::Control_Element_2C Control_Element_2C;
	//create control elements and their LUT's
	//victors
	Initialize_1C_LUT<Victor>(props.GetVictors(),m_Victors,m_VictorLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
	//relays
	Initialize_1C_LUT<Relay>(props.GetRelays(),m_Relays,m_RelayLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
	//double solenoids
	Initialize_2C_LUT<DoubleSolenoid>(props.GetDoubleSolenoids(),m_DoubleSolenoids,m_DoubleSolenoidLUT,this,&RobotControlCommon::RobotControlCommon_Get_DoubleSolenoid_EnumValue);
	//digital inputs
	Initialize_1C_LUT<DigitalInput>(props.GetDigitalInputs(),m_DigitalInputs,m_DigitalInputLUT,this,&RobotControlCommon::RobotControlCommon_Get_DigitalInput_EnumValue);
	//encoders
	Initialize_2C_LUT<Encoder2>(props.GetEncoders(),m_Encoders,m_EncoderLUT,this,&RobotControlCommon::RobotControlCommon_Get_Victor_EnumValue);
}


void RobotControlCommon::TranslateToRelay(size_t index,double Voltage)
{
	Relay::Value value=Relay::kOff;  //*NEVER* want both on!
	const double Threshold=0.08;  //This value is based on dead voltage for arm... feel free to adjust, but keep high enough to avoid noise

	if (Voltage>Threshold)
		value=Relay::kForward;
	else if (Voltage<-Threshold)
		value=Relay::kReverse;

	m_Relays[m_RelayLUT[index]]->Set(value);
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*																Encoder2															*/
/***********************************************************************************************************************************/

Encoder2::Encoder2(uint8_t ModuleNumber,UINT32 aChannel, UINT32 bChannel,const char *name) : 
	Control_2C_Element_UI(ModuleNumber,aChannel,bChannel,name), m_LastDistance(0.0)
{
	m_Name+="_encoder";
}

void Encoder2::TimeChange(double dTime_s,double adjustment_delta)
{
	m_LastTime=dTime_s;
	m_Distance+=adjustment_delta;
	display_number(m_Distance);
}
void Encoder2::Reset2()
{
	m_LastDistance=0.0;
	Reset();
}

double Encoder2::GetRate2(double dTime_s)
{
	//Using distance will yield the same rate as GetRate, without precision loss to GetRaw()
	const double CurrentDistance=GetDistance();
	const double delta=CurrentDistance - m_LastDistance;
	m_LastDistance=CurrentDistance;
	return delta/dTime_s;
}

void Encoder2::Start() {}
int32_t Encoder2::Get() {return 0;}
int32_t Encoder2::GetRaw() {return 0;}
void Encoder2::Reset() 
{
	m_Distance=0;
}
void Encoder2::Stop() {}
double Encoder2::GetDistance() {return m_Distance;}
double Encoder2::GetRate() {return GetRate2(m_LastTime);}
void Encoder2::SetMinRate(double minRate) {}
void Encoder2::SetDistancePerPulse(double distancePerPulse) {}
void Encoder2::SetReverseDirection(bool reverseDirection) {}

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

void Control_1C_Element_UI::display_number(double value)
{
	SmartDashboard::PutNumber(m_Name,value);
}

void Control_1C_Element_UI::display_bool(bool value)
{
	SmartDashboard::PutBoolean(m_Name,value);
}

bool Control_1C_Element_UI::get_bool() const
{
	return SmartDashboard::GetBoolean(m_Name);
}
double Control_1C_Element_UI::get_number() const
{
	return (double)SmartDashboard::GetNumber(m_Name);
}

  /***********************************************************************************************************************************/
 /*														Control_2C_Element_UI														*/
/***********************************************************************************************************************************/

Control_2C_Element_UI::Control_2C_Element_UI(uint8_t moduleNumber, uint32_t forward_channel, uint32_t reverse_channel,const char *name)
{
	m_Name=name;
	char Buffer[4];
	m_Name+="_";
	itoa(forward_channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(reverse_channel,Buffer,10);
	m_Name+=Buffer;
	m_Name+="_";
	itoa(moduleNumber,Buffer,10);
	m_Name+=Buffer;
}

void Control_2C_Element_UI::display_bool(bool value)
{
	SmartDashboard::PutBoolean(m_Name,value);
}

void Control_2C_Element_UI::display_number(double value)
{
	SmartDashboard::PutNumber(m_Name,value);
}

bool Control_2C_Element_UI::get_bool() const
{
	return SmartDashboard::GetBoolean(m_Name);
}
#endif
