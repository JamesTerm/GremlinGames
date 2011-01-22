#pragma once

const unsigned AI_DRIVE_AVG_LEN = 10;

namespace Fringe
{
	namespace Base
	{

		struct ThrustShip2_TransmittedEntity;

///This class contains all the common code that AI and UI controllers may need (e.g. resources).  
///(It may also contain common code for remote too).  
class FRINGE_BASE_API AI_Base_Controller
{
public:
	AI_Base_Controller(ThrustShip2& ship);
	~AI_Base_Controller(){ResetAI_Reaction();}

	void InitializeEventMaps(bool canUserPilot); //Hooks up the event map callbacks called from ship once it has them

	/// This version of the UpdateController is ONLY called when the ship is NOT being piloted,
	/// Classes that inherit from this can call this function when they are not piloting
	/// To allow the AI to pilot.
	virtual void UpdateController(double dTime_s);
	virtual void ReadScriptParameters(ThrustShip2_TransmittedEntity& te);
	virtual void ResetPos(){m_currWayPointIndex = 0;}


	bool HasAutoPilotRoute();
	bool GetCanUserPilot();
	virtual void CancelAllControls();

	ThrustShip2& GetShip(){return m_ship;}

	//This is mostly for pass-thru since the timing of this is in alignment during a ship att-pos update
	void UpdateUI(double dTime_s);

	GG_Framework::AppReuse::DestroyableEntity* GetTarget();
	const osg::Vec3d& GetTargetLeadPoint() {return m_targetLeadPt;}
	unsigned GetNumHits() const {return m_numHits;}
	unsigned GetNumKills() const {return m_numKills;}

	void IncrementKills(){++m_numKills;}
	void IncrementHits(){++m_numHits;}

	// Call this to set a new target
	void SetTarget(GG_Framework::AppReuse::DestroyableEntity* newTarget);

	/// I put the word try, as there may be some extra logic to determine if it has permission
	/// This is a bit different than viewing an AI with no controls, where it simply does not
	/// Allow a connection
	/// \return true if it was allowed to bind
	virtual bool Try_SetUIController(UI_Controller *controller);
	bool IsUIControlled() {return m_UI_Controller!=NULL;}

	void TargetNext(){SelectTarget(1);}
	void TargetPrev(){SelectTarget(-1);}
	void TargetTgtYou();
	void TargetInSight();

	GG_Framework::AppReuse::DestroyableEntity* TargetInFront(double& closestHit, double& greatestCosAngle);

	// Using the Target Bank (numbered for use in key bindings
	void SetTargetBank1(){m_targetBank[1]=GetTarget();}
	void SetTargetBank2(){m_targetBank[2]=GetTarget();}
	void SetTargetBank3(){m_targetBank[3]=GetTarget();}
	void SetTargetBank4(){m_targetBank[4]=GetTarget();}
	void SetTargetBank5(){m_targetBank[5]=GetTarget();}
	void SetTargetBank6(){m_targetBank[6]=GetTarget();}
	void SetTargetBank7(){m_targetBank[7]=GetTarget();}
	void SetTargetBank8(){m_targetBank[8]=GetTarget();}
	void SetTargetBank9(){m_targetBank[9]=GetTarget();}

	void UseTargetBank1(){SetTarget(m_targetBank[1]);}
	void UseTargetBank2(){SetTarget(m_targetBank[2]);}
	void UseTargetBank3(){SetTarget(m_targetBank[3]);}
	void UseTargetBank4(){SetTarget(m_targetBank[4]);}
	void UseTargetBank5(){SetTarget(m_targetBank[5]);}
	void UseTargetBank6(){SetTarget(m_targetBank[6]);}
	void UseTargetBank7(){SetTarget(m_targetBank[7]);}
	void UseTargetBank8(){SetTarget(m_targetBank[8]);}
	void UseTargetBank9(){SetTarget(m_targetBank[9]);}

	void MatchTargetSpeed();

protected:
	friend UI_Controller;
	UI_Controller *m_UI_Controller;
	IEvent::HandlerList ehl;
	ThrustShip2& m_ship;
	std::vector<WayPoint> WayPoints;
	FollowShip ShipToFollow;
	FollowShip TargetShip;

	osg::Vec3d m_targetLeadPt;
	void UpdateTargetLeadPoint();

	// Does not send the change along the network, but sets up the other changes
	void InnerSetTarget(GG_Framework::AppReuse::DestroyableEntity* newTarget);
	void NetworkSetTarget(double targetID);

