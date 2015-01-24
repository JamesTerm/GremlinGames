The OSG_10297_VS_2008 folder was created to make it easier to build OSG for the simulation.

Currently at this time there is no interest with later versions of OSG, as this revision has been used for simple graphics in robot simulation.  This will explain how this folder was created and what is needed to get it to run.

This folder started out as a cmake automated file where all the vcproj files had the following modifications
1.  All absolute paths to OSG were replaced with $(GG_UTILITIES) in the prefix so that we can work on any machine
	(Be sure to run the environment script  in here: .\Source\Utilities\SetUpEnvironmentVars)
2.  All custom build commands of executing cmake were stripped
3.  All paths to Visual_Studio_9_2008_Win32 were switched to OSG_10297_VS_2008  (so this step would be needed for say 64bit or 2013)
3.  Some of the redundant header files were stripped.
--------------------------------------------------------------

To build osg:
============

1.  To build these files ensure the environment variables are setup on your machine (this is a one-time setup process)
2. unload projects of: zero check all build and install projects... these could work but you'd need to tweak the location of cmake, and really the things these do are things VS can do anyway.
	-Right click on those projects and select unload project... the settings to keep them unloaded are stored locally on your machine
3.  build solution

----

That is pretty much it... they all should build... now some final steps when building the robot code (or gg)
If you wish to use VS 2008 win 32 builds  you can keep the environment variables as they are in 
.\Source\Utilities\SetUpEnvironmentVars\SetEnvironment_OSGSVN_VS9.bat

But if you keep this you'll need to export the include files to here
.\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\include
Also export .\Source\Utilities\OSG_SVN\OSG_10297_VS_2008\include (for the config files)

and copy 

.\Source\Utilities\OSG_SVN\OSG_10297_VS_2008\lib to here:
.\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\lib

Altenatively you can modify the SetEnvironment_OSGSVN_VS9.bat file to point the include and lib paths to where they reside without a copy.  If you make a different platform it would be good to checkin the projects like this one and a new SetEnvironment batch file to these paths.

------------------------------------

That's to get it to build... to get it to run... you'll need
.\Source\Utilities\OSG_SVN\OSG_10297_VS_2008\bin to here:
.\Source\Utilities\OSG_SVN\Visual_Studio_9_2008_Win32\INSTALL\bin

to use the batch files that reside in debug_bin and release_bin... once again you may create new batch files per platform to your paths... these simply copy the dll's over to the actual directory used when launching the executable.

A birds eye view of what is needed in its default environment looks like this:

Visual_Studio_9_2008_Win32
	Install
		bin
		include
	lib

Where... the robot testing code pull the includes and libs to build... and then where the bins pull the dlls to be in the path of the .exe

Good Luck!




