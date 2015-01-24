#include "stdafx.h"
#include "Robot_Tester.h"
#include "Robots.h"

using namespace Robot_Tester;
using namespace std;
using namespace GG_Framework::Base;

//This is dynamic so that we can zoom in on the fly
//double g_WorldScaleFactor=0.01; //This will give us about 128,000 x 102,400 meters resolution before wrap around
//double g_WorldScaleFactor=0.1; //This will give us about 12,800 x 10,240 meters resolution before wrap around ideal size to see
//double g_WorldScaleFactor=1.0; //This only give 1280 x 1024 meters but ideal to really see everything
double g_WorldScaleFactor=2.0; //This only give 640 x 512 good for windowed mode
bool g_TestPhysics=false;

  /***********************************************************************************************************************************/
 /*																Actor																*/
/***********************************************************************************************************************************/

Actor::Actor() : m_EntityProperties_Interface(NULL)
{
}

#if 0
void Actor::operator()(osg::Node *node,osg::NodeVisitor *nv)
{
	if (m_EntityProperties_Interface)
	{
		//Note copy it for the vec... since it is volitile there is only one read to it now
		osg::Vec2d Position=m_EntityProperties_Interface->GetPos_m();
		double Heading=m_EntityProperties_Interface->GetAtt_r();

		//TODO figure out if this works for text and which coordinates to apply
		//also figure out the rotation quat conversion
		osg::PositionAttitudeTransform *pat=dynamic_cast<osg::PositionAttitudeTransform *>(node);
		if (pat)
		{
			pat->setPosition(osg::Vec3d(Position[0],0.0,Position[1]));
		}
		//TODO we may want to scale down position here first
		//Bounds check, we'll wrap around the position by applying a modulo on the resolution
		//Position[0]=Position[0]%c_Scene_XRes_InPixels;
		//Position[1]=Position[1]%c_Scene_XRes_InPixels;
	}
	traverse(node,nv);
}
#endif

  /***********************************************************************************************************************************/
 /*																Actor_Text															*/
/***********************************************************************************************************************************/
const double PI=3.1415926535897;
const double c_halfxres=c_Scene_XRes_InPixels/2.0;
const double c_halfyres=c_Scene_YRes_InPixels/2.0;

Actor_Text::~Actor_Text()
{
	//Keeping destructor for debugging purposes (make sure stuff is getting deleted)
}

Actor_Text::Actor_Text(const char TextImage[]) : Actor(),m_TextImage(TextImage)  
{
	m_Text = new osgText::Text;
	m_FontSize = 20.0;
	osg::Vec4 layoutColor(1.0f,1.0f,1.0f,1.0f);
	//Seems like the default font is ideal for me for now
	//osgText::Font* font = osgText::readFontFile("fonts/VeraMono.ttf");
	//m_Text->setFont(font);
	m_Text->setColor(layoutColor);
	m_Text->setCharacterSize(m_FontSize);
	m_Text->setFontResolution(10,10);

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_Text->setPosition(position);
	//text->setDrawMode(osgText::Text::TEXT|osgText::Text::BOUNDINGBOX);
	m_Text->setAlignment(osgText::Text::CENTER_CENTER);
	m_Text->setText(TextImage);
	m_Text->setUpdateCallback(this);

}
void Actor_Text::Init_IntendedOrientation()
{
	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_IntendedOrientation= new osgText::Text;
	m_IntendedOrientation->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
	m_IntendedOrientation->setCharacterSize(m_FontSize);
	m_IntendedOrientation->setFontResolution(10,10);
	m_IntendedOrientation->setPosition(position);
	m_IntendedOrientation->setAlignment(osgText::Text::CENTER_CENTER);
	{
		char IntendedImage[128];
		strcpy(IntendedImage,"^\n");
		for (size_t i=0;i<m_CharacterDimensions.y();i++)
			strcat(IntendedImage," \n");
		m_IntendedOrientation->setText(IntendedImage);
	}
	m_IntendedOrientation->setUpdateCallback(this);
}

void Actor_Text::UpdateScene_Additional (osg::Geode *geode, bool AddOrRemove)
{
	if (m_EntityProperties_Interface)
		m_EntityProperties_Interface->UpdateScene(geode,AddOrRemove);
}

void Actor_Text::SetEntityProperties_Interface(EntityPropertiesInterface *entity)
{
	__super::SetEntityProperties_Interface(entity);
	assert(m_EntityProperties_Interface);
	m_EntityProperties_Interface->UI_Init(this);
}

