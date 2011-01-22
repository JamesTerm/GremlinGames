#include "stdafx.h"
#include "GG_Framework.Base.h"

bool GG_Framework::Base::ThreadedClass::ERROR_STATE = false;

void GG_Framework::Base::ThreadSleep(unsigned sleepMS)
{
	Sleep(sleepMS);
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::Base::ThreadedClass::run()
{
	try
	{
		tryRun();
	}

	catch (std::exception & exc)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << exc.what() << std::endl;
		ERROR_STATE = true;
	}
	catch (const char* msg)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
		ERROR_STATE = true;
	}
	catch (const std::string& msg)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
		ERROR_STATE = true;
	}
	catch (...)
	{
		std::cout << "*** UNRECOVERABLE ERROR: Unknown Error Type" << std::endl;
		ERROR_STATE = true;
	}
}