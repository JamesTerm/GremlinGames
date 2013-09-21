#pragma once

#if 1
#if 1
const double c_Scene_XRes_InPixels=1280.0;
const double c_Scene_YRes_InPixels=1024.0;
#else
const double c_Scene_XRes_InPixels=1680.0;
const double c_Scene_YRes_InPixels=1050.0;
#endif
#else
const double c_Scene_XRes_InPixels=1600.0;
const double c_Scene_YRes_InPixels=1200.0;
#endif

class Actor
{
	public:
		Actor(); //osg::Node &node <--may not need this
		virtual void SetEntityProperties_Interface(EntityPropertiesInterface *entity) {m_EntityProperties_Interface=entity;}
		EntityPropertiesInterface *GetEntityProperties_Interface() const {return m_EntityProperties_Interface;}
	protected:
		//Gives life to this shell dynamically
		EntityPropertiesInterface *m_EntityProperties_Interface;
		//osg::Node &m_Node;
};


class Actor_Text : public Actor, public osg::Drawable::UpdateCallback
{
	public:
		~Actor_Text();
		Actor_Text(const char TextImage[]="X");
		//call this if you want intended orientation graphics (after setting up the character dimensions)
		void Init_IntendedOrientation();
		osg::ref_ptr<osgText::Text> GetText() {return m_Text;}
		osg::ref_ptr<osgText::Text> GetIntendedOrientation() {return m_IntendedOrientation;}
		std::string &GetTextImage() {return m_TextImage;}
		//For now just have the client code write these
		osg::Vec2d &GetCharacterDimensions() {return m_CharacterDimensions;}
		double GetFontSize() const {return m_FontSize;}
		virtual void SetEntityProperties_Interface(EntityPropertiesInterface *entity);
		virtual void UpdateScene_Additional (osg::Geode *geode, bool AddOrRemove);
	protected:
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw);
	private:
		std::string m_TextImage;
		std::string m_TeamName; //cache team name to avoid flooding
		osg::ref_ptr<osgText::Text> m_Text;
		osg::ref_ptr<osgText::Text> m_IntendedOrientation;
		//Here is a quick reference on the character layout, used to determine size against the real dimensions, and also to center the intended orientation
		//caret for ships
		osg::Vec2d m_CharacterDimensions;
		double m_FontSize; //cache the last size setting to avoid flooding
};

class Viewer_Callback_Interface
{
	public:
		virtual void UpdateData(double dtime_s)=0;
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode)=0;
};

class Entity2D;

class GameClient : public Viewer_Callback_Interface
{
	public:
		~GameClient();
		//For now use Inert type to populate obstacles (will not be a ship entity)
		Entity2D *GetEntity(const char EntityName[]);
		virtual void RemoveEntity(Entity2D *Entity);
		void RemoveAllEntities();
	protected:
		virtual void UpdateData(double dtime_s);
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode) {}  //No UI to muck with at this level

		//Derived classes may overload AddEntity and use these instead for the same functionality
		void AddEntity(Entity2D *Entity);
		Entity2D *CreateEntity(const char EntityName[],const Entity_Properties &props);

		//Note: we may need to put a critical section around this during a scene update
		std::vector<Entity2D *> m_Entities;
		typedef std::vector<Entity2D *>::iterator EntityIterator;
	private:
		EventMapList m_MapList;
};

class UI_GameClient : public GameClient
{
	public:
		Entity2D *AddEntity(const char EntityName[],const Entity_Properties &props);
		void RemoveEntity(Entity2D *Entity); 
		void RemoveEntity(const char EntityName[]); 
	protected:
		//This only manages adding and removing nodes not their positions
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode);
		virtual void AboutTo_RemoveEntity(Entity2D *Entity) {}
	private:
		OpenThreads::Mutex m_BlockActorLists;
		//The new and old actor lists are 
		std::vector<osg::ref_ptr<Actor_Text>> m_Actors,m_NewActors,m_OldActors;
		typedef std::vector<osg::ref_ptr<Actor_Text>>::iterator ActorIterator;
};

class UI_Controller_GameClient : public UI_GameClient
{
	public:
		UI_Controller_GameClient();
		~UI_Controller_GameClient();
		virtual void SetControlledEntity(Entity2D* newEntity,bool AddJoystickDefaults=true);
		virtual void AboutTo_RemoveEntity(Entity2D *Entity) {if (Entity==m_controlledEntity) SetControlledEntity(NULL);}
	private:
		//The one, the only!
		UI_Controller *m_UI_Controller;  //unfortunately this is late binding once the window is setup
		Entity2D* m_controlledEntity;
};