void Actor_Text::update(osg::NodeVisitor *nv, osg::Drawable *draw)
{
	if (m_EntityProperties_Interface)
	{
		osgText::Text *Text=dynamic_cast<osgText::Text *>(draw);
		if (Text)
		{
			//Note copy it for the vec... since it is volatile there is only one read to it now
			osg::Vec2d Position=m_EntityProperties_Interface->GetPos_m();
			double Heading=m_EntityProperties_Interface->GetAtt_r();
			double IntendedOrientation=m_EntityProperties_Interface->GetIntendedOrientation();

			//Scale down position here first
			Position[0]*=g_WorldScaleFactor;
			Position[1]*=g_WorldScaleFactor;

			//Center the offset
			Position[0]+=c_halfxres;
			Position[1]+=c_halfyres;
			//Now to wrap around the resolution
			Position[0]-=floor(Position[0]/c_Scene_XRes_InPixels)*c_Scene_XRes_InPixels;
			Position[1]-=floor(Position[1]/c_Scene_YRes_InPixels)*c_Scene_YRes_InPixels;
			//DOUT1 ("%f %f",Position[0],Position[1]);

			osg::Vec3 pos(Position[0],Position[1],0.0f);

			Text->setPosition(pos);
			//Using negative so that radians increment in a clockwise direction //e.g. 90 is to the right
			Text->setRotation(FromLW_Rot_Radians(-Heading,0.0,0.0));

			if (m_IntendedOrientation.valid())
			{
				m_IntendedOrientation->setPosition(pos);
				m_IntendedOrientation->setRotation(FromLW_Rot_Radians(-IntendedOrientation,0.0,0.0));
			}
			m_EntityProperties_Interface->custom_update(nv,draw,pos);

			{//Now to determine the size the setCharacterSize is roughly one pixel per meter with the default font
				double XSize=m_EntityProperties_Interface->GetDimensions()[0] / m_CharacterDimensions[0] * g_WorldScaleFactor;
				double YSize=m_EntityProperties_Interface->GetDimensions()[1] / m_CharacterDimensions[1] * g_WorldScaleFactor;
				double SizeToUse=max(XSize,YSize); //we want bigger always so it is easier to see
				SizeToUse=max(SizeToUse,5.0);  //Anything smaller than 5 is not visible


				if (SizeToUse!=m_FontSize)
				{
					//TODO this should be working... it did once... need to figure out clean way to alter it
					Text->setCharacterSize(SizeToUse);
					if (m_IntendedOrientation.valid())
						m_IntendedOrientation->setCharacterSize(SizeToUse);
					m_EntityProperties_Interface->Text_SizeToUse(SizeToUse);
					m_FontSize=SizeToUse;
				}
			}
			#if 0
			{ //Now to determine the color based on the team name
				if (m_TeamName!=m_EntityProperties_Interface->GetTeamName())
				{
					m_TeamName=m_EntityProperties_Interface->GetTeamName();
					if (m_TeamName=="red")
						Text->setColor(osg::Vec4(1.0f,0.0f,0.5f,1.0f));  //This is almost magenta (easier to see)
					else if (m_TeamName=="blue")
						Text->setColor(osg::Vec4(0.0f,0.5f,1.0f,1.0f));  //This is almost cyan (easier to see too)
					else if (m_TeamName=="green")
						Text->setColor(osg::Vec4(0.0f,1.0f,0.5f,1.0f)); 
				}
			}
			#endif
		}
	}
}
  /***********************************************************************************************************************************/
 /*																GameClient															*/
/***********************************************************************************************************************************/

void GameClient::RemoveAllEntities()
{
	for (size_t i=0;i<m_Entities.size();i++)
		delete m_Entities[i];
	m_Entities.clear();
}

GameClient::~GameClient()
{
	RemoveAllEntities();
}

