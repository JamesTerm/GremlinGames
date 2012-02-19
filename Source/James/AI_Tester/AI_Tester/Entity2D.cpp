#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;

const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*																	Entity1D														*/
/***********************************************************************************************************************************/

Entity1D::Entity1D(const char EntityName[]) : m_Dimension(1.0),m_Position(0.0),m_Name(EntityName)
{
	ResetPos();
}

Entity1D::~Entity1D() 
{
}

void Entity1D::Initialize(GG_Framework::Base::EventMap& em, const Entity1D_Properties *props)
{
	m_eventMap = &em;
	if (props)
		props->Initialize(this);
}

void Entity1D::ResetPos()
{
	//CancelAllControls();
	m_Physics.ResetVectors();
	m_Position=0.0;
}

void Entity1D::TimeChange(double dTime_s)
{
	double PositionDisplacement;
	//Either we apply displacement computations here or the derived class will handle it
	if (!InjectDisplacement(dTime_s,PositionDisplacement))
		m_Physics.TimeChangeUpdate(dTime_s,PositionDisplacement);

	m_Position+=PositionDisplacement;
	if (m_IsAngular)
		NormalizeRotation(m_Position);
}


  /***********************************************************************************************************************************/
 /*																	Entity2D														*/
/***********************************************************************************************************************************/

Entity2D::Entity2D(const char EntityName[]) : 
	m_PosAtt_Read(&m_PosAtt_Buffers[0]),m_PosAtt_Write(&m_PosAtt_Buffers[1]),m_DefaultPos(Vec2D(0,0)),m_DefaultAtt(0.0),m_eventMap(NULL),
	m_Dimensions(1.0,1.0),m_Name(EntityName),m_BypassPosAtt_Update(false)
{
	ResetPos();
}

Entity2D::~Entity2D() 
{
}

void Entity2D::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	m_eventMap = &em;
	if (props)
		props->Initialize(this);
}

//Note: If for some reason there are multiple threads which need to write we would need to put a critical section around this
//Ideally that will never be the case as we keep the logic thread a single piece... if we find that we did have to distribute the
//entities and put a critical section here for external forces, the critical section would succeed 99% of the time (all the time the ship
//is not having them applied)... this would then have no penalty
void Entity2D::UpdatePosAtt()
{
	//Note 2: In theory I do not believe I need to use the OS calls for this, and one thing we can do is try this vs. just a pure pointer
	//exchange.  I believe the results would be the same for both reliability and performance.  We could test both cases to make sure in the real
	//world tests.  (I am not sure if I can put enough stress here to know for sure)

	void *Temp=m_PosAtt_Write.get();
	//exchange the pointers this works since this is the only thread doing all of the writing
	m_PosAtt_Write.assign(m_PosAtt_Read.get(),m_PosAtt_Write.get());
	m_PosAtt_Read.assign(Temp,m_PosAtt_Read.get());
}

void Entity2D::ResetPos()
{
	//CancelAllControls();
	m_Physics.ResetVectors();
	if (!m_BypassPosAtt_Update)
	{
		PosAtt *writePtr=(PosAtt *)m_PosAtt_Write.get();
		//SetPosAtt(m_origPos, FromLW_Rot(m_origAtt[0], m_origAtt[1], m_origAtt[2]));
		writePtr->m_pos_m=m_DefaultPos;
		writePtr->m_att_r=m_DefaultAtt;  //a.k.a heading
		//GetEventMap()->Event_Map["ResetPos"].Fire();
		UpdatePosAtt();
	}
}

void Entity2D::TimeChange(double dTime_s)
{
	Vec2D PositionDisplacement;
	double RotationDisplacement;
	//Either we apply displacement computations here or the derived class will handle it
	if (!InjectDisplacement(dTime_s,PositionDisplacement,RotationDisplacement))
		m_Physics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
	//This is using an atomic double buffering mechanism... should be thread safe
	PosAtt *writePtr=(PosAtt *)m_PosAtt_Write.get();
	PosAtt *readPtr=(PosAtt *)m_PosAtt_Read.get();
	writePtr->m_pos_m=readPtr->m_pos_m+PositionDisplacement;

	double Rotation=readPtr->m_att_r+RotationDisplacement;
	NormalizeRotation(Rotation);
	writePtr->m_att_r=Rotation;
	UpdatePosAtt();
}


  /***********************************************************************************************************************************/
 /*														RimSpace_GameAttributes														*/
/***********************************************************************************************************************************/

  /***********************************************************************************************************************************/
 /*														AI_Controller																*/
/***********************************************************************************************************************************/

  /***********************************************************************************************************************************/
 /*														Ship																		*/
/***********************************************************************************************************************************/
  /***********************************************************************************************************************************/
 /*													Ship_Tester																		*/
/***********************************************************************************************************************************/

Ship_Tester::~Ship_Tester()
{
	//not perfect but in a test environment will do
	delete GetController()->m_Goal;
	GetController()->m_Goal=NULL;
	//assert(!GetController()->m_Goal);
}

void Ship_Tester::SetPosition(double x,double y) 
{
	
	PosAtt *writePtr=(PosAtt *)m_PosAtt_Write.get();
	PosAtt *readPtr=(PosAtt *)m_PosAtt_Read.get();
	writePtr->m_pos_m.set(x,y);
	writePtr->m_att_r=readPtr->m_att_r;  //make sure the entire structure is updated!
	UpdatePosAtt();
}

void Ship_Tester::SetAttitude(double radians)
{

	PosAtt *writePtr=(PosAtt *)m_PosAtt_Write.get();
	PosAtt *readPtr=(PosAtt *)m_PosAtt_Read.get();
	writePtr->m_pos_m=readPtr->m_pos_m;  //make sure the entire structure is updated!
	writePtr->m_att_r=radians;
	UpdatePosAtt();
}

Goal *Ship_Tester::ClearGoal()
{
	//Ensure there the current goal is clear
	if (GetController()->m_Goal)
	{
		GetController()->m_Goal->Terminate();
		//TODO determine how to ensure the update thread is finished with the process
	}
	return GetController()->m_Goal;
}

void Ship_Tester::SetGoal(Goal *goal) 
{
	GetController()->m_Goal=goal;
}
