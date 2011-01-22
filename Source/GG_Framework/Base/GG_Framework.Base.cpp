#include "stdafx.h"
#include "GG_Framework.Base.h"

// We can place global definitions and other DLL general work here
bool GG_Framework::Base::TEST_IGNORE_COLLISIONS = false;

#ifdef DEBUG
bool GG_Framework::Base::TEST_USE_SIMPLE_MODELS = true;
#else
bool GG_Framework::Base::TEST_USE_SIMPLE_MODELS = false;
#endif
