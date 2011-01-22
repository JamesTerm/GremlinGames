// GG_Framework.Logic BB_ActorScene.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic;


GG_Framework::UI::ActorTransform* 
BB_ActorScene::ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, const char* fn)
{
	//! \todo Make this function watch for LWS and LWO files.  For LWO files,
	//! look for a file called *_bb.lwo, and parse through a lws file to 
	//! to the same kind of thing by looking through the lws file.
	char buff[1024];
	strcpy(buff, fn);
	char* lastDot = strrchr(buff, '.');
	char* lastUnderscore = strrchr(buff, '_');
	if (lastUnderscore && !strnicmp(lastUnderscore, "_LOD", 4))
		strcpy(lastUnderscore, "_BB.lwo");
	else if (lastDot)
	{
		if (!stricmp(lastDot, ".lwo"))
			strcpy(lastDot, "_BB.lwo");
		else if (!stricmp(lastDot, ".lws"))
		{
			strcpy(lastDot, "_BB.lws");

			// Make a copy of the lws into a new file, but replace with lwo references a _bb
			// We only need to do this if the _bb version of the file does not exist or is older than the original
			int fileCompare = GG_Framework::Base::CompareFileLastWriteTimes(fn, buff);

			// If the master file does not exist, circumvent all of this
			if (fileCompare < -2)
				return new GG_Framework::UI::ActorTransform();

			// If the _BB version is older or does not exist ...
			if ((fileCompare == -2) || (fileCompare == 1))
			{
				FILE* inFile = fopen(fn, "r");
				if (!inFile)
				{
					osg::notify(osg::FATAL) << "Could not find the file to read: \"" << fn << "\"\n";
					return new GG_Framework::UI::ActorTransform();
				}

				FILE* outFile = fopen(buff, "w");
				if (!outFile)
				{
					osg::notify(osg::FATAL) << "Could not find the file to write: \"" << buff << "\"\n";
					fclose(inFile);
					return new GG_Framework::UI::ActorTransform();
				}

				bool writeChars = true;	// There are some parts that we will want to skip over
				while (!feof(inFile))
				{
					char c = getc(inFile);
					if ((c == '.') || (c == '_'))
					{
						// See if the next few characters are ".lwo" or "_LOD"
						char afterBuff[5];
						memset(afterBuff, 0, 5);
						afterBuff[0] = c;
						for (int i = 1; (i < 4) && !feof(inFile); ++i)
						{
							c = getc(inFile);
							if (!feof(inFile))	// Watch for that last bad character
								afterBuff[i] = c;
						}
						if (!stricmp(afterBuff, ".lwo"))
						{
							if (writeChars)
								fputs("_BB.lwo", outFile);
							writeChars = true; // We got to the end of a filename that might have had LOD in it
							// We can start writing again
						}
						else if (!stricmp(afterBuff, "_LOD"))
						{
							if (writeChars)
								fputs("_BB.lwo", outFile);
							writeChars = false; // Ignore the rest of the filename
						}
						else
						{
							if (writeChars)
								fputs(afterBuff, outFile);
						}
					}
					else if (!feof(inFile))	// Watch for that last bad character
					{
						if (writeChars)
							fputc(c, outFile);
					}
				}

				fclose(inFile);
				fclose(outFile);
			}
		}
	}

	// Disable warnings from Bounding Box
	osg::NotifySeverity oldNS = osg::getNotifyLevel();
	osg::setNotifyLevel(osg::FATAL);

	// USE KD trees for all collision detection (KDTree is now set in ExeMain, and it seems to make the low-end speeds faster that way)
	// osgDB::Registry::instance()->setBuildKdTreesHint(osgDB::ReaderWriter::Options::BUILD_KDTREES);

	GG_Framework::UI::ActorTransform* ret = GG_Framework::UI::ActorScene::ReadFromSceneFile(localEventMap, buff);

	// Revert to the way it was (KDTree is now set in ExeMain, and it seems to make the low-end speeds faster that way)
	// osgDB::Registry::instance()->setBuildKdTreesHint(osgDB::ReaderWriter::Options::NO_PREFERENCE);
	osg::setNotifyLevel(oldNS);

	return ret;
}
//////////////////////////////////////////////////////////////////////////