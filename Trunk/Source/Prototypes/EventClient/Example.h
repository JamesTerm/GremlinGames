#ifndef __Example_Included_
#define __Example_Included_

// Include Headers BEFORE namespace declarations, external libs before internal files
#include <vector>

// Internal headers that are necessary
#include "BaseInterface.h"
#include "Event.h"

// Use Pre-Declarations to avoid extra file dependencies
class ProjectNS::CEncapsulatedClass;

namespace ProjectNS {


/** Use the class comment to describe what roles this class plays, 
 *	and in what user stories it is involved with and
 *	a bit about its usage, and where it is being tested
 */
class AExample : public IBaseInterface
{
public:	// C'tors and D'tors
	AExample();
	AExample(unsigned count);
	virtual ~AExample();

	AExample(const AExample& copy);
	AExample& operator=(const AExample& copy);

public:	// Public Enumerations and Inner Classes
	enum EExampleEnum {opt1, opt2, opt3};
	class CExampleNode 
	{
	public:
		const CExampleNode* Prev();
		const CExampleNode* Next();

	private:
		// LOOK, a PRIVATE constructor
		CExampleNode(CExampleNode* prev, CExampleNode* next) : _prev(prev), _next(next) {}
		CExampleNode* _prev;
		CExampleNode* _next;

	friend AExample;
	};

public:	// Events
	Event1<EExampleEnum> EnumChanged;

public: // Often Used Member Functions
	const CEncapsulatedClass& Encapsulated() const {return _encapsulated;}
	const CExampleNode* Head() const {return _listHead;}

public: // Public Callbacks

	// This should not be overridden, came from IBaseInterface
	void Callback1();

	//! May Override this one, call the base class if...
	virtual void Callback2(
		float param1_meters, //!< More notes about this parameter, range, exception if out of range
		float param2_seconds //!< More notes about this parameter
		);

	// This one is pure virtual, so you MUST override
	virtual void Callback3(const std::vector<int>& intV) = 0;

protected:	// Member functions
	void ProtectedFunc() const;

private:	// Helper Functions (called from other functions)
	void Helper1();
	void Helper2();

private:
	CEncapsulatedClass& _encapsulated;
	CExampleNode* _listHead;
};	// AExample



};	// ProjectNS

#endif	// __Example_Included_