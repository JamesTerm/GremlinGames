// GG_Framework.UI ArgumentParser.h
#include "stdafx.h"
#include "GG_Framework.UI.h"

#define DEBUG_ARGS // printf

using namespace GG_Framework::UI;

bool ConsoleApp_ArgumentParser::ProcessFlag(unsigned& index, unsigned argc, const char** argv)
{
	if (stricmp(argv[index], "-notify") == 0)
	{
		// Set the notify level
		++index;
		if (index >= argc)
			printf("ERROR: Missing Parameter for Notify Flag '-notify'\n");
		else
		{
			DEBUG_ARGS("ConsoleApp_ArgumentParser::ProcessFlag - Notify = %s\n", argv[index]);
			switch (argv[index][0])
			{
			case 'a':
			case 'A':
				NotifySeverity = osg::ALWAYS;
				break;
			case 'f':
			case 'F':
				NotifySeverity = osg::FATAL;
				break;
			case 'w':
			case 'W':
				NotifySeverity = osg::WARN;
				break;
			case 'n':
			case 'N':
				NotifySeverity = osg::NOTICE;
				break;
			case 'i':
			case 'I':
				NotifySeverity = osg::INFO;
				break;
			case 'd':
			case 'D':
				NotifySeverity = osg::DEBUG_INFO;
				break;
			default:
				printf("ERROR: Invalid Parameter for Notify Flag '-notify', using FATAL\n");
			}
			++index;
		}
		return true;
	}
	else if (stricmp(argv[index], "-ignore_collisions") == 0)
	{
		// Turn off collision detection while we are playing with framerate
		GG_Framework::Base::TEST_IGNORE_COLLISIONS = true;
		++index;
		return true;
	}
	else if (stricmp(argv[index], "-simple_models") == 0)
	{
		// Simplify models while we are playing with framerate
		GG_Framework::Base::TEST_USE_SIMPLE_MODELS = true;
		++index;
		return true;
	}
	else if (stricmp(argv[index], "-log_timer") == 0)
	{
		// Used to start logging the AI timer
		LogTimer = true;
		++index;
		return true;
	}
	else
		return __super::ProcessFlag(index, argc, argv);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ViewerApp_ArgumentParser::ViewerApp_ArgumentParser() 
	: ConsoleApp_ArgumentParser(), 
	UseAntiAliasing(false), RES_WIDTH(0), RES_HEIGHT(0), MAX_FPS(0), MockAudio(false),
	m_startStats(-1.0), m_endStats(-1.0), m_actualStart(-1.0), m_minFrameRate(1e6), m_maxFrameRate(-1e6), m_avgFrameRate(0.0), m_numFrames(0), 
	m_mainWindow(NULL), m_lastTime(0.0)
{
}
//////////////////////////////////////////////////////////////////////////

void ViewerApp_ArgumentParser::AttatchCamera(GG_Framework::UI::MainWindow* mainWindow, GG_Framework::Base::Timer* gameTimer)
{
	m_mainWindow = mainWindow;
	// Connect the TimerLogger
	mainWindow->UpdateTimerLogger.SetLogger(gameTimer->Logger);

	if (m_mainWindow && gameTimer && (m_endStats > 0.0))
	{
		// Make sure the timer is updated the first time
		gameTimer->FireTimer();

		// Adjust the start times to use the current time
		m_startStats += gameTimer->GetCurrTime_s();
		m_endStats += gameTimer->GetCurrTime_s();

		// Listen for updates from the main window
		gameTimer->CurrTimeChanged.Subscribe(ehl, *this, &ViewerApp_ArgumentParser::GlobalTimeChangedCallback);
	}
}
//////////////////////////////////////////////////////////////////////////

void ViewerApp_ArgumentParser::GlobalTimeChangedCallback(double newTime_s)
{
	// Only start this when the game has run some amount of time
	if (newTime_s >= m_startStats)
	{
		// Remember the actual time this all started so we can get a good average
		if (m_actualStart == -1.0)
			m_actualStart = newTime_s;
		m_lastTime = newTime_s;
		++m_numFrames;

		// Keep the min and max frame rates (these are 1s averages)
		double currFramerate = m_mainWindow->GetFrameRate();
		if (currFramerate < m_minFrameRate)
			m_minFrameRate = currFramerate;
		if (currFramerate > m_maxFrameRate)
			m_maxFrameRate = currFramerate;
		
		// If we are done, just close.  Make sure we have advanced at least one frame
		if ((newTime_s > m_endStats) && (newTime_s > m_actualStart))
		{
			m_mainWindow->TryClose();

			// Ignore everything from now on
			m_startStats = 1e6;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

ViewerApp_ArgumentParser::~ViewerApp_ArgumentParser()
{
	// This is where we write stuff out to the console or to a file (append to the file)
	if (m_lastTime > 0.0)
	{
		// Get the real average
		ASSERT(m_lastTime > m_actualStart);
		m_avgFrameRate = ((double)m_numFrames) / (m_lastTime-m_actualStart);

		if (!m_statsOutputFN.empty())
		{
			FILE* outFile = fopen(m_statsOutputFN.c_str(), "a");
			if (outFile)
			{
				fprintf(outFile, "Framerates: (%3.1f, %3.1f, %3.1f)\n", m_minFrameRate, m_avgFrameRate, m_maxFrameRate);
				fclose(outFile);
			}
		}
		
		// Just write out the values to cout all the time
		printf("Framerates: (%3.1f, %3.1f, %3.1f)\n", m_minFrameRate, m_avgFrameRate, m_maxFrameRate);
	}
}
//////////////////////////////////////////////////////////////////////////

bool ViewerApp_ArgumentParser::ProcessFlag(unsigned& index, unsigned argc, const char** argv)
{
	if (stricmp(argv[index], "-aa") == 0)
	{
		DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - AA = true\n");
		UseAntiAliasing = true;
		++index;
		return true;
	}
	else if (stricmp(argv[index], "-na") == 0)
	{
		DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - AA = false\n");
		UseAntiAliasing = false;
		++index;
		return true;
	}
	else if (stricmp(argv[index], "-mockaudio") == 0)
	{
		DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - mockaudio\n");
		MockAudio = true;
		++index;
		return true;
	}
	else if (stricmp(argv[index], "-music") == 0)
	{
		// Set the music track (This will be used somewhere else, but for now it is good)
		++index;
		if (index >= argc)
			printf("ERROR: Missing Parameter for Soundtrack Flag '-m'\n");
		else
		{
			SoundTrack = argv[index];
			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - Soundtrack = %s\n", SoundTrack.c_str());
			++index;
		}
		return true;
	}
   else if (stricmp(argv[index], "-userPrefs") == 0)
	{
		// Set the music track (This will be used somewhere else, but for now it is good)
		++index;
		if (index >= argc)
			printf("ERROR: Missing Parameter user preference file '-userPrefs'\n");
		else
		{
			SoundTrack = argv[index];
			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag -userPrefs = %s\n", SoundTrack.c_str());
			++index;
		}
		return true;
	}
	else if (stricmp(argv[index], "-maxfps") == 0)
	{
		// The maximum frames per second used.  This will cause an extra sleep in the OSG thread
		++index;
		if (index >= argc)
			printf("ERROR: Missing Parameter for Flag '-maxfps'\n");
		else
		{
			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - maxfps = %s\n", argv[index]);
			MAX_FPS = atof(argv[index]);
			++index;
		}
		return true;
	}
	else if (stricmp(argv[index], "-res") == 0)
	{
		// The resolution of the final OSG window
		if (argc <= (index+2))
		{
			printf("ERROR: Missing Parameter(s) for Flag '-res'\n");
			index = argc+1;
		}
		else
		{
			RES_WIDTH = atoi(argv[++index]);
			RES_HEIGHT = atoi(argv[++index]);
			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - res = %ix%i\n", RES_WIDTH, RES_HEIGHT);
			++index;
		}
		return true;
	}
	else if (stricmp(argv[index], "-stats") == 0)
	{
		// The minimum and maximum time the stats will run, and optionally the filename to write to
		if (argc <= (index+2))
		{
			printf("ERROR: Missing Parameter(s) for Flag '-stats'\n");
			index = argc+1;
		}
		else
		{
			// We are grabbing stats for the scene to print out and auto exit
			m_startStats = atof(argv[++index]);
			m_endStats = atof(argv[++index]);
			if (m_startStats < 1.0)
				m_startStats = 1.0;
			if (m_endStats < (m_startStats + 1.0))
				m_endStats = m_startStats + 1.0;

			++index;
			if ((argc > index) && (argv[index][0] != '-'))
			{
				m_statsOutputFN = argv[index];
				++index;
			}

			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - stats = %f,%f, (%s)\n", m_startStats, m_endStats, m_statsOutputFN.c_str());
		}
		return true;
	}
	else if (stricmp(argv[index], "-perf") == 0)
	{
		// The minimum and maximum framerates that will cause a performance index change.
		if (argc <= (index+3))
		{
			printf("ERROR: Missing Parameter(s) for Flag '-perf'\n");
			index = argc+1;
		}
		else
		{
			// Set the minimum and maximum frame rates that force a change in the window performance index
			Window::PERFORMANCE_MIN = atof(argv[++index]);
			Window::PERFORMANCE_MAX = atof(argv[++index]);
			Window::PERFORMANCE_INIT_INDEX = atoi(argv[++index]);

			++index;
			DEBUG_ARGS("ViewerApp_ArgumentParser::ProcessFlag - perf = %f,%f [%i]\n", Window::PERFORMANCE_MIN, Window::PERFORMANCE_MAX, Window::PERFORMANCE_INIT_INDEX);
		}
		return true;
	}
	else
		return __super::ProcessFlag(index, argc, argv);
}
//////////////////////////////////////////////////////////////////////////
