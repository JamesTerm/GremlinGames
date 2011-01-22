/*******************************************************
      Lightwave Scene Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "..\CompilerSettings.h"

#include <string>
#include <sstream>

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include "SceneLoader.h"

char* GetLastSlash(char* fn, char* before)
{
	if (!fn) return NULL;
	char* lastSlash = before ? before-1 : fn+strlen(fn);

	while (lastSlash > fn)
	{
		if ((*lastSlash == '/') || (*lastSlash == '\\'))
			return lastSlash;
		--lastSlash;
	}

	return NULL;
}

const char* GetCWD_FromSceneFile(const char* fn)
{
	if (fn == NULL)
		return NULL;

	// Get the path name to set the PWD
	static char buff[1024];
	strcpy(buff, fn);

	// Strip the filename off to just get the parent folder
	char* lastSlash = GetLastSlash(buff, NULL);
	if (lastSlash)
	{
		*lastSlash = 0;

		// Now work up the path until we find Objects or Scenes
		while (lastSlash = GetLastSlash(buff, lastSlash))
		{
			// The content Directory SHOULD be right above the objects or scenes directory
			if (	!strnicmp(lastSlash+1, "objects/", 8)
				||	!strnicmp(lastSlash+1, "objects\\", 8)
				||	!strnicmp(lastSlash+1, "scenes\\", 7)
				||	!strnicmp(lastSlash+1, "scenes/", 7)
				||	!stricmp(lastSlash+1, "objects")
				||	!stricmp(lastSlash+1, "scenes"))
			{
				*lastSlash = 0;
				break;
			}
		}
	}

	return buff;
}

class ReaderWriterLWS : public osgDB::ReaderWriter
{
public:
	ReaderWriterLWS()
	{
		supportsExtension("lws","Lightwave scene format");
	}

    virtual const char* className() const { return "ReaderWriterLWS"; }

    virtual bool acceptsExtension(const std::string &extension) const {
        return osgDB::equalCaseInsensitive(extension, "lws");
    }

    virtual ReadResult readNode(const std::string &file, const osgDB::ReaderWriter::Options *options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);        
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        // code for setting up the database path so that content directory is searched for on relative paths.
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
		
		const char* contentDir = GetCWD_FromSceneFile(fileName.c_str());
		osg::notify(osg::NOTICE) << "LWS Content Directory(" << (options?"old opts":"new opts") << ") : " << contentDir << std::endl;
		local_opt->setDatabasePath(contentDir);
	   // local_opt->setDatabasePath(osgDB::getFilePath(fileName));

        lwosg::SceneLoader::Options conv_options = parse_options(local_opt.get());

        lwosg::SceneLoader scene_loader(conv_options);
        osg::ref_ptr<osg::Node> node = scene_loader.load(fileName, local_opt.get());
        if (node.valid()) {
            return node.release();
        }

        return ReadResult::FILE_NOT_HANDLED;
    }

    lwosg::SceneLoader::Options parse_options(const Options *options) const;

protected:

    

};

lwosg::SceneLoader::Options ReaderWriterLWS::parse_options(const Options *options) const
{
    lwosg::SceneLoader::Options conv_options;

    if (options) {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt) {
            // no options yet!
        }
    }

    return conv_options;
}


// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(lws, ReaderWriterLWS)

