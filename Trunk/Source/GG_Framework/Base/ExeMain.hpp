// ExeMain.hpp
#pragma once
#include <assert.h>
#include <exception>

#include <osgDB\DynamicLibrary>
#include <osgDB\Registry>
extern const char* g_build_TIMESTAMP;
extern const unsigned g_build_SVN_VERSION;
extern const unsigned g_OSG_SVN_VERSION;

//! This Class is just used for executables to have a nice entry in DOxygen
//! And to wrap up the common stuff we are putting around all EXE's.
class IExeMain
{
public:
	virtual int Main(unsigned argc, const char* argv[]) = 0;
	int Run(unsigned argc, const char* argv[])
	{
		// exe name and build information
		printf("%s\n", argv[0]);
		printf("Build: %i, (OSG %i) %s\n", g_build_SVN_VERSION, g_OSG_SVN_VERSION, g_build_TIMESTAMP);

		// Show the command line
		for (unsigned i = 1; i < argc; ++i)
			printf("%s ", argv[i]);
		printf("\n");

		// Optimize the osg database by caching all read image files
		{
			//grrrr I thought the instance would have options instantiated but this is not the case
			//This is size_t so that I can use the or operator (not sure why it can't work otherwise)
			//size_t options=osgDB::Registry::instance()->getOptions()->getObjectCacheHint();
			//We may want to cache other things; you can see what we have now
			//options|=osgDB::ReaderWriter::Options::CACHE_IMAGES;  
			//osgDB::Registry::instance()->getOptions()->setObjectCacheHint((osgDB::ReaderWriter::Options::CacheHintOptions)options);

			osg::ref_ptr<osgDB::ReaderWriter::Options> options=new osgDB::ReaderWriter::Options;
			options->setObjectCacheHint(
				(osgDB::ReaderWriter::Options::CacheHintOptions)(osgDB::ReaderWriter::Options::CACHE_IMAGES|osgDB::ReaderWriter::Options::CACHE_ARCHIVES));
			osgDB::Registry::instance()->setOptions(options.get());
			osgDB::Registry::instance()->setBuildKdTreesHint(osgDB::ReaderWriter::Options::BUILD_KDTREES);
		}

		// Initialize random number generation
		osg::Timer t;
		unsigned rseed = (unsigned)t.tick();
		srand(rseed);

		int progRet = 0;

#ifndef _DEBUG
		try
#endif
		{
			progRet = Main(argc, argv);
		}

#ifndef _DEBUG
		catch (std::exception & exc)
		{
			cout << "*** UNRECOVERABLE ERROR: " << exc.what() << endl;
			cout << "< Press RETURN to exit >";
			cin.get();
			return 1;
		}
		catch (const char* msg)
		{
			std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
			cout << "< Press RETURN to exit >";
			cin.get();
			return 1;
		}
		catch (const std::string& msg)
		{
			std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
			cout << "< Press RETURN to exit >";
			cin.get();
			return 1;
		}
		catch (...)
		{
			cout << "*** UNRECOVERABLE ERROR: Unknown Error Type" << endl;
			cout << "< Press RETURN to exit >";
			cin.get();
			return 1;
		}
#endif

		if (GG_Framework::Base::ThreadedClass::ERROR_STATE || progRet)
		{
			std::cout << "< Press RETURN to exit >";
			std::cin.get();
			if (!progRet)
				progRet = 1;
		}

		return progRet;
	}
};