	// Tells the AI to drive to a location, but it will not happen immediately, 
	// Based on AI Reaction Time, etc.
	// The values passed in here are the perfect values, not corrupted by perception
	void UpdateIntendedLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, osg::Vec3d* matchVel);


	void SetShipSpeed(double speed_mps);

	// Call this to setup the target and follow if needed
	void UpdateTargetAndFollow();

	void UpdateController_Base(double dTime_s);

	//TODO these may be moved to the ships sensors area
	std::string LEAD_RET_OSGV;
	std::string INRANGE_LEAD_RET_OSGV;
	std::string FWD_RET_OSGV;

	// We can try to avoid collisions in trajectories, level 2 and above
	// We do this here so we can more easily use the internal map
	// Called from DriveToLocation to avoid collisions with other entities
	// If there is nothing in between to worry
	// about colliding with, intendedPos will not change and the return value will be false.
	// If there may be a collision, the intendedPos is altered and the function returns true
	bool TryAvoidCollision(osg::Vec3d& intendedPos);

	double GetRandomOversteerFactor();
	void GetTurningReactionTimes(double& minTime_s, double& timeTill90_s);


private:
	class AI_Reaction
	{
	private:
		AI_Base_Controller& m_controller;
		osg::Vec3d m_globalTrajectoryPoint;
		osg::Vec3d m_shipStartingPos;

		double m_power;
		osg::Vec3d m_matchVel;
		bool m_shouldMatchVel;
		bool m_readyToFly;

		double m_timeSinceLastUpdate;
		double m_timeTillUpdateOK;

		osg::Quat m_startingQuat;
		osg::Quat m_endingQuat;
		double m_startingDist;
		double m_endingDist;

		osg::Vec3d m_startingPositionPoint;
		osg::Vec3d m_endingPositionPoint;

		double m_oversteer;

	public:
		AI_Reaction(AI_Base_Controller& controller);

		void UpdateIntendedLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, osg::Vec3d* matchVel);
		void FlyToNextLocation(double dTime_s);
		bool ReadyForNextUpdate();
		void Reset();
	};
	AI_Reaction m_AI_Reaction;
	friend AI_Reaction;
	void UpdateAI_Reaction(double dTime_s);
	bool HandleGs(double dTime_s);
	bool FlyWayPoints(double dTime_s);
	void ResetAI_Reaction();

	virtual void OnCollision(GG_Framework::Logic::Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s);
	osg::Vec3d* m_jinkFromCollision;

	/// \param TrajectoryPoint- This is the point that your nose of your ship will orient to from its current position (usually the same as PositionPoint)
	/// \param PositionPoint- This is the point where your ship will be to position to (Usually the same as TrajectoryPoint)
	/// \power- The scaled value multiplied to the ships max speed.  If > 1 it can be interpreted as explicit meters per second speed
	/// \matchVel- You can make it so the velocity of the ship when it reaches the point.  
	/// Use NULL when flying through way-points
	/// Use (0,0,0) if you want to come to a stop, like at the end of a way-point series
	/// Otherwise, use the velocity of the ship you are targeting or following to keep up with it
	void DriveToLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, double dTime_s,osg::Vec3d* matchVel);

	void AI_TryFireCannons(GG_Framework::AppReuse::DestroyableEntity* target);
	bool TrackFollowShip(FollowShip& followMe);
	bool TrackTarget(FollowShip& targetFollow);

	unsigned m_currWayPointIndex;
	bool m_firingCannons;
	Averager<double, AI_DRIVE_AVG_LEN> m_speedAverager;

	// A bank of easily selected objects
	GG_Framework::AppReuse::DestroyableEntity* m_targetBank[10];

	// Use -1 for back, 1 for forward
	void SelectTarget(int dir);

	unsigned m_numHits, m_numKills;
	bool m_AI_recoverGs;

	// Called back from the event handler
	virtual void TargetDestroyed(const osg::Vec3d& collisionPt){IncrementKills();}
	virtual void ShipDestroyed(const osg::Vec3d& collisionPt);
	virtual void OnCannonRoundHit(GG_Framework::Logic::Entity3D* otherEntity);

	// Some ships cannot be controlled at ALL, and are ALWAYS in auto-pilot
	bool m_canUserPilot;

	// The internal function for avoiding collisions
	bool TryAvoidCollision(double thisMaxRad, osg::Vec3d& intendedPos, EntityMap_NetID::iterator it);
};	

	}
}
