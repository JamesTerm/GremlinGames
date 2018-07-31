// Robot_Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Robot_Tester.h"
namespace Robot_Tester
{
	#include "Viewer.h"
}
#include "Robots.h"

void cls(void *hConsole=NULL);
extern double g_WorldScaleFactor;
extern bool g_TestPhysics;

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace std;



class GUIThread : public GG_Framework::Base::ThreadedClass
{
	public:
		GUIThread(bool useUserPrefs=true) : m_Viewer(NULL),m_IsBeingDestroyed(false),m_UseUserPrefs(useUserPrefs)
		{
			Run();
		}
		~GUIThread() 
		{
			//ThreadSleep(600);
			if (!m_IsBeingDestroyed)
			{
				m_IsBeingDestroyed=true;
				delete m_Viewer;
				m_Viewer=NULL;
			}
			cancel();
		}
		void Run()
		{
			m_IsBeingDestroyed=false;
			start();
		}
		Viewer *GetUI() {return m_Viewer;}
		bool GetUseUserPrefs() const {return m_UseUserPrefs;}
	protected:
		void tryRun() 
		{
			assert(!m_Viewer);
			m_Viewer= new Viewer(m_UseUserPrefs);
			m_Viewer->Start();
			//printf("Exiting GUI Thread\n");
			if (!m_IsBeingDestroyed)
			{
				m_IsBeingDestroyed=true;
				delete m_Viewer;
				m_Viewer=NULL;
			}
		}
	private:
		Viewer *m_Viewer;
		bool m_IsBeingDestroyed;
		bool m_UseUserPrefs;
};

void createHUDText(osg::Group *rootNode,osg::Geode* geode );
class TestCallback : public Viewer_Callback_Interface
{
	public:
		TestCallback() : m_IsSetup(false) {}
		bool GetIsSetup() const {return m_IsSetup;}
	protected:  //from UI_Callback_Interface
		virtual void UpdateData(double dtime_s) {}
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode)
		{
			if (!m_IsSetup)
			{
				createHUDText(rootNode,geode);
				m_IsSetup=true;
			}
		}
	private:
		bool m_IsSetup;
};
const double PI=3.1415926535897;

//Use PI/2 to init the cycle on zero for rho
double SineInfluence(double &rho,double freq_hz=0.1,double SampleRate=30.0,double amplitude=1.0)
{
	double			 theta, scale, pi2;

	pi2 = PI;
	theta = freq_hz / SampleRate;
	theta *= (pi2 * 2.0);
	pi2 *= 2.0;

	scale = amplitude;

	double Sample= sin( rho ) * scale;
	rho += theta;
	if ( rho > pi2 )
		rho -= pi2;
	return Sample;
}

double g_dTest=0.0;
double g_dTest2=0.0;

class Test_Actor2 : public osg::Drawable::UpdateCallback
{
public:
	Test_Actor2() : m_Rho(0.0) 
	{
		osg::Vec4 layoutColor(0.0f,1.0f,1.0f,1.0f);
		m_Text = new osgText::Text;
		////m_Text->setFont(font);
		m_Text->setColor(layoutColor);
		m_Text->setCharacterSize(20.0);
		m_Text->setFontResolution(10,10);

		osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
		m_Text->setPosition(position);
		//m_Text->setDrawMode(osgText::Text::TEXT|osgText::Text::BOUNDINGBOX);
		m_Text->setAlignment(osgText::Text::CENTER_CENTER);
		m_Text->setText("/^\\\n|:|\nC|:|D\n/*\\");
		m_Text->setUpdateCallback(this);
	}
	osg::ref_ptr<osgText::Text> GetText() {return m_Text;}
protected:
	virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw) 
	{
		osgText::Text *Text=dynamic_cast<osgText::Text *>(draw);
		double sample=SineInfluence(m_Rho);
		if (Text)
		{
			double halfxres=c_Scene_XRes_InPixels/2.0;
			double halfyres=c_Scene_YRes_InPixels/2.0;
			osg::Vec3 position(sample*halfxres + halfxres,0.5*c_Scene_YRes_InPixels,0.0f);
			Text->setPosition(position);
			Text->setRotation(FromLW_Rot_Radians(sample*PI,0.0,0.0));
		}
		//printf("\r %f      ",sample);

	}
private:
	osg::ref_ptr<osgText::Text> m_Text;
	double m_Rho;
};

class Test_Actor : public osg::Drawable::UpdateCallback
{
public:
	Test_Actor() : m_Rho(0.0) {}
protected:
	virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw) 
	{
		osgText::Text *Text=dynamic_cast<osgText::Text *>(draw);
		double sample=SineInfluence(m_Rho);
		if (Text)
		{
			double halfxres=c_Scene_XRes_InPixels/2.0;
			double halfyres=c_Scene_YRes_InPixels/2.0;
			osg::Vec3 position(sample*halfxres + halfxres,0.5*c_Scene_YRes_InPixels,0.0f);
			//osg::Vec3 position(0.5*c_Scene_XRes_InPixels,sample*halfyres + halfyres,0.0f);
			Text->setPosition(position);
			//Text->setRotation(FromLW_Rot_Radians(g_dTest,0.0,0.0));
			Text->setRotation(FromLW_Rot_Radians(sample*PI,0.0,0.0));
		}
		//printf("\r %f      ",sample);

	}
private:
	double m_Rho;
};

class TestCallback_2 : public Viewer_Callback_Interface
{
	public:
		TestCallback_2() : m_IsSetup(false) {}
		bool GetIsSetup() const {return m_IsSetup;}
	protected:  //from UI_Callback_Interface
		virtual void UpdateData(double dtime_s) {}
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode)
		{
			if (!m_IsSetup)
			{
#if 0
				osg::Vec4 layoutColor(0.0f,1.0f,1.0f,1.0f);
				osgText::Text *text = new osgText::Text;
				////text->setFont(font);
				text->setColor(layoutColor);
				text->setCharacterSize(20.0);
				text->setFontResolution(10,10);

				osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
				text->setPosition(position);
				//text->setDrawMode(osgText::Text::TEXT|osgText::Text::BOUNDINGBOX);
				text->setAlignment(osgText::Text::CENTER_CENTER);
				text->setText("/^\\\n|:|\nC|:|D\n/*\\");
				m_Actor=new Test_Actor;
				text->setUpdateCallback(m_Actor);
				geode->addDrawable(text);
#else
				m_Actor=new Test_Actor2;
				geode->addDrawable(m_Actor->GetText());
#endif
				m_IsSetup=true;
			}
		}
	private:
		//osg::ref_ptr<Test_Actor> m_Actor;
		osg::ref_ptr<Test_Actor2> m_Actor;
		bool m_IsSetup;
};

