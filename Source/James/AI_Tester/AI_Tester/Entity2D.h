#pragma once

class Entity1D
{
	public:
		Entity1D(const char EntityName[]);

		class EventMap : public GG_Framework::UI::EventMap
		{
			public:
				EventMap(bool listOwned = false) : GG_Framework::UI::EventMap(listOwned) {}
		};

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(Entity1D::EventMap& em, const Entity1D_Properties *props=NULL);
		virtual ~Entity1D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		PhysicsEntity_1D &GetPhysics() {return m_Physics;}
		const PhysicsEntity_1D &GetPhysics() const {return m_Physics;}
		virtual double GetDimension() const {return m_Dimension;}
		virtual void ResetPos();
		// This is where both the entity and camera need to align to, by default we use the actual position
		virtual const double &GetIntendedPosition() const {return m_Position;}
		Entity1D::EventMap* GetEventMap(){return m_eventMap;}

		virtual double GetPos_m() const {return m_Position;}
	protected: 
		PhysicsEntity_1D m_Physics;
	private:
		friend GameClient; //For now the game client can set up initial settings like the dimension
		friend Entity1D_Properties;

		Entity1D::EventMap* m_eventMap;
		double m_Dimension;
		double m_Position;
		std::string m_Name;
};


class Ship_Tester;
//This contains everything the AI needs for game play; Keeping this encapsulated will help keep a clear division
//of what Entity3D looked like before applying AI with goals

//Note Entity2D should not know anything about an actor
class Entity2D : public EntityPropertiesInterface
{
	public:
		Entity2D(const char EntityName[]);

		class EventMap : public GG_Framework::UI::EventMap
		{
		public:
			EventMap(bool listOwned = false) : GG_Framework::UI::EventMap(listOwned) {}

			// The hit, the other entity, local collision point, impulse time of the collision (1 frame?)
			//Event3<Entity3D&, const osg::Vec3d&, double> Collision;
		};

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual ~Entity2D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		FlightDynamics_2D &GetPhysics() {return m_Physics;}
		const FlightDynamics_2D &GetPhysics() const {return m_Physics;}
		virtual const osg::Vec2d &GetDimensions() const {return m_Dimensions;}
		virtual void ResetPos();
		// This is where both the vehicle entity and camera need to align to, by default we use the actual orientation
		virtual const double &GetIntendedOrientation() const {return ((PosAtt *)m_PosAtt_Read.get())->m_att_r;}
		Entity2D::EventMap* GetEventMap(){return m_eventMap;}

		//from EntityPropertiesInterface
		virtual const osg::Vec2d &GetPos_m() const {return ((PosAtt *)m_PosAtt_Read.get())->m_pos_m;}
		virtual double GetAtt_r() const {return ((PosAtt *)m_PosAtt_Read.get())->m_att_r;}
	protected: 
		FlightDynamics_2D m_Physics;
	private:
		friend Ship_Tester;
		friend GameClient; //For now the game client can set up initial settings like dimensions
		friend Entity_Properties;
		struct PosAtt
		{
			osg::Vec2d m_pos_m;

			//2d Orientation:
			double m_att_r;  //a.k.a heading
			//measurement in radians where 0 points north, pi/2 = east,  pi=south, and -pi/2 (or pi + pi/2) = west
			//from this a normalized vector can easily be computed by
			//x=sin(heading) y=cos(heading)
			//We can keep the general dimensions of the entity
			//Note: we do not need pitch or roll axis so this keeps things much simpler
		} m_PosAtt_Buffers[2];

		//All read cases use the read pointer, all write cases use the write pointer followed by an interlocked exchange of the pointers
		OpenThreads::AtomicPtr m_PosAtt_Read,m_PosAtt_Write; 
		void UpdatePosAtt();

		Entity2D::EventMap* m_eventMap;

		osg::Vec2d m_Dimensions;
		std::string m_Name;
};

class RimSpace_GameAttributes
{
	public:
		//This can be dynamic as people can switch sides (Let the UI work out how to change its colors)
		std::string &GetTeamName() {return m_TeamName;}
		const std::string &GetTeamName() const {return m_TeamName;}
		//Read only... only the game client should set this initially 
		Character_Type GetCharacter_Type() const {return m_Character_Type;}
	private:
		friend GameClient;
		std::string m_TeamName;
		Character_Type m_Character_Type;
};

class Ship : public Entity2D
{
	public:
		Ship(const char EntityName[]) : Entity2D(EntityName) {}
		//Note this is technically in ThrusterShip (but it doesn't matter for our test simulation)
		//AI_Base_Controller* GetController() {return &m_Controller;}
		RimSpace_GameAttributes &GetGameAttributes() {return m_GameAttributes;}
		virtual const char *GetTeamName() const {return m_GameAttributes.GetTeamName().c_str();}
	protected:
		//Note: Only server side will aggregate this type while the client side only aggregates the base type
		//Note: It has been a struggle to find the optimum place to expose this... I originally wanted to keep this encapsulated within the controller, but
		//later found that Ideally the game client needs to populate the types and other game characteristics.  Ship is the ideal access place, as it is still
		//a part of the fringe project, and entity is too generic (i.e. a part of framework)
		RimSpace_GameAttributes m_GameAttributes;
};
