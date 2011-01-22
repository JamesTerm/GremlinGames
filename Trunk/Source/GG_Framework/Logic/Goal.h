#pragma once


namespace GG_Framework
{
	namespace Logic
	{

/// These classes reflect the design on page 385 in the "Programming Game AI by Example" book.  There are some minor changes/improvements from
/// What is written, but perhaps the biggest design decision to consider is if they need to be templated with an entity type.  I am going to 
/// need to run through some simulation code to know for sure, but for now I am going to steer away from doing that! The actual implementation
/// of some of these methods will reside in a cpp file.  Once the decision is final they will either stay in a cpp, or change to an 'hpp' file
/// using a nested include technique to work with a templated class.  Hopefully this will not be necessary.
//  [2/23/2010 James]
class Goal
{
	public:
		///This contains initialization logic and represents the planning phase of the goal.  A Goal is able to call its activate method
		///any number of times to re-plan if the situation demands.
		virtual void Activate()=0;

		enum Goal_Status
		{
			eInactive,  //The goal is waiting to be activated
			eActive,    //The goal has been activated and will be processed each update step
			eCompleted, //The goal has completed and will be removed on the next update
			eFailed     //The goal has failed and will either re-plan or be removed on the next update
		};
		//TODO we may want to pass in the delta time slice here
		/// This is executed during the update step
		virtual Goal_Status Process()=0;
		/// This undertakes any necessary tidying up before a goal is exited and is called just before a goal is destroyed.
		virtual void Terminate()=0;
		//bool HandleMessage()  //TODO get event equivalent
		//TODO see if AddSubgoal really needs to be at this level 
		Goal_Status GetStatus() const {return m_Status;}
	private:
		Goal_Status m_Status;
		//TODO see if Owner and Type are necessary
}; 

class FRAMEWORK_LOGIC_API AtomicGoal : public Goal
{
	protected:  //from Goal
		virtual void Activate() {}
		virtual Goal_Status Process() {return eCompleted;}
		virtual void Terminate() {}
		//bool HandleMessage()  //TODO get event equivalent

};

class FRAMEWORK_LOGIC_API CompositeGoal : public Goal
{
	protected:  //from Goal
		virtual void Activate() {}
		virtual Goal_Status Process() {return eCompleted;}
		virtual void Terminate() {}
		//bool HandleMessage()  //TODO get event equivalent
		//Subgoals are pushed in LIFO like a stack
		virtual void AddSubgoal(Goal *g) {m_SubGoals.push_front(g);}
		//Feel free to make this virtual if we find that necessary
		/// All composite goals call this method each update step to process their subgoals.  The method ensures that all completed and failed goals
		/// are removed from the list before processing the next subgoal in line and returning its status.  If the subgoal is empty eCompleted is
		/// returned.
		Goal_Status ProcessSubgoals();
		void RemoveAllSubgoals();
	private:
		typedef std::list<Goal *> SubgoalList;
		SubgoalList m_SubGoals;
};


	}
}