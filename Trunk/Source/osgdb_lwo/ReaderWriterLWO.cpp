// -*-c++-*-

/*
 * Lightwave Object loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 * Improved LWO2 reader is (C) 2003-2004 Marco Jez <marco.jez@poste.it>
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 */

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include "..\CompilerSettings.h"

#include <string>
#include <memory>
#include <sstream>
#include <algorithm>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/lod>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>


#include "Converter.h"
#include "VertexMap.h"

#include "old_lw.h"
#include "old_Lwo2.h"

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
//////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////

class ReaderWriterLWO : public osgDB::ReaderWriter
{
private:
	bool Read_LOD_Layers(const char* LOD_substring, osg::Group* thisModel, osg::Group* mainModel) const
	{
		// Watch for proper pointers
		if (!thisModel || !mainModel || !LOD_substring)
			return false;

		// Make sure we can get the ranges
		float minRange, maxRange;
		if (sscanf(LOD_substring, "_LOD-%f-%f", &minRange, &maxRange) != 2)
			return false;

		// Make sure ranges make sense
		if ((minRange < 0.0) || (maxRange <= minRange))
			return false;

		// Work through each layer
		unsigned numObjLayers = thisModel->getNumChildren();
		for (unsigned i = 0; i < numObjLayers; ++i)
		{
			osg::Node* thisLayer = thisModel->getChild(i);
			osg::ref_ptr<osg::LOD> layer_LOD;
			if (i >= mainModel->getNumChildren())
			{
				layer_LOD = new osg::LOD;
				mainModel->addChild(layer_LOD.get());
			}
			else
				layer_LOD = dynamic_cast<osg::LOD*>(mainModel->getChild(i));

			layer_LOD->addChild(thisLayer, minRange, maxRange);
		}
		return true;
	}

	//! See if we can merge a LOD (only if reading the 0 level of detail)
	//! This is based on the naming convention:
	//! filenamebase_LOD-start-end.lwo
	//! e.g myFile_LOD-0-100.lwo
	//! if the file matches this pattern this function will look in the same directory for other files
	ReadResult readLWO_LOD(const std::string& fileName, const osgDB::ReaderWriter::Options* options, const osgDB::ReaderWriter::Options* local_opt) const
	{
		// Find the folder that contains this path
		std::string filePath = osgDB::getFilePath(fileName);
		std::string fileNameBase = osgDB::getSimpleFileName(fileName);

		// Watch for LOD in the name
		unsigned LOD_substr_pos = fileNameBase.find("_LOD-0-");
		if (LOD_substr_pos != std::string::npos)
		{
			// Find all of the files in this directory
			osgDB::DirectoryContents dirContents = osgDB::getDirectoryContents(filePath);

			// Loop through the files to see if there are any others besides this one to have in an LOD node
			bool foundOtherLOD = false;

			osgDB::DirectoryContents::iterator pos;
			ReadResult ret(new osg::Group);
			for (pos = dirContents.begin(); pos != dirContents.end(); ++pos)
			{
				std::string thisFile = (*pos);
				// Ignore our own name
				if (thisFile != fileNameBase)
				{
					if (fileNameBase.compare(0, LOD_substr_pos+5, thisFile.substr(0,LOD_substr_pos+5)) == 0)
					{
						if (!foundOtherLOD)
						{
							// We only want to do this part one time
							foundOtherLOD = true;

							// We found at least one file, make sure we can read the original OK
							osg::notify(osg::NOTICE) << "LWO base LOD: " << fileName << endl;
							ReadResult baseLOD_RR = readLWO_1or2(fileName, options, local_opt);

							// Get the group and see how many layers there are
							osg::Group* baseLOD_group = baseLOD_RR.getNode() ? baseLOD_RR.getNode()->asGroup() : NULL;

							// If there are no layers, something went wrong, just return what we have
							if (!baseLOD_group)
								return baseLOD_RR;

							// Now add this file in with the other LOD nodes
							std::string fileSubstr = fileName.substr(fileName.find("_LOD-0-"));
							if (!Read_LOD_Layers(fileSubstr.c_str(), baseLOD_group, ret.getNode()->asGroup()))
								osg::notify(osg::WARN) << "Problem Reading LWO LOD file: " << fileName << std::endl;
						}
						
						// Try reading in this new file and adding it to the LOD layers
						osg::notify(osg::NOTICE) << "LWO other LOD: " << thisFile << endl;
						ReadResult this_RR = readLWO_1or2(thisFile, options, local_opt);
						osg::Group* this_Group = this_RR.getNode() ? this_RR.getNode()->asGroup() : NULL;

						// Now add this file in with the other LOD nodes
						std::string fileSubstr = thisFile.substr(LOD_substr_pos);
						if (!Read_LOD_Layers(fileSubstr.c_str(), this_Group, ret.getNode()->asGroup()))
							osg::notify(osg::WARN) << "Problem Reading LWO LOD file: " << thisFile << std::endl;
					}
				}
			}

			// We were able to build a good model to return
			if (foundOtherLOD)
				return ret;
		}

		// Just a regular file, just do the regular read
		osg::notify(osg::NOTICE) << "LWO base: " << fileName << endl;
		return readLWO_1or2(fileName, options, local_opt);
	}

