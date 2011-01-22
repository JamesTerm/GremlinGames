#include <iostream>
using namespace std;

#include "luainc.h"

#ifndef assert
#define assert(cond) if (!(cond)){ cerr << "ASSERT FAILED: " << #cond << endl; abort();}
#endif

static int printMessage (lua_State *lua)
{
	assert (lua_isstring (lua,1));

	const char *msg = lua_tostring (lua, 1);

	// debug output
	cout << "script: " << msg << endl;
	return 0;
}

int main(int argc, const char** argv)
{
	for (int i = 0; i < argc; ++i)
		cout << argv[i] << endl;

	int iErr = 0;
	lua_State *lua = lua_open ();  // Open Lua
	luaL_openlibs (lua);              // Load all of the libraries
	lua_pushcclosure (lua, printMessage, 0);
	lua_setglobal (lua, "trace");

	if ((iErr = luaL_loadfile (lua, "mytest.lua")) == 0)
	{
		// Call main...
		if ((iErr = lua_pcall (lua, 0, LUA_MULTRET, 0)) == 0)
		{ 
			// Push the function name onto the stack
			lua_pushstring (lua, "helloWorld");
			// Function is located in the Global Table
			lua_gettable (lua, LUA_GLOBALSINDEX);  
			iErr = lua_pcall (lua, 0, 0, 0);
			if (iErr != 0)
				cerr << "Failed first luapcall" << endl;
		}
		else
			cerr << "Failed first luapcall" << endl;
	}
	lua_close (lua);

	return iErr;
}