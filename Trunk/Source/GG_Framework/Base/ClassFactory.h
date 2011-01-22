// GG_Framework.Base ClassFactory.h

#include <string>
#include <map>
#include <OpenThreads/Mutex>
#include "ASSERT.h"
#include "Misc.h"

template<class BaseClass> struct FactoryMapT;

template<class BaseClass>
class IClassFactoryT
{
public:
	IClassFactoryT(std::string className, FactoryMapT<BaseClass>& factoryMap);
	virtual BaseClass* New() = 0;
	virtual void Delete(BaseClass* bc) = 0;
};
//////////////////////////////////////////////////////////////////////////

template<class BaseClass>
struct FactoryMapT
{
	std::map<std::string, IClassFactoryT<BaseClass>*, std::greater<std::string> > factoryMap;
	OpenThreads::Mutex mutex;

	BaseClass* Create(std::string className)
	{
		GG_Framework::Base::RefMutexWrapper mr(mutex);
		try
		{
			return factoryMap[className]->New();
		}
		catch (...)
		{
			ASSERT_MSG(false, GG_Framework::Base::BuildString("Failed to create Class Factory for %s\n", className.c_str()).c_str());
		}
		return NULL;	// Should never get here, but to avoid the warnings we will place the return here
	}
	void Destroy(std::string className, BaseClass* bc)
	{
		GG_Framework::Base::RefMutexWrapper mr(mutex);
		factoryMap[className]->Delete(bc);
	}
};
//////////////////////////////////////////////////////////////////////////

template<class BaseClass>
IClassFactoryT<BaseClass>::IClassFactoryT(std::string className, FactoryMapT<BaseClass>& factoryMap)
{
	GG_Framework::Base::RefMutexWrapper mr(factoryMap.mutex);
	factoryMap.factoryMap[className] = this;
}
//////////////////////////////////////////////////////////////////////////

template<class SuperClass, class BaseClass>
class ClassFactoryT : public IClassFactoryT<BaseClass>
{
public:
	ClassFactoryT(std::string className, FactoryMapT<BaseClass>& factoryMap) : 
	  IClassFactoryT<BaseClass>(className, factoryMap) {}
	virtual BaseClass* New() {return new SuperClass();}
	virtual void Delete(BaseClass* bc){delete ((SuperClass*)bc);}
};
//////////////////////////////////////////////////////////////////////////
