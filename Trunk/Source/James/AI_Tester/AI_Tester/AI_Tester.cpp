// AI_Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Viewer.h"
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "FRC2011_Robot.h"
	#include "FRC2012_Robot.h"
	#include "Swerve_Robot.h"
}

void cls(void *hConsole=NULL);
extern double g_WorldScaleFactor;
extern bool g_TestPhysics;
extern bool g_UseMouse;

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace std;



class GUIThread : public GG_Framework::Base::ThreadedClass
{
	public:
		GUIThread() : m_Viewer(NULL),m_IsBeingDestroyed(false)
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
	protected:
		void tryRun() 
		{
			assert(!m_Viewer);
			m_Viewer= new Viewer;
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
		"AddCharacter <name> <type> <x> <y>\n"
		"0-inert 1-capital 2-bomber 3-fighter 4-flak 5-scout 6-sniper 7-spawn\n"
		"LoadShip <filename> <ship name>\n"
		"AddShip <unique name> <name> <x> <y>\n"
		"Follow <name> <FollowShip> <rel x=-40> <rel y=-40>\n"
		"SetPos <name> <x> <y>\n"
		"MovePos <name> <x> <y>\n"
		"SetAtt <name> <degrees> \n"
		"MoveAtt <name> <degrees> \n"
		"Control <name>\n"
		"Mouse <1=use>\n"
		"Join <ship name> <team name>\n"
		"RemoveShip <name>\n"
		"RampEngine <1=ramp>\n"
		"Zoom <scale factor>"
		"Test <#>\n"
		"Help (displays this)\n"
		"\nType \"Quit\" at anytime to exit this application\n"
		);
}