Entity2D *GameClient::CreateEntity(const char EntityName[],const Entity_Properties &props)
{
	Entity2D *NewEntity=NULL;
	const Ship_Properties *ship_props=dynamic_cast<const Ship_Properties *>(&props);
	if (ship_props)
	{
		Ship_2D *NewShip=NULL;

		switch(ship_props->GetShipType())
		{
			case Ship_Props::eDefault:
				NewShip=new Ship_Tester(EntityName);
				break;
			case Ship_Props::eRobotTank:
				NewShip=new Tank_Robot_UI_Control(EntityName);
				break;
			case Ship_Props::eSwerve_Robot:
				NewShip=new Swerve_Robot_UI_Control(EntityName);
				break;
			case Ship_Props::eButterfly_Robot:
				NewShip=new Butterfly_Robot_UI_Control(EntityName);
				break;
			case Ship_Props::eNona_Robot:
				NewShip=new Nona_Robot_UI_Control(EntityName);
				break;
			case Ship_Props::eFRC2011_Robot:
				NewShip=new FRC_2011_Robot_UI(EntityName);
				break;
			case Ship_Props::eFRC2012_Robot:
				NewShip=new FRC_2012_Robot_UI(EntityName);
				break;
			case Ship_Props::eFRC2013_Robot:
				NewShip=new FRC_2013_Robot_UI(EntityName);
				break;
			case Ship_Props::eFRC2014_Robot:
				NewShip=new FRC_2014_Robot_UI(EntityName);
				break;
			case Ship_Props::eFRC2015_Robot:
				NewShip=new FRC_2015_Robot_UI(EntityName);
				break;
			case Ship_Props::eHikingViking_Robot:
				NewShip=new HikingViking_Robot_UI(EntityName);
				break;
		}
		assert(NewShip);
		NewEntity=NewShip;
	}
	else
	{
		NewEntity=new Entity2D(EntityName);

	}

	assert(NewEntity);

	Entity2D::EventMap* newEm = new Entity2D::EventMap(true);
	m_MapList.push_back(newEm);
	NewEntity->Initialize(*newEm,ship_props);
	return NewEntity;

}

void GameClient::AddEntity(Entity2D *Entity)
{
	m_Entities.push_back(Entity);
}

Entity2D *GameClient::GetEntity(const char EntityName[])
{
	Entity2D *ret=NULL;
	for (EntityIterator i=m_Entities.begin();i!=m_Entities.end();i++)
	{
		if ((stricmp((*i)->GetName().c_str(),EntityName)==0))
		{
			ret=(*i);
			break;
		}
	}
	return ret;
}

void GameClient::RemoveEntity(Entity2D *Entity)
{
	for (EntityIterator i=m_Entities.begin();i!=m_Entities.end();i++)
	{
		if ((*i)==Entity)
		{
			m_Entities.erase(i);
			delete Entity;
			break;
		}
	}
}

void GameClient::UpdateData(double dtime_s)
{
	for (size_t i=0;i<m_Entities.size();i++)
		m_Entities[i]->TimeChange(dtime_s);
}

  /***********************************************************************************************************************************/
 /*																UI_GameClient														*/
/***********************************************************************************************************************************/


Entity2D *UI_GameClient::AddEntity(const char EntityName[],const Entity_Properties &props)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_BlockActorLists);
	Entity2D *NewEntity=CreateEntity(EntityName,props);
	__super::AddEntity(NewEntity); //Added to game client
	const char *TextImage=NULL;
	osg::Vec2d Dimension;

	const UI_Ship_Properties *ui_ship=dynamic_cast<const UI_Ship_Properties *>(&props);
	assert(ui_ship);  //this is all we have for now
	ui_ship->Initialize(&TextImage,Dimension);

	assert(TextImage);
	osg::ref_ptr<Actor_Text> NewActor=new Actor_Text(TextImage);
	NewActor->GetCharacterDimensions()=Dimension;

	//This can be removed if we do not want to see this image
	if (ui_ship)
		NewActor->Init_IntendedOrientation();
	m_NewActors.push_back(NewActor);
	//Bind the Entity with its actor
	NewActor->SetEntityProperties_Interface(NewEntity);
	return NewEntity;
}

void UI_GameClient::RemoveEntity(Entity2D *Entity)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_BlockActorLists);
	EntityPropertiesInterface *Entity_Interface=Entity;  //I can just cast down
	for (ActorIterator i=m_Actors.begin();i!=m_Actors.end();i++)
	{
		osg::ref_ptr<Actor_Text> entry=(*i);
		if (Entity_Interface==entry->GetEntityProperties_Interface())
		{
			AboutTo_RemoveEntity(Entity);
			m_OldActors.push_back(entry); //push to the old list where it will be removed from the scene
			m_Actors.erase(i);
			break;
		}
	}
}

void UI_GameClient::RemoveEntity(const char EntityName[])
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_BlockActorLists);
	for (ActorIterator i=m_Actors.begin();i!=m_Actors.end();i++)
	{
		osg::ref_ptr<Actor_Text> entry=(*i);
		if (string(EntityName)==entry->GetEntityProperties_Interface()->GetName())
		{
			AboutTo_RemoveEntity(dynamic_cast<Entity2D *>(entry->GetEntityProperties_Interface()));
			m_OldActors.push_back(entry); //push to the old list where it will be removed from the scene
			m_Actors.erase(i);
			break;
		}
	}
}

