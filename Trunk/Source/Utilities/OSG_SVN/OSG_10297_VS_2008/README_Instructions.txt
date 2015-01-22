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
2.  export .\Source\Utilities\OSG_SVN\OpenSceneGraph\include these includes to here: 
.\Source\Utilities\OSG_SVN\OSG_10297_VS_2008\include

3. Then build ALL_Build... it will auto build all of them. This will take about 10 minutes ( a lot of projects)
4. Once this is done build INSTALL... you can build from INSTALL instead if you wish.
