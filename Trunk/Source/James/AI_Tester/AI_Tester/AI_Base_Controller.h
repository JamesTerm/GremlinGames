#pragma once



class AI_Base_Controller
{
	public:
		AI_Base_Controller(Ship_2D &ship);

		///This is the single update point to all controlling of the ship.  The base class contains no goal arbitration, but does implement
		///Whatever goal is passed into it if the UI controller is off line
		virtual void UpdateController(double dTime_s);

		//This is mostly for pass-thru since the timing of this is in alignment during a ship att-pos update
		void UpdateUI(double dTime_s);

		/// I put the word try, as there may be some extra logic to determine if it has permission
		/// This is a bit different than viewing an AI with no controls, where it simply does not
		/// Allow a connection
		/// \return true if it was allowed to bind
		virtual bool Try_SetUIController(UI_Controller *controller);
		virtual void ResetPos() {}

		bool HasAutoPilotRoute() {return true;}
		bool GetCanUserPilot() {return true;}

		void SetShipVelocity(double velocity_mps) {m_ship.SetRequestedVelocity(velocity_mps);}

		/// \param TrajectoryPoint- This is the point that your nose of your ship will orient to from its current position (usually the same as PositionPoint)
		/// \param PositionPoint- This is the point where your ship will be to position to (Usually the same as TrajectoryPoint)
		/// \power- The scaled value multiplied to the ships max speed.  If > 1 it can be interpreted as explicit meters per second speed
		/// \matchVel- You can make it so the velocity of the ship when it reaches the point.  
		/// Use NULL when flying through way-points
		/// Use (0,0) if you want to come to a stop, like at the end of a way-point series
		/// Otherwise, use the velocity of the ship you are targeting or following to keep up with it
		void DriveToLocation(osg::Vec2d TrajectoryPoint,osg::Vec2d PositionPoint, double power, double dTime_s,osg::Vec2d* matchVel);
		
		Ship_2D &GetShip() {return m_ship;}
	protected:
		friend Ship_Tester;
		Goal *m_Goal; //Dynamically set a goal for this controller
	private:
		friend UI_Controller;
		UI_Controller *m_UI_Controller;

		//TODO determine way to properly introduce UI_Controls here	
		Ship_2D &m_ship;
};

//TODO get these functions re-factored
//Update Reaction->FlyWayPoints->UpdateIndendedLocation
//FlyToNextLocation->DriveToLocation

struct WayPoint
{
	WayPoint() : Power(0.0), Position(0,0),TurnSpeedScaler(1.0) {}
	double Power;
	double TurnSpeedScaler;  //This will have a default value if not in script
	osg::Vec2d Position;
};

//This is similar to Traverse_Edge in book (not to be confused with its MoveToPosition)
class Goal_Ship_MoveToPosition : public AtomicGoal
{
	public:
		Goal_Ship_MoveToPosition(AI_Base_Controller *controller,const WayPoint &waypoint,bool UseSafeStop=true);
		~Goal_Ship_MoveToPosition();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();

	protected:
		//Similar to FlyWayPoints, except it only checks for the tolerance
		bool HitWayPoint();  

	private:
		AI_Base_Controller * const m_Controller;
		WayPoint m_Point;
		Ship_2D &m_ship;
		bool m_Terminate;
		bool m_UseSafeStop;
};

class Goal_Ship_FollowPath : public CompositeGoal
{
	public:
		Goal_Ship_FollowPath(AI_Base_Controller *controller,std::list<WayPoint> path,bool LoopMode=false);
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();
	private:
		AI_Base_Controller * const m_Controller;
		std::list<WayPoint> m_Path,m_PathCopy;
		bool m_LoopMode;
};

class Goal_Ship_FollowShip : public AtomicGoal
{
public:
	Goal_Ship_FollowShip(AI_Base_Controller *controller,const Ship_2D &Followship,const osg::Vec2d &RelPosition);
	~Goal_Ship_FollowShip();
	virtual void Activate();
	virtual Goal_Status Process(double dTime_s);
	virtual void Terminate();
	//Allow client to change its relative position dynamically
	void SetRelPosition(const osg::Vec2d &RelPosition);
private:
	AI_Base_Controller * const m_Controller;
	osg::Vec2d m_RelPosition,m_TrajectoryPosition;
	Ship_2D &m_ship;
	const Ship_2D &m_Followship;
	bool m_Terminate;
};


//This goal simply will fire an event when all goals are complete
class Goal_NotifyWhenComplete : public CompositeGoal
{
public:
	Goal_NotifyWhenComplete(GG_Framework::Base::EventMap &em,char *EventName);
	//give public access for client to populate goals
	virtual void AddSubgoal(Goal *g) {__super::AddSubgoal(g);}
	//client activates manually when goals are added
	virtual void Activate();
	virtual Goal_Status Process(double dTime_s);
	virtual void Terminate();

private:
	std::string m_EventName;  //name to fire when complete
	GG_Framework::Base::EventMap &m_EventMap;
};

class AI_Controller : public AI_Base_Controller
{
	public:
	private:
};
