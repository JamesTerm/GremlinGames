#pragma once

class Entity1D
{
	private:
		friend class Entity1D_Properties;
		friend GameClient; //For now the game client can set up initial settings like the dimension

		GG_Framework::Base::EventMap* m_eventMap;
		double m_Dimension;
		double m_Position;
		std::string m_Name;
		bool m_BypassPos_Update;  //used to preserve pos during a ResetPos() call
	public:
		Entity1D(const char EntityName[]);

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(GG_Framework::Base::EventMap& em, const Entity1D_Properties *props=NULL);
		virtual ~Entity1D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		PhysicsEntity_1D &GetPhysics() {return m_Physics;}
		const PhysicsEntity_1D &GetPhysics() const {return m_Physics;}
		virtual double GetDimension() const {return m_Dimension;}
		virtual void ResetPos();
		// This is where both the entity and camera need to align to, by default we use the actual position
		virtual const double &GetIntendedPosition() const {return m_Position;}
		GG_Framework::Base::EventMap* GetEventMap(){return m_eventMap;}

		virtual double GetPos_m() const {return m_Position;}
		//This is used when a sensor need to correct for the actual position
		void SetPos_m(double value) {m_Position=value;}
		//Be sure to always set this back to false!
		void SetBypassPos_Update(bool bypass) {m_BypassPos_Update=bypass;}
		bool GetBypassPos_Update() const {return m_BypassPos_Update;}
	protected: 
		///This gives derived class the ability to manipulate the displacement
		/// \ret true if this is to be used and manipulated, false uses the default displacement
		virtual bool InjectDisplacement(double DeltaTime_s,double &PositionDisplacement) {return false;}

		PhysicsEntity_1D m_Physics;
		bool m_IsAngular;
};


class Ship_Tester;
//This contains everything the AI needs for game play; Keeping this encapsulated will help keep a clear division
//of what Entity3D looked like before applying AI with goals

//Note Entity2D should not know anything about an actor
class Entity2D : public EntityPropertiesInterface
{
	public:
		typedef osg::Vec2d Vec2D;

		class EventMap : public GG_Framework::UI::EventMap
		{
		public:
			EventMap(bool listOwned = false) : GG_Framework::UI::EventMap(listOwned) {}

			// The hit, the other entity, local collision point, impulse time of the collision (1 frame?)
			//Event3<Entity3D&, const osg::Vec3d&, double> Collision;
		};

	private:
		friend class Ship_Tester;
		friend class Entity_Properties;
		friend class GameClient; //For now the game client can set up initial settings like dimensions
		struct PosAtt
		{
			Vec2D m_pos_m;
	
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

		Vec2D m_DefaultPos;
		double m_DefaultAtt;
		Entity2D::EventMap* m_eventMap;
	
		Vec2D m_Dimensions;
		std::string m_Name;
		bool m_BypassPosAtt_Update;  //used to preserve pos att during a ResetPos() call

	public:
		Entity2D(const char EntityName[]);

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual ~Entity2D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		FlightDynamics_2D &GetPhysics() {return m_Physics;}
		const FlightDynamics_2D &GetPhysics() const {return m_Physics;}
		virtual const Vec2D &GetDimensions() const {return m_Dimensions;}
		virtual void ResetPos();
		// This is where both the vehicle entity and camera need to align to, by default we use the actual orientation
		virtual const double &GetIntendedOrientation() const {return ((PosAtt *)m_PosAtt_Read.get())->m_att_r;}
		Entity2D::EventMap* GetEventMap(){return m_eventMap;}

		//from EntityPropertiesInterface
		virtual const Vec2D &GetPos_m() const {return ((PosAtt *)m_PosAtt_Read.get())->m_pos_m;}
		virtual double GetAtt_r() const {return ((PosAtt *)m_PosAtt_Read.get())->m_att_r;}

		//This is the position used when ResetPos() is called
		void SetDefaultPosition(const Vec2D &pos) {m_DefaultPos=pos;}
		void SetDefaultAttitude(double att) {m_DefaultAtt=att;}
		//Be sure to always set this back to false!
		void SetBypassPosAtt_Update(bool bypass) {m_BypassPosAtt_Update=bypass;}
	protected: 
		FlightDynamics_2D m_Physics;
		///This gives derived class the ability to manipulate the displacement
		/// \ret true if this is to be used and manipulated, false uses the default displacement
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement) {return false;}
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