void DisplayHelp()
{
	printf(
		"Start\n"
		"timer <use synthetic deltas>\n"
		"Stop\n"
		"0-inert 1-capital 2-bomber 3-fighter 4-flak 5-scout 6-sniper 7-spawn\n"
		"LoadShip <filename> <ship name>\n"
		"AddShip <unique name> <name> <x> <y>\n"
		"LoadRobot <filename> <ship name>\n"
		"AddRobot <unique name> <name> <x> <y>\n"
		"Follow <name> <FollowShip> <rel x=-40> <rel y=-40>\n"
		"SetPos <name> <x> <y>\n"
		"MovePos <name> <x> <y>\n"
		"SetAtt <name> <degrees> \n"
		"MoveAtt <name> <degrees> \n"
		"Control <name>\n"
		"Mouse <1=use>\n"
		"RemoveShip <name>\n"
		"RampEngine <1=ramp>\n"
		"Zoom <scale factor>"
		"Test <#>\n"
		"Time [y|Y|1] SetUseSyntheticTimeDeltas else false\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
		);
}

void SetUpUI(GUIThread *&UI_thread,Viewer_Callback_Interface *ViewerCallback,bool useUserPrefs=true)
{
	//Check if we have an thread instance
	if (!UI_thread)
		UI_thread=new GUIThread(useUserPrefs);

	assert(UI_thread);

	//Ensure the thread is running... it will not be in the case where user closed UI window from a previous session
	if ((!UI_thread->GetUI())&&(!UI_thread->isRunning()))
		UI_thread->Run();


	//Now to let the thread kick in... we wait for it to finish
	size_t TimeOut=0;
	//wait for the UI to get set up (doh)
	while (!UI_thread->GetUI() && TimeOut++<2000)
		ThreadSleep(10);

	//Finally it should be good to go... set the callback interface
	if ((UI_thread)&&(UI_thread->GetUI()))
		UI_thread->GetUI()->SetCallbackInterface(ViewerCallback);
	else
		throw "Unable to start UI";
}

class Commands
{
private:	
	typedef map<string ,UI_Ship_Properties,greater<string>> ShipMap;
	ShipMap Character_Database;
	typedef map<string ,Tank_Robot_Properties,greater<string>> RobotMap;
	RobotMap Robot_Database;
	typedef map<string ,Swerve_Robot_Properties,greater<string>> SwerveRobotMap;
	SwerveRobotMap SwerveRobot_Database;
	typedef map<string ,Butterfly_Robot_Properties,greater<string>> ButterflyRobotMap;
	ButterflyRobotMap ButterflyRobot_Database;
	typedef map<string ,Nona_Robot_Properties,greater<string>> NonaRobotMap;
	NonaRobotMap NonaRobot_Database;

	typedef map<string ,FRC_2011_Robot_Properties,greater<string>> Robot2011Map;
	Robot2011Map Robot2011_Database;
	typedef map<string ,FRC_2012_Robot_Properties,greater<string>> Robot2012Map;
	Robot2012Map Robot2012_Database;
	typedef map<string ,FRC_2013_Robot_Properties,greater<string>> Robot2013Map;
	Robot2013Map Robot2013_Database;
	typedef map<string ,FRC_2014_Robot_Properties,greater<string>> Robot2014Map;
	Robot2014Map Robot2014_Database;
	typedef map<string ,FRC_2015_Robot_Properties,greater<string>> Robot2015Map;
	Robot2015Map Robot2015_Database;
	typedef map<string ,Curivator_Robot_Properties,greater<string>> CurivatorMap;
	CurivatorMap Curivator_Database;
	typedef map<string ,HikingViking_Robot_Properties,greater<string>> RobotHikingVikingMap;
	RobotHikingVikingMap RobotHikingViking_Database;

	UI_Controller_GameClient &game;

public:
	Commands(UI_Controller_GameClient &_game) : game(_game) {}

