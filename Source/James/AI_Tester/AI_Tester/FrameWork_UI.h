#pragma once

#if 1
const double c_Scene_XRes_InPixels=1280.0;
const double c_Scene_YRes_InPixels=1024.0;
#else
const double c_Scene_XRes_InPixels=1600.0;
const double c_Scene_YRes_InPixels=1200.0;
#endif

class EntityPropertiesInterface
{
	public:
		virtual const osg::Vec2d &GetPos_m() const =0;
		virtual double GetAtt_r() const=0;
		virtual const std::string &GetName() const=0;
		virtual const osg::Vec2d &GetDiminsions() const=0;
		virtual const double &GetIntendedOrientation() const=0;
		//I'm not sure if this would be needed in the real game, I use it so the actor knows what color to paint itself
		virtual const char *GetTeamName() const {return "";}
};

class Actor
{
	public:
		Actor(); //osg::Node &node <--may not need this
		void SetEntityProperties_Interface(EntityPropertiesInterface *entity) {m_EntityProperties_Interface=entity;}
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
		osg::Vec2d &GetCharacterDiminsions() {return m_CharacterDiminsions;}
	protected:
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw);
	private:
		std::string m_TextImage;
		std::string m_TeamName; //cache team name to avoid flooding
		osg::ref_ptr<osgText::Text> m_Text;
		osg::ref_ptr<osgText::Text> m_IntendedOrientation;
		osg::Vec2d m_CharacterDiminsions;
		double m_FontSize; //cache the last size setting to avoid flooding
};

class Viewer_Callback_Interface
{
	public:
		virtual void UpdateData(double dtime_s)=0;
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode)=0;
};

class Entity2D;
//TODO name these as content deems, we may want to have these scripted eventually... for now I'm keeping it simple
enum Character_Type
{
	e_Default_Inert,  //Does not have any goals just sits there 
	e_CaptialShip,
	e_Bomber,
	e_Fighter,
	e_Flak,
	e_Scout,
	e_Sniper,
	e_SpawnShip
};

class GameClient : public Viewer_Callback_Interface
{
	public:
		~GameClient();
		//For now use Inert type to populate obstacles (will not be a ship entity)
		virtual Entity2D *AddEntity(const char EntityName[],Character_Type Type=e_Default_Inert);
		Entity2D *GetEntity(const char EntityName[]);
		virtual void RemoveEntity(Entity2D *Entity); 
		//for testing purposes only
		void SetDisableEngineRampUp2(bool DisableRampUp);
	protected:
		virtual void UpdateData(double dtime_s);
		virtual void UpdateScene(osg::Group *rootNode,osg::Geode *geode) {}  //No UI to muck with at this level

		//Derived classes may overload AddEntity and use these instead for the same functionality
		void AddEntity(Entity2D *Entity);
		Entity2D *CreateEntity(const char EntityName[],Character_Type Type=e_Default_Inert);

		//Note: we may need to put a critical section around this during a scene update
		std::vector<Entity2D *> m_Entities;
		typedef std::vector<Entity2D *>::iterator EntityIterator;
	private:
		EventMapList m_MapList;
};

class UI_GameClient : public GameClient
{
	public:
		Entity2D *AddEntity(const char EntityName[],Character_Type Type=e_Default_Inert);
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