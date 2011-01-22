/*! \file ExecutableVersionSystem.hpp
	\brief This file is included from all of the executable files so they can include
	a string in their output that includes the build date and the SVN version number.

	As a process, when a change is made that a dev wants to provide internally (to artists
	or whatever), they are to first check in the changes to SVN (not the binaries), 
	and get that SVN version.  They will then know that the check-in with the compiled
	versions in stable-bin will be in the NEXT SVN build number.  They should put THAT build 
	number in g_build_SVN_VERSION, which will modify this file, so that __TIMESTAMP__ will
	give the appropriate date.  They can then do a full RELEASE rebuild of all projects
	to the bin directory, and copy over those to stable_bin.  The following commit will
	then include only these rebuilt binaries (with the correct version number and date, 
	and this file.
*/

/** The g_build_TIMESTAMP is included in this file, rather than being used directly
from the EXE files, because the __TIMESTAMP__ macro shows the last time the source
file was modified.  The file from the EXE's may not have been modified, but this
file WILL be, because someone will always set the g_build_SVN_VERSION. */
const char* g_build_TIMESTAMP = __TIMESTAMP__;

/** This number is maintained by the developer by hand, just before a developer builds 
a build to go into StableBin, so the one there has the proper number.
It would be great to figure out how to handle this automatically, but for now, devs can
do this manually.  
*/
const unsigned g_build_SVN_VERSION = 1664;
const unsigned g_OSG_SVN_VERSION = 10297;
