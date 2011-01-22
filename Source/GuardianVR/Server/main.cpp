// GuardianVR.Server main.cpp
#include "stdafx.h"
#include "GuardianVR.Server.h"
using namespace std;

using namespace GG_Framework::Logic;
using namespace GG_Framework::Logic::Network;

int Test_C_FUNC(lua_State *L) 
{
	double d = luaL_checknumber(L, 1);
	lua_pushnumber(L, 100.0*d);
	return 1;  /* number of results */
};

REGISTER_C_SCRIPT_FUNC(Test_C_FUNC);

#include "../../GG_Framework/Base/ExeMain.hpp"
namespace GuardianVR
{
	namespace MainServer
	{
		class EXE : public IExeMain
		{
		public:
			virtual int Main(unsigned argc, const char* argv[]);
		};
	}
}

int main(unsigned argc, const char** argv)
{
	GuardianVR::MainServer::EXE exe;
	return exe.Run(argc, argv);
}

int GuardianVR::MainServer::EXE::Main(unsigned argc, const char* argv[])
{
	// Content Directory
	char contentDIR[512];
	_getcwd(contentDIR, 512);
	printf("Content Directory: %s\n", contentDIR);

	// Strip out the flagged and non-flagged options
	GG_Framework::UI::ConsoleApp_ArgumentParser argParser;
	std::vector<string> argList = argParser.ProcessArguments(argc, argv);
	osg::setNotifyLevel(argParser.NotifySeverity);

	// Use the parameters, Config file, and File Dialog to find the name of the script to run
	GG_Framework::Base::ConfigFileIO configFile;
	vector<string> fileList;
	if (!configFile.GetFilesToLoad(argList, fileList, "Select Script File", NULL))
	{
		printf("User Aborted, QUITTING\n");
		return 0;
	}
	string scriptFN = fileList[0];


	// Verify that we can read the script
	// printf("Loading LUA Script from %s ...\n", scriptFN.c_str());
	GuardianVR::Base::GuardianVR_Script script;
	printf("Loading Script file: %s\n", scriptFN.c_str());
	const char* err = script.LoadScript(scriptFN.c_str(), true);
	if (err)
	{
		printf("Unable to load script file: %s\n", err);
		return 1;
	}
	else
		printf("Script File Loaded OK\n");

	// After reading the script, we have to reset the CWD
	_chdir(contentDIR);

	// Get the Maximum Number of Players from the Script, or default to 32.
	double MAX_Players = 32.0;
	script.GetGlobal("MAX_Players", NULL, NULL, &MAX_Players);
	printf("MAX_Players = %i\n", (int)MAX_Players);

	//////// TESTING
	#undef __Testing__
	#ifdef __Testing__
	{
		double funcRet = 0.0;
		const char* err = script.CallFunction("TestFunc", "dd>d", 3.0, 1.0, &funcRet);
		if (err)
			printf("%s\n", err);
		else
			printf("TestFunc(3.0, 1.0) returns %lf\n", funcRet);
	}
	#endif

	// Get the port number.  Watch for negative numbers.
	unsigned portNum = DEFAULT_SERVER_PORT;
	cout << "Enter the Port #(" << portNum << "): ";
	cin >> portNum;
	if (portNum <= 0)
		portNum = DEFAULT_SERVER_PORT;

	// Generate the Servers that will Talk to all Clients, the AI, and a ServerManager
	Server player_server((int)MAX_Players, portNum);
	MockServer ai_server("AI Server");
	MockClient ai_client(ai_server, MOCK_AI_PLAYER_ID);
	GG_Framework::Logic::ServerManager serverManager(&script, &player_server, &ai_server, contentDIR);

	// Provide the IP address and Port Number
	// I am not sure if this is the only one (inside and outside my router firewall)
	printf("\nIP Address:Port#\n%s\n\n", player_server.GetRakServer()->GetInternalID().ToString());

	// Wait for the first Player to Connect (Provide the IP and Port#)
	printf("Waiting for Client Connections...\n");

	// The primary loop exits when there are no more players
	// We can run this in the single thread
	serverManager.run();

	// Watch for problems in all threads
	return 0;
}