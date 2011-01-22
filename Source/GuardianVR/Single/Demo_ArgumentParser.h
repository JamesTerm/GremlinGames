// Demo_ArgumentParser.h
#pragma once

class Demo_ArgumentParser : public GG_Framework::UI::ViewerApp_ArgumentParser
{
protected:
	virtual bool ProcessFlag(unsigned& index, unsigned argc, const char** argv)
	{
		if (stricmp(argv[index], "-assist") == 0)
		{
			// The amount of assistance given to the player for shooting
			++index;
			if (index >= argc)
				printf("ERROR: Missing Parameter for Flag '-assist'\n");
			else
			{
				//printf("ConsoleApp_ArgumentParser::ProcessFlag - assist = %s\n", argv[index]);
				GG_Framework::AppReuse::Cannon::ASSIST = atof(argv[index]);
				++index;
			}
			return true;
		}
		else if (stricmp(argv[index], "-ignore_collisions") == 0)
		{
			// Turn off collision detection while we are playing with framerate
			GG_Framework::Logic::GameClient::TEST_IGNORE_COLLISIONS = true;
			++index;
			return true;
		}
		else
			return __super::ProcessFlag(index, argc, argv);
	}
};