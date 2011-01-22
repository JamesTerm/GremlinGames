#pragma once

namespace GuardianVR
{
	namespace Base
	{

		// struct Mech_TransmittedEntity;

///This class contains all the common code that AI and UI controllers may need (e.g. resources).  
///(It may also contain common code for remote too).  
class GUARDIANVR_BASE_API Mech_ControllerBase
{
public:
	Mech_ControllerBase(Mech& mech);

	/// This version of the UpdateController is ONLY called when the mech is NOT being piloted,
	/// Classes that inherit from this can call this function when they are not piloting
	/// To allow the AI to pilot.
	virtual void UpdateController(double dTime_s);
	// virtual void ReadScriptParameters(Mech_TransmittedEntity& te);

	virtual void CancelAllControls(){}
	virtual void ResetPos(){}

	Mech& GetMech(){return m_mech;}

	unsigned GetNumHits() const {return m_numHits;}
	unsigned GetNumKills() const {return m_numKills;}

	void IncrementKills(){++m_numKills;}
	void IncrementHits(){++m_numHits;}

	// Working with targets (NOT sure how they will be set yet, but this handles the events)
	void SetTarget(GG_Framework::AppReuse::DestroyableEntity* newTarget);
	GG_Framework::AppReuse::DestroyableEntity* GetTarget(){return m_target;}

protected:
	IEvent::HandlerList ehl;
	Mech& m_mech;

private:
	bool m_firingCannons;
	unsigned m_numHits, m_numKills;
	GG_Framework::AppReuse::DestroyableEntity* m_target;

	// Called back from the event handler
	virtual void TargetDestroyed(const osg::Vec3d& collisionPt){IncrementKills();}
	virtual void MechDestroyed(const osg::Vec3d& collisionPt);
	virtual void OnCannonRoundHit(GG_Framework::Logic::Entity3D* otherEntity);
};	

	}
}