void SetUpUI(GUIThread *&UI_thread,Viewer_Callback_Interface *ViewerCallback)
{
	//Check if we have an thread instance
	if (!UI_thread)
		UI_thread=new GUIThread;

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

	typedef map<string ,FRC_2011_Robot_Properties,greater<string>> Robot2011Map;
	Robot2011Map Robot2011_Database;
	typedef map<string ,FRC_2012_Robot_Properties,greater<string>> Robot2012Map;
	Robot2012Map Robot2012_Database;

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
		e2011,
		e2012
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
			Ship_Tester *Control_A_Bomber()
			{
				SetUpUI(UI_thread,&game);
				Entity2D *TestEntity=NULL;
				TestEntity=game.AddEntity("test",e_Bomber);
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
				assert(ship);
				ship->GetGameAttributes().GetTeamName()="blue";
				game.SetControlledEntity(TestEntity);
				return ship;
			}
			Ship_Tester *Create_A_Fighter(const char *Name)
			{
				SetUpUI(UI_thread,&game);
				Entity2D *TestEntity=NULL;
				TestEntity=game.AddEntity(Name,e_Fighter);
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
				assert(ship);
				ship->GetGameAttributes().GetTeamName()="blue";
				return ship;
			}
			Ship_Tester *Create_A_Sniper(const char *Name)
			{
				SetUpUI(UI_thread,&game);
				Entity2D *TestEntity=NULL;
				TestEntity=game.AddEntity(Name,e_Sniper);
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
				assert(ship);
				ship->GetGameAttributes().GetTeamName()="red";
				return ship;
			}
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
		eRobot2012,
		eRobot2011,
		eTestGoals_2011,
		eTestGoals_2012,
		eTestFollowGod,
		eTestLUAShip,
		eControlABomber,
		eFollowShipTest,
		eFollowPathTest,
		ePhysicsTest,
		eActorUpdateTest,
		eTextTest
	};
	const char * const TestName[]=
	{
		"current",
		"TankRobot",
		"SwerveRobot",
		"Robot2012",
		"Robot2011",
		"Goals2011",
		"Goals2012",
		"FollowGod",
		"GodShip",
		"bomber",
		"FollowTest",
		"FollowPathTest",
		"PhysicsTest",
		"ActorUpdateTest",
		"TextTest"
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
			game.SetDisableEngineRampUp2(true);
			_command.LoadRobot("TestRobot.lua","TestRobot",Commands::eTank);
			Entity2D *TestEntity=_command.AddRobot("TankRobot","TestRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity);
		}
		break;

	case eSwerveRobot:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			game.SetDisableEngineRampUp2(true);
			_command.LoadRobot("TestSwerveRobot.lua","TestSwerveRobot",Commands::eSwerve);
			Entity2D *TestEntity=_command.AddRobot("SwerveRobot","TestSwerveRobot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity);
		}
		break;
	case eCurrent:
	case eRobot2012:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			game.SetDisableEngineRampUp2(true);
			_command.LoadRobot("FRC2012Robot.lua","FRC2012Robot",Commands::e2012);
			Entity2D *TestEntity=_command.AddRobot("Robot2012","FRC2012Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity);
		}
		break;
	case eRobot2011:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			game.SetDisableEngineRampUp2(true);
			_command.LoadRobot("FRC2011Robot.lua","FRC2011Robot",Commands::e2011);
			Entity2D *TestEntity=_command.AddRobot("Robot2011","FRC2011Robot",str_3,str_4,str_5);
			game.SetControlledEntity(TestEntity);
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
				case 2:		goal=Get_TestLengthGoal(Robot);					break;
					//case 3:		goal=Get_TestRotationGoal(ship);				break;
				case 4:		goal=Get_UberTubeGoal(Robot);	break;
				}
				if (goal)
					goal->Activate(); //now with the goal(s) loaded activate it
				Robot->SetGoal(goal);
			}
			else
				printf("Robot not found\n");
			break;
		}
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
			}
			else
				printf("Robot not found\n");
			break;
		}
	case eTestFollowGod:
		{
			#ifdef _DEBUG
			UI_thread->GetUI()->SetUseSyntheticTimeDeltas(false);
			#endif
			g_WorldScaleFactor=100.0;
			game.SetDisableEngineRampUp2(true);

			Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity("TankRobot"));
			Ship_Tester *SwerveShip=dynamic_cast<Ship_Tester *>(game.GetEntity("SwerveRobot"));
			Ship_Tester *Followship=dynamic_cast<Ship_Tester *>(game.GetEntity("GodShip"));
			if (!ship)
			{
				_command.LoadRobot("TestRobot.lua","TestRobot",Commands::eTank);
				ship=dynamic_cast<Ship_Tester *>(_command.AddRobot("TankRobot","TestRobot",str_3,str_4,str_5));
			}
			if (!SwerveShip)
			{
				_command.LoadRobot("TestSwerveRobot.lua","TestSwerveRobot",Commands::eSwerve);
				SwerveShip=dynamic_cast<Ship_Tester *>(_command.AddRobot("SwerveRobot","TestSwerveRobot",str_3,str_4,str_5));
			}

			if (!Followship)
			{
				_command.LoadShip("TestShip.lua","TestShip");
				Followship=dynamic_cast<Ship_Tester *>(_command.AddShip("GodShip","TestShip",str_3,str_4,str_5));
			}
			ship->GetGameAttributes().GetTeamName()="blue";
			SwerveShip->GetGameAttributes().GetTeamName()="red";
			_.ShipFollowShip(ship,Followship,0.5,-1.0,0.5);
			_.ShipFollowShip(SwerveShip,Followship,-0.5,-1.0,5.0);
			_.GiveRobotSquareWayPointGoal(Followship);
			game.SetControlledEntity(Followship);
		}
		break;
	case eTestLUAShip:
		{
			g_WorldScaleFactor=100.0;
			game.SetDisableEngineRampUp2(true);
			_command.LoadShip("TestShip.lua","TestShip");
			Entity2D *TestEntity=_command.AddShip("GodShip","TestShip",str_3,str_4,str_5);
			_.GiveRobotSquareWayPointGoal(dynamic_cast<Ship_Tester *>(TestEntity));
			game.SetControlledEntity(TestEntity);
		}
		break;
	case eControlABomber:
		{
			Ship_Tester *ship=_.Control_A_Bomber();
			//UI_thread->GetUI()->SetCallbackInterface(&game);
			double ForceTorque=1.0*ship->GetPhysics().GetMass();
			//TestEntity->GetPhysics().ApplyFractionalTorque(ForceTorque,1.0);
			//TestEntity->GetPhysics().ApplyFractionalForce(osg::Vec2d(ForceTorque,ForceTorque),1.0);

		}
		break;
	case eFollowShipTest:
		{
			Ship_Tester *ship=_.Control_A_Bomber();
			_.GiveSquareWayPointGoal(ship);
			Ship_Tester *F1=_.Create_A_Fighter("F1");
			_.ShipFollowShip(F1,ship);
			Ship_Tester *F2=_.Create_A_Fighter("F2");
			_.ShipFollowShip(F2,ship,40);
			Ship_Tester *F3=_.Create_A_Fighter("F3");
			_.ShipFollowShip(F3,F1);
			Ship_Tester *F4=_.Create_A_Fighter("F4");
			_.ShipFollowShip(F4,F2,40);
			Ship_Tester *Snip=_.Create_A_Sniper("Sn1");
			_.ShipFollowShip(Snip,F4,0);
		}
		break;
	case eFollowPathTest:
		{
			Ship_Tester *ship=_.Control_A_Bomber();
			_.GiveSquareWayPointGoal(ship);
		}
		break;
	case ePhysicsTest:
		{
			SetUpUI(UI_thread,&game);
			Entity2D *TestEntity=NULL;
			g_TestPhysics=true;
			TestEntity=game.AddEntity("test",e_Bomber);
			g_TestPhysics=false;
			double ForceTorque=1.0*TestEntity->GetPhysics().GetMass();
			//TestEntity->GetPhysics().ApplyFractionalTorque(ForceTorque,1.0);
			TestEntity->GetPhysics().ApplyFractionalForce(osg::Vec2d(ForceTorque,ForceTorque),1.0);
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
	}
}

#define MAX_PATH          260
#pragma warning(disable : 4996)
void CommandLineInterface()
{
	UI_Controller_GameClient game;

	Commands _command(game);

	#if 0
	GUIThread *UI_thread=NULL;
	#else
	GUIThread *UI_thread=NULL;
	SetUpUI(UI_thread,&game);
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
			else if (!_strnicmp( input_line, "AddCharacter", 4))
			{
				int ShipType=atoi(str_2);
				double x=atof(str_3);
				double y=atof(str_4);
				double heading=atof(str_5);
				Entity2D *TestEntity=NULL;
				TestEntity=game.AddEntity(str_1,(Character_Type)ShipType);
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(TestEntity);
				if (ship)
				{
					ship->SetPosition(x,y);
					ship->SetAttitude(heading * (PI/180.0));
				}
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
			else if (!_strnicmp( input_line, "Join", 4))
			{
				Ship_Tester *ship=dynamic_cast<Ship_Tester *>(game.GetEntity(str_1));
				if (ship)
					ship->GetGameAttributes().GetTeamName()=str_2;
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
			else if (!_strnicmp( input_line, "RampEngine", 5))
			{
				game.SetDisableEngineRampUp2(str_1[0]=='1');
			}
			else if (!_strnicmp( input_line, "poke", 4))
			{
				double value=atof(str_1);
				g_dTest=value;
				double value2=atof(str_2);
				g_dTest2=value2;
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
	DisplayHelp();
	CommandLineInterface();
}