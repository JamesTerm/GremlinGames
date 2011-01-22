This distribution contains the OpenSceneGraph-1.2 release, and its main
dependencies - OpenThreads-1.5 and Producer-1.1

Compilation should proceed in the order of OpenThreads then Producer then
OpenSceneGraph.  Under Windows you'll need to open up the workspace
files for each project respectively and build them all. Under unix:

   cd OpenThreads; make; sudo make install; cd ..
   cd Producer; make; sudo make install; cd ..
   cd OpenSceneGraph; make COMPILE_EXAMPLES=yes; sudo make install

If you wish to keep your OpenThreads & Producer installed locally, then
define their locations via the following environmental variables:

   setenv OPENTHREADS_INC_DIR mydirecotory/OpenThreads/include
   setenv OPENTHREADS_LIB_DIR mydirecotory/OpenThreads/lib/platform
   setenv PRODUCER_INC_DIR mydirecotory/Producer/include
   setenv PRODUCER_LIB_DIR mydirecotory/Producer/lib/platform

   where platform is the host machine type ie. Linux32

To run the OpenSceneGraph examples :

   cd OpenSceneGraph; sh runexamples.bat

If you have any problems compiling or running the executables, please don't
hesitate to send a report of the problems to the osg-user mailing list.

Have fun,
Robert Osfield
12th September 2006.
