#include "stdafx.h"
#include "../../../CompilerSettings.h"
#include "../../../ExecutableVersionSystem.hpp"

using namespace std;

int main(int argc, const char** argv)
{
	// exe name and build information
	cout << argv[0] << endl;
	cout << "Build: " << g_build_SVN_VERSION << ", " << g_build_TIMESTAMP << endl;

	// Show the command line
	for (int i = 1; i < argc; ++i)
		cout << argv[i] << " ";
	cout << endl;

	return UnitTest::RunAllTests();
}