//This only manages adding and removing nodes not their positions
void UI_GameClient::UpdateScene(osg::Group *rootNode,osg::Geode *geode)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(m_BlockActorLists);
	//first lets remove nodes
	if (m_OldActors.size())
	{
		for (size_t i=0;i<m_OldActors.size();i++)
		{
			geode->removeDrawable(m_OldActors[i]->GetText());
			if (m_OldActors[i]->GetIntendedOrientation().valid())
				geode->removeDrawable(m_OldActors[i]->GetIntendedOrientation());
			__super::RemoveEntity(dynamic_cast<Entity2D *>(m_OldActors[i]->GetEntityProperties_Interface()));  //remove from super's list
			m_NewActors[i]->UpdateScene_Additional(geode,false); //remove any additional nodes
		}
		m_OldActors.clear();
	}
	//now lets add new ones
	if (m_NewActors.size())
	{
		for (size_t i=0;i<m_NewActors.size();i++)
		{
			geode->addDrawable(m_NewActors[i]->GetText());
			if (m_NewActors[i]->GetIntendedOrientation().valid())
				geode->addDrawable(m_NewActors[i]->GetIntendedOrientation());
			m_NewActors[i]->UpdateScene_Additional(geode,true); //Add any additional nodes
			m_Actors.push_back(m_NewActors[i]); //Now this is in the list of active actors
		}
		m_NewActors.clear();
	}
}

UI_Controller_GameClient::UI_Controller_GameClient() : m_controlledEntity(NULL),m_UI_Controller(NULL)
{
}

UI_Controller_GameClient::~UI_Controller_GameClient()
{
	delete m_UI_Controller;
	m_UI_Controller=NULL;
}

void UI_Controller_GameClient::SetControlledEntity(Entity2D* newEntity,bool AddJoystickDefaults)
{
	//printf("UI_GameClient::SetControlledEntity\n");
	if (newEntity != m_controlledEntity)
	{
		//Disconnect the UI first
		if (m_controlledEntity)
		{
			m_UI_Controller->HookUpUI(false);
			AI_Base_Controller *OldController=NULL;
			{
				Ship_2D *oldship=dynamic_cast<Ship_2D *>(m_controlledEntity);
				OldController=(oldship)?oldship->GetController():NULL;
			}
			//Detach association from the old controller
			if (OldController)
				OldController->Try_SetUIController(NULL);
		}

		//This has been moved down so that I can detach to nothing

		//Associate our controller to the AI controller
		Ship_2D *ship=dynamic_cast<Ship_2D *>(newEntity);
		if (!ship) 
		{
			m_UI_Controller->Set_AI_Base_Controller(NULL);
			m_controlledEntity=NULL;
			return;
		}
		AI_Base_Controller *controller=ship->GetController();
		assert(controller);

		//We set this when we need it
		if (!m_UI_Controller)
			m_UI_Controller= new UI_Controller(NULL,AddJoystickDefaults);

		// JAMES: Not sure of the error handling here.  Can Try_SetUIController fail? if so, why do Set_AI_Base_Controller
		// Only to have to set it back to NULL?  What does it mean to set it to NULL?  Is this an invalid state?

		//Rick:  Try_SetUIController currently does not ever fail.  So everything is safe, the real question is to determine what ships we can
		//fly to and at what level do we manage this.  This is somewhat related to the idea of only being able to have access to the team's ships, or
		//full access for people observing the game.  There may be other logic such as another player has control of the ship etc.
		//If these things are managed elsewhere then we can change the method to not return bool.  If it is determined that this would be the level
		//to manage this we can add a method to check before doing the pointer assignments.  I suspect access might be managed elsewhere.

		m_UI_Controller->Set_AI_Base_Controller(controller);
		//Now see if the AI will allow us to use it
		if (controller->Try_SetUIController(m_UI_Controller))
		{
			//Success... now to let the entity set things up
			m_UI_Controller->HookUpUI(true);

			//__super::SetControlledEntity(newEntity);
			{
				if (newEntity != m_controlledEntity)
				{
					//Entity2D* oldEntity = m_controlledEntity;
					//if (m_controlledEntity)
					//	m_controlledEntity->SetPlayerControlled(false);
					m_controlledEntity = newEntity;
					//if (m_controlledEntity)
					//	m_controlledEntity->SetPlayerControlled(true);
					//ControlledEntityChanged.Fire(oldEntity, newEntity);
				}
			}
		}
		else
			m_UI_Controller->Set_AI_Base_Controller(NULL);   //no luck... flush ship association
	}

}