	//! Try reading from the various LW options out there, either version 1 or 2
	ReadResult readLWO_1or2(const std::string& fileName, const osgDB::ReaderWriter::Options* options, const osgDB::ReaderWriter::Options* local_opt) const
	{
		if (!options || options->getOptionString() != "USE_OLD_READER") {
			ReadResult result = readNode_LWO2(fileName, local_opt);
			if (result.success()) return result;
		}

		ReadResult result = readNode_LWO1(fileName,local_opt);
		if (result.success()) return result;

		return readNode_old_LWO2(fileName, local_opt);
	}

public:
	ReaderWriterLWO()
	{
		supportsExtension("lwo","Lightwave object format");
		supportsExtension("lw","Lightwave object format");
		supportsExtension("geo","Lightwave geometry format");
	}

    virtual const char* className() const { return "Lightwave Object Reader"; }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);        
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

        // code for setting up the database path so that internally referenced file are searched for on relative paths. 
        osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
	   
		const char* contentDir = GetCWD_FromSceneFile(fileName.c_str());
		osg::notify(osg::NOTICE) << "LWO Content Directory: " << contentDir << std::endl;
	   local_opt->setDatabasePath(contentDir);
	   // local_opt->setDatabasePath(osgDB::getFilePath(fileName));

        return readLWO_LOD(fileName, options, local_opt.get());
    }

    lwosg::Converter::Options parse_options(const Options *options) const;

    virtual ReadResult readNode_LWO2(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
    virtual ReadResult readNode_old_LWO2(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
    virtual ReadResult readNode_LWO1(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;

protected:

    

};

lwosg::Converter::Options ReaderWriterLWO::parse_options(const Options *options) const
{
    lwosg::Converter::Options conv_options;

    if (options) {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt) {
            if (opt == "COMBINE_GEODES")           conv_options.combine_geodes = true;
            if (opt == "FORCE_ARB_COMPRESSION")    conv_options.force_arb_compression = true;
            if (opt == "USE_OSGFX")                conv_options.use_osgfx = true;
            if (opt == "NO_LIGHTMODEL_ATTRIBUTE")  conv_options.apply_light_model = false;
            if (opt == "BIND_TEXTURE_MAP")
            {
                std::string mapname;
                int unit;
                if (iss >> mapname >> unit)
                {
                    conv_options.texturemap_bindings.insert(lwosg::VertexMap_binding_map::value_type(mapname,  unit));
                }
            }
            if (opt == "MAX_TEXTURE_UNITS") {
                int n;
                if (iss >> n) {
                    conv_options.max_tex_units = n;
                }
            }
        }
    }

    return conv_options;
}


// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(lwo, ReaderWriterLWO)


osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode_LWO2(const std::string &fileName, const osgDB::ReaderWriter::Options *options) const
{
    lwosg::Converter::Options conv_options = parse_options(options);

    lwosg::Converter converter(conv_options, options);
    osg::ref_ptr<osg::Node> node = converter.convert(fileName);
    if (node.valid()) {
        return node.release();
    }

    return ReadResult::FILE_NOT_HANDLED;
}


osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode_old_LWO2(const std::string& fileName, const osgDB::ReaderWriter::Options*) const
{
    std::auto_ptr<Lwo2> lwo2(new Lwo2());
    if (lwo2->ReadFile(fileName))
    {
        osg::ref_ptr<Group> group = new osg::Group();
        if (lwo2->GenerateGroup(*group)) return group.release();
    }
    return ReadResult::FILE_NOT_HANDLED;
}





// collect all the data relavent to a particular osg::Geometry being created.
struct GeometryCollection
{
    GeometryCollection():
        _numPrimitives(0),
        _numPrimitivesWithTexCoords(0),
        _numPoints(0),
        _texturesActive(false),
        _vertices(osg::Vec3Array::iterator()),
        _texcoords(osg::Vec2Array::iterator()),
        _coordCount(0),
        _geom(0) {}

    int                         _numPrimitives;
    int                         _numPrimitivesWithTexCoords;
    int                         _numPoints;
    bool                        _texturesActive;
    osg::Vec3Array::iterator    _vertices;
    osg::Vec2Array::iterator    _texcoords;
    int                         _coordCount;
    osg::Geometry*              _geom;
};



// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterLWO::readNode_LWO1(const std::string& fileName, const osgDB::ReaderWriter::Options*) const
{
    lwObject* lw = lw_object_read(fileName.c_str(),osg::notify(osg::NOTICE));
    if (!lw)
        return ReadResult::FILE_NOT_HANDLED;

    osg::notify(osg::INFO) << "faces " << lw->face_cnt << std::endl;
    osg::notify(osg::INFO) << "materials " << lw->material_cnt << std::endl;
    osg::notify(osg::INFO) << "vertices " << lw->vertex_cnt << std::endl;

    typedef std::map<int,GeometryCollection> MaterialToGeometryCollectionMap;
    MaterialToGeometryCollectionMap mtgcm;
    
    // bin the indices for each material into the mtis;
    int i;
    for (i = 0; i < lw->face_cnt; ++i)
    {
        lwFace& face = lw->face[i];
        if (face.index_cnt>=3)
        {
            GeometryCollection& gc = mtgcm[face.material];
            gc._numPoints += face.index_cnt;
            gc._numPrimitives += 1;
            if (face.texcoord) gc._numPrimitivesWithTexCoords += 1;
        }
    }
    
    MaterialToGeometryCollectionMap::iterator itr;
    for(itr=mtgcm.begin(); itr!=mtgcm.end(); ++itr)
    {
        GeometryCollection& gc = itr->second;

        if (gc._numPrimitives)
        {
            lwMaterial& lw_material = lw->material[itr->first];

            gc._geom = new osg::Geometry;
            
            osg::Vec3Array* vertArray = new osg::Vec3Array(gc._numPoints);
            gc._vertices = vertArray->begin();
            gc._geom->setVertexArray(vertArray);

            // set up color.
            osg::Vec4Array* colors = new osg::Vec4Array(1);
            (*colors)[0].set(lw_material.r,
                             lw_material.g,
                             lw_material.b,
                             1.0f);
                             
            gc._geom->setColorArray(colors);
            gc._geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
            // set up texture if needed.
            if (gc._numPrimitivesWithTexCoords==gc._numPrimitives)
            {
                if (lw_material.ctex.flags && strlen(lw_material.ctex.name)!=0)
                {
                    osg::notify(osg::INFO) << "ctex " << lw_material.ctex.name << std::endl;
                    osg::Image* image = osgDB::readImageFile(lw_material.ctex.name);
                    if (image)
                    {
                        // create state
                        osg::StateSet* stateset = new osg::StateSet;

                        // create texture
                        osg::Texture2D* texture = new osg::Texture2D;
                        texture->setImage(image);

                        // texture wrap mode
                        static osg::Texture::WrapMode mode[] = {
                            osg::Texture::CLAMP,
                            osg::Texture::CLAMP,
                            osg::Texture::REPEAT,
                            osg::Texture::MIRROR
                        };
                        texture->setWrap(osg::Texture::WRAP_S,
                                         mode[lw_material.ctex.u_wrap]);
                        texture->setWrap(osg::Texture::WRAP_T,
                                         mode[lw_material.ctex.v_wrap]);

                        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
                        gc._texturesActive=true;
                        
                        gc._geom->setStateSet(stateset);

                        osg::Vec2Array* texcoordArray = new osg::Vec2Array(gc._numPoints);
                        gc._texcoords = texcoordArray->begin();
                        gc._geom->setTexCoordArray(0,texcoordArray);
                    }
					else
						osg::notify(osg::FATAL) << "Failed To Read Image: " << lw_material.ctex.name << std::endl;
                }
            }
        }        
    }
    
    
    for (i = 0; i < lw->face_cnt; ++i)
    {
        lwFace& face = lw->face[i];
        if (face.index_cnt>=3)
        {
            GeometryCollection& gc = mtgcm[face.material];
            
            osg::PrimitiveSet::Mode mode;
            switch(face.index_cnt)
            {
                case(0):
                    mode = osg::PrimitiveSet::POINTS;
                    break;
                case(1):
                    mode = osg::PrimitiveSet::POINTS;
                    break;
                case(2):
                    mode = osg::PrimitiveSet::LINES;
                    break;
                case(3):
                    mode = osg::PrimitiveSet::TRIANGLES;
                    break;
                case(4):
                    mode = osg::PrimitiveSet::QUADS;
                    break;
                default:
                    mode = osg::PrimitiveSet::POLYGON;
                    break;
            }
                        
            gc._geom->addPrimitiveSet(new osg::DrawArrays(mode,gc._coordCount,face.index_cnt));
            gc._coordCount += face.index_cnt;

            // From the spec_low.lxt :
            //   "By convention, the +X direction is to the right or east, the +Y
            //    direction is upward, and the +Z direction is forward or north"
            // However, the osg sticks to the more conventional, y to the north,
            // z upwards, x is the same - rigth/east.  To handle this difference
            // simple exchange osg_z for lwo_y, and osg_y for lwo_z.

            // add the corners in reverse order to reverse the windings, to keep the anticlockwise rotation of polys.
            int j;
            for(j=face.index_cnt-1;j>=0;--j)
            {
                (*gc._vertices++).set(lw->vertex[face.index[j]*3], lw->vertex[face.index[j]*3+2], lw->vertex[face.index[j]*3+1]);
            }
            
            if (gc._texturesActive && face.texcoord)
            {
                for(j=face.index_cnt-1;j>=0;--j)
                {
                    (*gc._texcoords++).set(face.texcoord[j*2],face.texcoord[j*2+1]);
                }            
            }            
        }
    }

    osg::Geode* geode = new osg::Geode;
    osgUtil::Tessellator tessellator;
    
    // add everthing into the Geode.    
    osgUtil::SmoothingVisitor smoother;
    for(itr=mtgcm.begin();
        itr!=mtgcm.end();
        ++itr)
    {
        GeometryCollection& gc = itr->second;
        if (gc._geom)
        {        
            tessellator.retessellatePolygons(*gc._geom);
            smoother.smooth(*gc._geom);
            
            geode->addDrawable(gc._geom);
        }

    }

    // free
    lw_object_free(lw);

    return geode;
}
