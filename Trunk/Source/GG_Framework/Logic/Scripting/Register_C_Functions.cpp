// Register_C_Functions.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.Scripting.h"

using namespace GG_Framework::Logic::Scripting;

std::vector<Register_C_Function*> Register_C_Function::C_funcs;
OpenThreads::Mutex Register_C_Function::s_mutex;

Register_C_Function::Register_C_Function(lua_CFunction func, const char* funcName) :
m_func(func), m_funcName(funcName)
{
	ASSERT(func);
	ASSERT(funcName);
	m_index = (unsigned)C_funcs.size();
	GG_Framework::Base::RefMutexWrapper rmw(s_mutex);
	C_funcs.push_back(this);
}

Register_C_Function::~Register_C_Function()
{
	// Just remove this from the list at the position m_index
	GG_Framework::Base::RefMutexWrapper rmw(s_mutex);
	C_funcs[m_index] = NULL;
}

void Register_C_Function::RegisterAllFunctions(lua_State* lua_state)
{
	ASSERT(lua_state);
	GG_Framework::Base::RefMutexWrapper rmw(s_mutex);
	unsigned numRegFuncs = (unsigned)C_funcs.size();
	for (unsigned i = 0; i < numRegFuncs; ++i)
	{
		Register_C_Function* rf = C_funcs[i];
		if (rf)
		{
			lua_pushcfunction(lua_state, rf->m_func);
			lua_setglobal(lua_state, rf->m_funcName.c_str());
		}
	}
}