	void LoadShip(const char *FileName,const char *ShipName)
	{
		ShipMap::iterator iter=Character_Database.find(ShipName);
		if (iter==Character_Database.end())
		{
			//New entry
			Character_Database[ShipName]=UI_Ship_Properties();
			UI_Ship_Properties &new_entry=Character_Database[ShipName];  //reference to avoid copy
			GG_Framework::Logic::Scripting::Script script;
			script.LoadScript(FileName,true);
			script.NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";
			const char *test=new_entry.SetUpGlobalTable(script);
			assert(test==NULL);
			new_entry.LoadFromScript(script);
		}
		else
			printf("%s already loaded\n",ShipName);
	}
	Entity2D *AddShip(const char *str_1,const char *str_2,const char *str_3,const char *str_4,const char *str_5)
	{
		Entity2D *ret=NULL;
		ShipMap::iterator iter=Character_Database.find(str_2);
		if (iter!=Character_Database.end())
		{
			double x=atof(str_3);
			double y=atof(str_4);
			double heading=atof(str_5);
			Entity2D *TestEntity=NULL;
			TestEntity=game.AddEntity(str_1,(*iter).second);
			Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
			if (ship)
			{
				ship->SetPosition(x,y);
				ship->SetAttitude(heading * (PI/180.0));
				ret=ship;
			}
		}
		else
			printf("%s Ship not found, try loading it\n",str_2);
		return ret;
	}
	enum RobotType
	{
		eTank,
		eSwerve,
		eButterfly,
		eNona,
		e2011,
		e2012,
		e2013,
		e2014,
		e2015,
		eHikingViking,
		eCurivator
	};
	void LoadRobot(const char *FileName,const char *RobotName,RobotType type)
	{
		UI_Ship_Properties *new_entry=NULL;
		switch (type)
		{
			case eTank:
				{
					RobotMap::iterator iter=Robot_Database.find(RobotName);
					if (iter==Robot_Database.end())
					{
						//New entry
						Robot_Database[RobotName]=Tank_Robot_Properties();
						new_entry=&Robot_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case eSwerve:
				{
					SwerveRobotMap::iterator iter=SwerveRobot_Database.find(RobotName);
					if (iter==SwerveRobot_Database.end())
					{
						//New entry
						SwerveRobot_Database[RobotName]=Swerve_Robot_Properties();
						new_entry=&SwerveRobot_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case eButterfly:
				{
					ButterflyRobotMap::iterator iter=ButterflyRobot_Database.find(RobotName);
					if (iter==ButterflyRobot_Database.end())
					{
						//New entry
						ButterflyRobot_Database[RobotName]=Butterfly_Robot_Properties();
						new_entry=&ButterflyRobot_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case eNona:
				{
					NonaRobotMap::iterator iter=NonaRobot_Database.find(RobotName);
					if (iter==NonaRobot_Database.end())
					{
						//New entry
						NonaRobot_Database[RobotName]=Nona_Robot_Properties();
						new_entry=&NonaRobot_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case e2011:
				{
					Robot2011Map::iterator iter=Robot2011_Database.find(RobotName);
					if (iter==Robot2011_Database.end())
					{
						//New entry
						Robot2011_Database[RobotName]=FRC_2011_Robot_Properties();
						new_entry=&Robot2011_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case e2012:
				{
					Robot2012Map::iterator iter=Robot2012_Database.find(RobotName);
					if (iter==Robot2012_Database.end())
					{
						//New entry
						Robot2012_Database[RobotName]=FRC_2012_Robot_Properties();
						new_entry=&Robot2012_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case e2013:
				{
					Robot2013Map::iterator iter=Robot2013_Database.find(RobotName);
					if (iter==Robot2013_Database.end())
					{
						//New entry
						Robot2013_Database[RobotName]=FRC_2013_Robot_Properties();
						new_entry=&Robot2013_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case e2014:
				{
					Robot2014Map::iterator iter=Robot2014_Database.find(RobotName);
					if (iter==Robot2014_Database.end())
					{
						//New entry
						Robot2014_Database[RobotName]=FRC_2014_Robot_Properties();
						new_entry=&Robot2014_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case e2015:
				{
					Robot2015Map::iterator iter=Robot2015_Database.find(RobotName);
					if (iter==Robot2015_Database.end())
					{
						//New entry
						Robot2015_Database[RobotName]=FRC_2015_Robot_Properties();
						new_entry=&Robot2015_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case eHikingViking:
				{
					RobotHikingVikingMap::iterator iter=RobotHikingViking_Database.find(RobotName);
					if (iter==RobotHikingViking_Database.end())
					{
						//New entry
						RobotHikingViking_Database[RobotName]=HikingViking_Robot_Properties();
						new_entry=&RobotHikingViking_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
			case eCurivator:
				{
					CurivatorMap::iterator iter=Curivator_Database.find(RobotName);
					if (iter==Curivator_Database.end())
					{
						//New entry
						Curivator_Database[RobotName]=Curivator_Robot_Properties();
						new_entry=&Curivator_Database[RobotName];  //reference to avoid copy
					}
				}
				break;
		}
		if (new_entry)
		{
			GG_Framework::Logic::Scripting::Script script;
			script.LoadScript(FileName,true);
			script.NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";
			const char *test=new_entry->SetUpGlobalTable(script);
			assert(test==NULL);
			new_entry->LoadFromScript(script);
		}
		else
			printf("%s already loaded\n",RobotName);
	}

	Entity2D *AddRobot(const char *str_1,const char *str_2,const char *str_3,const char *str_4,const char *str_5)
	{
		Entity2D *ret=NULL;
		RobotMap::iterator iter=Robot_Database.find(str_2);
		Entity_Properties *props=NULL;
		if (iter!=Robot_Database.end())
			props=&((*iter).second);

		if (props==NULL)
		{
			SwerveRobotMap::iterator iter=SwerveRobot_Database.find(str_2);
			if (iter!=SwerveRobot_Database.end())
				props=&((*iter).second);
		}

		if (props==NULL)
		{
			ButterflyRobotMap::iterator iter=ButterflyRobot_Database.find(str_2);
			if (iter!=ButterflyRobot_Database.end())
				props=&((*iter).second);
		}

		if (props==NULL)
		{
			NonaRobotMap::iterator iter=NonaRobot_Database.find(str_2);
			if (iter!=NonaRobot_Database.end())
				props=&((*iter).second);
		}

		if (props==NULL)
		{
			Robot2011Map::iterator iter=Robot2011_Database.find(str_2);
			if (iter!=Robot2011_Database.end())
				props=&((*iter).second);
		}

		if (props==NULL)
		{
			Robot2012Map::iterator iter=Robot2012_Database.find(str_2);
			if (iter!=Robot2012_Database.end())
				props=&((*iter).second);
		}
		if (props==NULL)
		{
			Robot2013Map::iterator iter=Robot2013_Database.find(str_2);
			if (iter!=Robot2013_Database.end())
				props=&((*iter).second);
		}
		if (props==NULL)
		{
			Robot2014Map::iterator iter=Robot2014_Database.find(str_2);
			if (iter!=Robot2014_Database.end())
				props=&((*iter).second);
		}
		if (props==NULL)
		{
			Robot2015Map::iterator iter=Robot2015_Database.find(str_2);
			if (iter!=Robot2015_Database.end())
				props=&((*iter).second);
		}
		if (props==NULL)
		{
			RobotHikingVikingMap::iterator iter=RobotHikingViking_Database.find(str_2);
			if (iter!=RobotHikingViking_Database.end())
				props=&((*iter).second);
		}
		if (props==NULL)
		{
			CurivatorMap::iterator iter=Curivator_Database.find(str_2);
			if (iter!=Curivator_Database.end())
				props=&((*iter).second);
		}

		if (props)
		{
			double x=atof(str_3);
			double y=atof(str_4);
			double heading=atof(str_5);
			Entity2D *TestEntity=NULL;
			TestEntity=game.AddEntity(str_1,*props);
			Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
			if (ship)
			{
				if (str_3[0]!=0)
					ship->SetDefaultPosition(osg::Vec2d(x,y));
				if (str_4[0]!=0)
					ship->SetDefaultAttitude(heading * (PI/180.0));
				ship->ResetPos();
				ret=ship;
			}
		}
		else
			printf("%s Robot not found, try loading it\n",str_2);
		return ret;
	}

};


const double Kilograms2Pounds=2.204622622;
const double Pounds2Kilograms=0.453592;
const double NewtonsToPounds=0.22480894309973575244048822067636;
const double PoundsToNewtons=4.44822161526;
class DriveTrainCalcTest
{
private:
	Drive_Train_Characteristics m_dtc;
public:
	DriveTrainCalcTest()
	{
		EncoderSimulation_Props props;
		props.motor.Free_Current_Amp=0.4;
		props.motor.Stall_Current_Amp=84;
		props.motor.Stall_Torque_NM=34;
		props.motor.FreeSpeed_RPM=263.88;
		props.Wheel_Mass=1.8;
		props.COF_Efficiency=0.9;
		props.GearReduction=1.0;
		props.TorqueAccelerationDampener=1.0;  //not needed for this test
		props.DriveWheelRadius=Inches2Meters(4.0);  //in meters  (note: consider changing to diameter)
		props.NoMotors=1.0;
		props.PayloadMass=200.0 * Pounds2Kilograms;  //in kilograms
		props.SpeedLossConstant=0.81;
		props.DriveTrainEfficiency=0.9;
		m_dtc.UpdateProps(props);
	}
	void Display()
	{
		const EncoderSimulation_Props &props=m_dtc.GetDriveTrainProps();
		const EncoderSimulation_Props::Motor_Specs &motor=props.motor;
		printf("fs=%.2f st=%.2f sca=%.2f fca=%.2f\n",motor.FreeSpeed_RPM,motor.Stall_Torque_NM,motor.Stall_Current_Amp,motor.Free_Current_Amp);
		printf("gb=2 nm=%.2f dwd=%.2f CoF=%.2f trw=%.2f ",props.NoMotors,props.DriveWheelRadius*39.3700787*2,props.COF_Efficiency,props.PayloadMass*2.20462);
		printf("wd=1.0 slc=%.2f dte=%.2f\n",props.SpeedLossConstant,props.DriveTrainEfficiency);
		const double MaxWheel=motor.FreeSpeed_RPM*props.GearReduction*props.SpeedLossConstant;
		const double WheelCircumference=props.DriveWheelRadius*2*PI;   //or better yet pi * d
		printf("MaxSpeed=%.2f\n",Meters2Feet(MaxWheel*WheelCircumference)/60.0);
		printf("MaxWheel=%.2f\n",MaxWheel);
		//const double WheelStallTorque=motor.Stall_Torque_NM/props.GearReduction  * props.DriveTrainEfficiency;
		const double WheelStallTorque=m_dtc.GetWheelStallTorque();
		printf("WheelStallTorque=%.2f\n",WheelStallTorque);
		//const double MaxTractionPounds=props.PayloadMass*Kilograms2Pounds*props.COF_Efficiency;
		const double MaxTractionPounds=m_dtc.GetMaxTraction()*Kilograms2Pounds;
		//const double MaxDriveForce=WheelStallTorque/(props.DriveWheelRadius*PoundsToNewtons)*2.0;  //Original form
		//torque=force * distance arraged to force=torque/distance and multiplied by number of gearboxes then converted to pounds
		//const double MaxDriveForce=(WheelStallTorque/props.DriveWheelRadius)*2.0*NewtonsToPounds;
		const double MaxDriveForce=m_dtc.GetMaxDriveForce()*NewtonsToPounds;
		//printf("MaxPushingForce= min(mt=%.2f mdf=%.2f) = %.2f\n",MaxTractionPounds,MaxDriveForce,std::min(MaxTractionPounds,MaxDriveForce));
		printf("MaxPushingForce= min(mt=%.2f mdf=%.2f) = %.2f\n",MaxTractionPounds,MaxDriveForce,m_dtc.GetMaxPushingForce()*NewtonsToPounds);
		const double DriveLoadPerSide=MaxTractionPounds/2;
		const double DriveLoadNewtons=DriveLoadPerSide*PoundsToNewtons;
		const double DriveLoadNM=DriveLoadNewtons*props.DriveWheelRadius;
		const double DriveLoad_Total=DriveLoadNM/props.DriveTrainEfficiency;
		printf("MotorTorqueLoad=%.2f\n",(DriveLoad_Total/props.GearReduction)/props.NoMotors);
	}
	__inline double I_disk(double mass,double r, double gr) const
	{
		return 0.5 * mass * r * r * gr;
	}
	__inline double I_disk_volume(double density,double length, double diameter) const
	{
		return (PI / 32.0) * density * length * (diameter*diameter*diameter*diameter);
	}
	double ComputeTotalInertia() const
	{
		struct MassElement
		{
			const char *Name;
			double msss;
			double radius;
			double gear_ratio;
		};
		MassElement elements[]=
		{
			{"motor",0.48,1.41/2.0,1.0},
			{"pulley12",0.0072,0.1792/2.0,1.0},
			{"pulley36",0.0577,1.035/2.0,1.0},
			{"sun_4_1",0.0252,0.5/2.0,1.0},
			{"planeta_spin_4_1",0.0138,0.5/2.0,1.0},
			{"planetb_spin_4_1",0.0138,0.5/2.0,1.0},
			{"planetc_spin_4_1",0.0138,0.5/2.0,1.0},
			{"planetd_spin_4_1",0.0138,0.5/2.0,1.0},
			{"planeta_orb_4_1",0.0138,0.46,2.0},  //using 2 to define point mass
			{"planetb_orb_4_1",0.0138,0.46,2.0},
			{"planetc_orb_4_1",0.0138,0.46,2.0},
			{"planetd_orb_4_1",0.0138,0.46,2.0},
			{"planetCarrier",0.0566,1.366/2.0,1.0},
			{"sun_3_1",0.0435,0.6299/2.0,1.0},
			{"planeta_spin_3_1",0.0138,0.5/2.0,1.0},
			{"planetb_spin_3_1",0.0138,0.5/2.0,1.0},
			{"planetc_spin_3_1",0.0138,0.5/2.0,1.0},
			{"planetd_spin_3_1",0.0138,0.5/2.0,1.0},
			{"planeta_orb_3_1",0.0138,0.46,2.0},  //using 2 to define point mass
			{"planetb_orb_3_1",0.0138,0.46,2.0},
			{"planetc_orb_3_1",0.0138,0.46,2.0},
			{"planetd_orb_3_1",0.0138,0.46,2.0},
			{"planetCarrier",0.0566,1.366/2.0,1.0},
			{"vp180_shaftOutput",0.06,0.5/2.0,1.0},
			{"sprocket_15t",0.05,1.6/2.0,1.0},
			{"chain_a",0.3732/2.0,0.9,1.0},
			{"chain_b",0.3732/2.0,3.61/2.0,1.0},
			{"Wheel",3.0,4.0,1.0}  //I really cheated on this one
		};
		size_t noElements=_countof(elements);
		double total=0;
		for (size_t i=0;i<noElements;i++)
		{
			MassElement element=elements[i];
			const double mass=element.msss * Pounds2Kilograms;
			const double r=Inches2Meters(element.radius);
			const double gr=element.gear_ratio;
			const double elementInertia=I_disk(mass,r,gr);
			printf("%s = %f\n",element.Name,elementInertia);
			total+=elementInertia;
		}
		printf("total = %f\n",total);
		return total;
	}
};

void Test(GUIThread *UI_thread,UI_Controller_GameClient &game,Commands &_command, const char * const Args[])
{
	const char * const str_1=Args[0];
	const char * const str_2=Args[1];
	const char * const str_3=Args[2];
	const char * const str_4=Args[3];
	const char * const str_5=Args[4];

	class commonStuff
	{
		public:
			commonStuff(GUIThread *_UI_thread,UI_Controller_GameClient &_game) : UI_thread(_UI_thread),game(_game)		{}
			void GiveSquareWayPointGoal(Ship_Tester *ship)
			{
				std::list <WayPoint> points;
				struct Locations
				{
					double x,y;
				} test[]=
				{
					{100.0,100.0},
					{100.0,-100.0},
					{-100.0,-100.0},
					{-100.0,100.0}
				};
				for (size_t i=0;i<_countof(test);i++)
				{
					WayPoint wp;
					wp.Position[0]=test[i].x;
					wp.Position[1]=test[i].y;
					wp.Power=0.5;
					points.push_back(wp);
				}
				//Now to setup the goal
				Goal_Ship_FollowPath *goal=new Goal_Ship_FollowPath(ship->GetController(),points,true);
				ship->SetGoal(goal);
			}
			void GiveRobotSquareWayPointGoal(Ship_Tester *ship)
			{
				std::list <WayPoint> points;
				struct Locations
				{
					double x,y;
				} test[]=
				{
					{4.0,4.0},
					{4.0,-4.0},
					{-4.0,-4.0},
					{-4.0,4.0}
				};
				for (size_t i=0;i<_countof(test);i++)
				{
					WayPoint wp;
					wp.Position[0]=test[i].x;
					wp.Position[1]=test[i].y;
					wp.Power=0.5;
					points.push_back(wp);
				}
				//Now to setup the goal
				Goal_Ship_FollowPath *goal=new Goal_Ship_FollowPath(ship->GetController(),points,true);
				ship->SetGoal(goal);
			}

			void ShipFollowShip(Ship_Tester *ship,Ship_Tester *Followship,double x=-40,double y=-40,double TrajectoryOffset=100.0)
			{
				osg::Vec2d RelPosition(x,y);
				assert(ship && Followship);
				//Now to setup the goal
				Goal_Ship_FollowShip *goal=new Goal_Ship_FollowShip(ship->GetController(),*Followship,RelPosition,TrajectoryOffset);
				ship->SetGoal(goal);
			}
		private:
			GUIThread *UI_thread;
			UI_Controller_GameClient &game;
	} _(UI_thread,game);

	int Test=atoi(str_1);

	enum
	{
		eCurrent,
		eTankRobot,
		eSwerveRobot,
		eButterflyRobot,
		eNonaRobot,
		eCurivator,
		eTestGoals_Curivator,
		eRobot2011,
		eTestGoals_2011,
		eRobotHikingViking,
		eRobot2012,
		eTestGoals_2012,
		eRobot2013,
		eTestGoals_2013,
		eRobot2014,
		eTestGoals_2014,
		eRobot2015,
		eTestGoals_2015,
		eTestTankFollowGod,
		eTestFollowGod,
		eTestLUAShip,
		eActorUpdateTest,
		eTextTest,
		eTestDriveTrainCalc
	};
	const char * const TestName[]=
	{
		"current",
		"TankRobot",
		"SwerveRobot",
		"ButterflyRobot",
		"NonaRobot",
		"Curivator",
		"GoalsCurivator",
		"Robot2011",
		"Goals2011",
		"HikingViking",
		"Robot2012",
		"Goals2012",
		"Robot2013",
		"Goals2013",
		"Robot2014",
		"Goals2014",
		"Robot2015",
		"Goals2015",
		"TankFollowGod",
		"FollowGod",
		"GodShip",
		"ActorUpdateTest",
		"TextTest",
		"TestDriveCalc"
	};

	//if the first character is not a number then translate the string
	if (((str_1[0]<'0')||(str_1[0]>'9'))&&(str_1[0]!=0))
	{
		Test=-1;
		for (size_t i=0;i<_countof(TestName);i++)
		{
			if (stricmp(TestName[i],str_1)==0)
			{
				Test=i;
				break;
			}
		}
		if (Test==-1)
		{
			printf("No match found.  Try:\n");
			for (size_t i=0;i<_countof(TestName);i++)
				printf("%s, ",TestName[i]);
			printf("\n");
			return;
		}
	}

	switch(Test)
	{
	case eTankRobot:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("TestTankRobot.lua","TestRobot",Commands::eTank);
			Entity2D *TestEntity=_command.AddRobot("TankRobot","TestRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eSwerveRobot:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("TestSwerveRobot.lua","TestSwerveRobot",Commands::eSwerve);
			Entity2D *TestEntity=_command.AddRobot("SwerveRobot","TestSwerveRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eButterflyRobot:
		{
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("TestButterflyRobot.lua","TestButterflyRobot",Commands::eButterfly);
			Entity2D *TestEntity=_command.AddRobot("ButterflyRobot","TestButterflyRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eNonaRobot:
		{
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("TestNonaRobot.lua","TestNonaRobot",Commands::eNona);
			Entity2D *TestEntity=_command.AddRobot("NonaRobot","TestNonaRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eRobot2011:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("FRC2011Robot.lua","FRC2011Robot",Commands::e2011);
			Entity2D *TestEntity=_command.AddRobot("Robot2011","FRC2011Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eTestGoals_2011:
		{
			FRC_2011_Robot *Robot=dynamic_cast<FRC_2011_Robot *>(game.GetEntity("Robot2011"));
			if (Robot)
			{
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				Goal *goal=NULL;
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				switch (AutonomousValue)
				{
					//case 1:		goal=Test_Arm(Robot);			break;
				case 2:		goal=FRC_2011_Goals::Get_TestLengthGoal(Robot);					break;
					//case 3:		goal=Get_TestRotationGoal(ship);				break;
				case 4:		goal=FRC_2011_Goals::Get_UberTubeGoal(Robot);	break;
				}
				if (goal)
					goal->Activate(); //now with the goal(s) loaded activate it
				Robot->SetGoal(goal);
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eRobotHikingViking:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("HikingVikingRobot.lua","HikingVikingRobot",Commands::eHikingViking);
			Entity2D *TestEntity=_command.AddRobot("RobotHikingViking","HikingVikingRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,UI_thread->GetUseUserPrefs());
		}
		break;
	case eRobot2012:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("FRC2012Robot.lua","FRC2012Robot",Commands::e2012);
			Entity2D *TestEntity=_command.AddRobot("Robot2012","FRC2012Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,false);
		}
		break;
	case eTestGoals_2012:
		{
			FRC_2012_Robot *Robot=dynamic_cast<FRC_2012_Robot *>(game.GetEntity("Robot2012"));
			if (Robot)
			{
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				if (DoAutonomous)
				{
					if (((AutonomousValue >> 6) & 1)==0)
					{
						//For this year we'll break up into 3 set pair of buttons (at least until vision is working)
						//First set is the key, second the target, and last the ramps.  Once vision is working we can
						//optionally remove key
						const size_t Key_Selection=   (AutonomousValue >> 0) & 3;
						const size_t Target_Selection=(AutonomousValue >> 2) & 3;
						const size_t Ramp_Selection=  (AutonomousValue >> 4) & 3;
						//Translate... the index is center left right, but we want right, left, and center
						const size_t KeyTable[4] = {(size_t)-1,2,1,0};
						const size_t Key=KeyTable[Key_Selection];
						//We'll want to have no buttons also represent the top target to compensate for user error (should always have a target!)
						const size_t TargetTable[4] = {0,2,1,0};
						const size_t Target=TargetTable[Target_Selection];
						const size_t Ramp=KeyTable[Ramp_Selection];
						//Just to be safe check (if they had the other buttons selected)
						if (Key!=(size_t)-1)
						{
							Goal *goal=NULL;
							goal=FRC_2012_Goals::Get_FRC2012_Autonomous(Robot,Key,Target,Ramp);
							if (goal)
								goal->Activate(); //now with the goal(s) loaded activate it
							Robot->SetGoal(goal);
						}
					}
					else
					{
						Goal *goal=NULL;
						goal=FRC_2012_Goals::Get_ShootBalls(Robot,true);
						if (goal)
							goal->Activate(); //now with the goal(s) loaded activate it
						Robot->SetGoal(goal);
					}
				}
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eRobot2013:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("FRC2013Robot.lua","FRC2013Robot",Commands::e2013);
			Entity2D *TestEntity=_command.AddRobot("Robot2013","FRC2013Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,false);
		}
		break;
	case eTestGoals_2013:
		{
			FRC_2013_Robot *Robot=dynamic_cast<FRC_2013_Robot *>(game.GetEntity("Robot2013"));
			if (Robot)
			{
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				if (DoAutonomous)
				{
					Goal *goal=NULL;
					goal=FRC_2013_Goals::Get_ShootFrisbees(Robot,(AutonomousValue&2)!=0);
					if (goal)
						goal->Activate(); //now with the goal(s) loaded activate it
					Robot->SetGoal(goal);
				}
			}
			else
				printf("Robot not found\n");
		}
		break;
	case eRobot2014:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("FRC2014Robot.lua","FRC2014Robot",Commands::e2014);
			Entity2D *TestEntity=_command.AddRobot("Robot2014","FRC2014Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,false);
		}
		break;
	case eTestGoals_2014:
		{
			FRC_2014_Robot *Robot=dynamic_cast<FRC_2014_Robot *>(game.GetEntity("Robot2014"));
			if (Robot)
			{
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				if (DoAutonomous)
				{
					Goal *goal=NULL;
					goal=FRC_2014_Goals::Get_FRC2014_Autonomous(Robot);
					if (goal)
						goal->Activate(); //now with the goal(s) loaded activate it
					Robot->SetGoal(goal);
				}
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eRobot2015:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("FRC2015Robot.lua","FRC2015Robot",Commands::e2015);
			Entity2D *TestEntity=_command.AddRobot("Robot2015","FRC2015Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,false);
		}
		break;
	case eTestGoals_2015:
		{
			FRC_2015_Robot *Robot=dynamic_cast<FRC_2015_Robot *>(game.GetEntity("Robot2015"));
			if (Robot)
			{
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				if (DoAutonomous)
				{
					Goal *goal=NULL;
					goal=FRC_2015_Goals::Get_FRC2015_Autonomous(Robot);
					if (goal)
						goal->Activate(); //now with the goal(s) loaded activate it
					Robot->SetGoal(goal);
				}
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eCurivator:
	case eCurrent:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			_command.LoadRobot("CurivatorRobot.lua","CurivatorRobot",Commands::eCurivator);
			Entity2D *TestEntity=_command.AddRobot("Curivator","CurivatorRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity,false);
		}
		break;
	case eTestGoals_Curivator:
		{
			Curivator_Robot *Robot=dynamic_cast<Curivator_Robot *>(game.GetEntity("Curivator"));
			if (Robot)
			{
				const int AutonomousValue=str_2[0]?atoi(str_2):2;
				const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
				Goal *oldgoal=Robot->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				if (DoAutonomous)
				{
					Goal *goal=NULL;
					goal=Curivator_Goals::Get_Curivator_Autonomous(Robot);
					if (goal)
						goal->Activate(); //now with the goal(s) loaded activate it
					Robot->SetGoal(goal);
				}
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eTestTankFollowGod:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;

			Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity("Robot2015"));

			Ship_Tester *Followship=dynamic_cast<Ship_Tester *>(game.GetEntity("GodShip"));
			if (!ship)
			{
				_command.LoadRobot("FRC2015Robot.lua","FRC2015Robot",Commands::e2015);
				ship=dynamic_cast<Ship_Tester *>(_command.AddRobot("Robot2015","FRC2015Robot",str_3,str_4,str_5));
			}

			if (!Followship)
			{
				_command.LoadShip("TestShip.lua","TestShip");
				Followship=dynamic_cast<Ship_Tester *>(_command.AddShip("GodShip","TestShip",str_3,str_4,str_5));
			}
			_.ShipFollowShip(ship,Followship,0.0,-1.0,1.0);
			_.GiveRobotSquareWayPointGoal(Followship);
			game.SetControlledEntity(Followship,UI_thread->GetUseUserPrefs());
		}
		break;
	case eTestFollowGod:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;

			Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity("TankRobot"));

			Ship_Tester *SwerveShip=dynamic_cast<Ship_Tester *>(game.GetEntity("SwerveRobot"));
			//Ship_Tester *SwerveShip=dynamic_cast<Ship_Tester *>(game.GetEntity("ButterflyRobot"));
			Ship_Tester *NonaShip=dynamic_cast<Ship_Tester *>(game.GetEntity("NonaRobot"));

			Ship_Tester *Followship=dynamic_cast<Ship_Tester *>(game.GetEntity("GodShip"));
			if (!ship)
			{
				_command.LoadRobot("TestTankRobot.lua","TestRobot",Commands::eTank);
				ship=dynamic_cast<Ship_Tester *>(_command.AddRobot("TankRobot","TestRobot",str_3,str_4,str_5));
			}
			if (!SwerveShip)
			{
				#if 1
				_command.LoadRobot("TestSwerveRobot.lua","TestSwerveRobot",Commands::eSwerve);
				SwerveShip=dynamic_cast<Ship_Tester *>(_command.AddRobot("SwerveRobot","TestSwerveRobot",str_3,str_4,str_5));
				#endif
				#if 0
				_command.LoadRobot("TestButterflyRobot.lua","TestButterflyRobot",Commands::eButterfly);
				SwerveShip=dynamic_cast<Ship_Tester *>(_command.AddRobot("ButterflyRobot","TestButterflyRobot",str_3,str_4,str_5));
				#endif
			}
			if (!NonaShip)
			{
				#if 1
				_command.LoadRobot("TestNonaRobot.lua","TestNonaRobot",Commands::eNona);
				NonaShip=dynamic_cast<Ship_Tester *>(_command.AddRobot("NonaRobot","TestNonaRobot",str_3,str_4,str_5));
				#endif
			}

			if (!Followship)
			{
				_command.LoadShip("TestShip.lua","TestShip");
				Followship=dynamic_cast<Ship_Tester *>(_command.AddShip("GodShip","TestShip",str_3,str_4,str_5));
			}
			_.ShipFollowShip(ship,Followship,0.0,-1.0,0.5);
			_.ShipFollowShip(SwerveShip,Followship,-1.0,-1.0,5.0);
			_.ShipFollowShip(NonaShip,Followship,1.0,-1.0,5.0);
			_.GiveRobotSquareWayPointGoal(Followship);
			game.SetControlledEntity(Followship,UI_thread->GetUseUserPrefs());
		}
		break;
	case eTestLUAShip:
		{
			g_WorldScaleFactor=100.0;
			_command.LoadShip("TestShip.lua","TestShip");
			Entity2D *TestEntity=_command.AddShip("GodShip","TestShip",str_3,str_4,str_5);
			_.GiveRobotSquareWayPointGoal(dynamic_cast<Ship_Tester *>(TestEntity));
			game.SetControlledEntity(TestEntity);
		}
		break;
	case eActorUpdateTest:
		{
			TestCallback_2 test;
			SetUpUI(UI_thread,&test);
			assert (UI_thread);
			size_t TimeOut=0;
			while(!test.GetIsSetup()&&TimeOut++<100)
				ThreadSleep(200);
			UI_thread->GetUI()->SetCallbackInterface(NULL);
		}
		break;

	case eTextTest:
		{
			TestCallback test;
			SetUpUI(UI_thread,&test);
			assert (UI_thread);
			size_t TimeOut=0;
			while(!test.GetIsSetup()&&TimeOut++<100)
				ThreadSleep(200);
			UI_thread->GetUI()->SetCallbackInterface(NULL);
		}
		break;
	case eTestDriveTrainCalc:
	//case eCurrent:
		{
			DriveTrainCalcTest test;
			//test.Display();
			test.ComputeTotalInertia();
		}
		break;
	}
}

#define MAX_PATH          260
#pragma warning(disable : 4996)
void CommandLineInterface(bool useUserPrefs=true)
{
	UI_Controller_GameClient game;

	Commands _command(game);

	#if 0
	GUIThread *UI_thread=NULL;
	#else
	GUIThread *UI_thread=NULL;
	SetUpUI(UI_thread,&game,useUserPrefs);
	#endif
	cout << endl;
	cout << "Ready." << endl;

   	char input_line[128];

	char		command[32];
	char		str_1[MAX_PATH];
	char		str_2[MAX_PATH];
	char		str_3[MAX_PATH];
	char		str_4[MAX_PATH];
	char		str_5[MAX_PATH];

	const char * const Args[]=
	{
		str_1,str_2,str_3,str_4,str_5
	};

    while (cout << ">",cin.getline(input_line,128))
    {
		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;
		str_5[0]=0;

		if (sscanf( input_line,"%s %s %s %s %s %s",command,str_1,str_2,str_3,str_4,str_5)>=1)
		{
			if (!_strnicmp( input_line, "cls", 3))
			{
				cls();
			}
			else if (!_strnicmp( input_line, "Start", 5))
			{
				//These are an auto clear... it is ideal for when code is not running properly I can restart fresh
				delete UI_thread;
				UI_thread=NULL;
				SetUpUI(UI_thread,&game);
			}
			else if (!_strnicmp( input_line, "Stop", 4))
			{
				//Disengage UI control
				game.SetControlledEntity(NULL);
				delete UI_thread;
				UI_thread=NULL;
				game.RemoveAllEntities();
			}
			else if (!_strnicmp( input_line, "timer", 5))
			{
				assert(UI_thread && UI_thread->GetUI());
				UI_thread->GetUI()->SetUseSyntheticTimeDeltas(atoi(str_1)==1);
			}
			else if (!_strnicmp( input_line, "LoadShip", 5))
			{
				_command.LoadShip(str_1,str_2);
			}
			else if (!_strnicmp( input_line, "AddShip", 4))
			{
				_command.AddShip(str_1,str_2,str_3,str_4,str_5);
			}
			else if (!_strnicmp( input_line, "LoadRobot", 5))
			{
				_command.LoadRobot(str_1,str_2,(Commands::RobotType)atoi(str_3));
			}
			else if (!_strnicmp( input_line, "AddRobot", 4))
			{
				_command.AddRobot(str_1,str_2,str_3,str_4,str_5);
			}

			else if (!_strnicmp( input_line, "SetPos", 6))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				double x=atof(str_2);
				double y=atof(str_3);
				if (ship)
					ship->SetPosition(x,y);
			}
			else if (!_strnicmp( input_line, "MovePos", 5))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				double x=atof(str_2);
				double y=atof(str_3);
				if (ship)
				{
					Goal *oldgoal=ship->ClearGoal();
					if (oldgoal)
						delete oldgoal;
					//Construct a way point
					WayPoint wp;
					wp.Position[0]=x;
					wp.Position[1]=y;
					wp.Power=1.0;
					//Now to setup the goal
					Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp);
					ship->SetGoal(goal);
				}
			}
			else if (!_strnicmp( input_line, "MoveAtt", 5))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				double heading=atof(str_2);
				if (ship)
				{
					Goal *oldgoal=ship->ClearGoal();
					if (oldgoal)
						delete oldgoal;
					//Now to setup the goal
					Goal_Ship_RotateToPosition *goal=new Goal_Ship_RotateToPosition(ship->GetController(),DEG_2_RAD(heading));
					ship->SetGoal(goal);
				}
			}
			else if (!_strnicmp( input_line, "UseEncoders", 6))
			{
				Tank_Robot *robot=dynamic_cast<Tank_Robot *>(game.GetEntity(str_1));
				if (robot)
					robot->SetUseEncoders(atoi(str_2)==0?false:true);
				else
					printf("%s is not loaded as a tank robot",str_1);
			}
			else if (!_strnicmp( input_line, "Follow", 6))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				Ship_Tester *Followship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_2));
				double x=str_3[0]?atof(str_3):-40;
				double y=str_4[0]?atof(str_4):-40;
				osg::Vec2d RelPosition(x,y);
				if (ship)
				{
					Goal *oldgoal=ship->ClearGoal();
					if (oldgoal)
						delete oldgoal;
					//Now to setup the goal
					if (Followship)
					{
						Goal_Ship_FollowShip *goal=new Goal_Ship_FollowShip(ship->GetController(),*Followship,RelPosition);
						ship->SetGoal(goal);
					}
					else
						ship->SetGoal(NULL);
				}
			}
			else if (!_strnicmp( input_line, "SetAtt", 6))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				double heading=atof(str_2);
				if (ship)
				{
					//ship->SetAttitude(heading * (PI/180.0));
					const_cast<double &>(ship->GetIntendedOrientation())=heading * (PI/180.0);
				}
			}
			else if (!_strnicmp( input_line, "Control", 7))
			{
				Entity2D *ship=game.GetEntity(str_1);
				//Ship can be null which relinquish control
				game.SetControlledEntity(ship);
			}
			else if (!_strnicmp( input_line, "Mouse", 5))
			{
				bool UseMouse=atoi(str_1)==1;
				g_UseMouse=UseMouse;
			}
			else if (!_strnicmp( input_line, "Remove", 6))
			{
				game.RemoveEntity(str_1);
			}
			else if (!_strnicmp( input_line, "Zoom", 4))
			{
				//TODO find a cool default
				g_WorldScaleFactor=str_1[0]?atof(str_1):1.0;
			}
			else if (!_strnicmp( input_line, "Test", 4))
			{
				Test(UI_thread,game,_command,Args);
			}
			else if (!_strnicmp( input_line, "poke", 4))
			{
				double value=atof(str_1);
				g_dTest=value;
				double value2=atof(str_2);
				g_dTest2=value2;
			}
			else if (!_strnicmp( input_line, "time", 4))
			{
				if ((str_1[0]=='y')||(str_1[0]=='Y')||(str_1[0]=='1'))
					UI_thread->GetUI()->SetUseSyntheticTimeDeltas(true);
				else
					UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			}
			else if (!_strnicmp( input_line, "Help", 4))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Quit", 4))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
    }
	delete UI_thread;
	UI_thread=NULL;
}

int main(int argc, char** argv)
{
	SmartDashboard::init();
	DisplayHelp();
	bool useUserPrefs=false;
	if ((argc==2)&&((argv[1])[0]=='1'))
		useUserPrefs=true;
	CommandLineInterface(useUserPrefs);
	SmartDashboard::shutdown();
}