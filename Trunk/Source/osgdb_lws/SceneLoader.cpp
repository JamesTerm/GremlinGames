/*******************************************************
      Lightwave Scene Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "SceneLoader.h"

#include <osg/Notify>
#include <osg/PositionAttitudeTransform>
#include <osg/AnimationPath>
#include <osg/io_utils>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <fstream>
#include <sstream>
#include "AnimatedDissolve.h"

using namespace lwosg;

namespace
{

    int str_to_int(const std::string &s)
    {
        std::istringstream iss(s);
        int n;
        iss >> n;
        return n;
    }

    int hex_to_int(const std::string &s)
    {
        std::istringstream iss(s);
        int n;
        iss >> std::hex >> n;
        return n;
    }

    osg::Quat rotate_ypr(const osg::Vec3 &ypr, const osg::Vec3 pivot_rot = osg::Vec3(0, 0, 0))
    {
        osg::Quat Q1(ypr.z(), osg::Vec3(0, -1, 0));            
        osg::Quat Q2(ypr.y(), osg::Vec3(-1, 0, 0));
        osg::Quat Q3(ypr.x(), osg::Vec3(0, 0, -1));
        osg::Quat Q4(pivot_rot.z(), osg::Vec3(0, -1, 0));            
        osg::Quat Q5(pivot_rot.y(), osg::Vec3(-1, 0, 0));
        osg::Quat Q6(pivot_rot.x(), osg::Vec3(0, 0, -1));
        return Q1 * Q2 * Q3 * Q4 * Q5 * Q6;
    }

    void trim(std::string& str)
    {
        // trim any trailing control characters.
        //std::cout<<"trim string "<<str<<std::endl;
        while (!str.empty() && str[str.size()-1]<32)
        {
            // removing control character
            //std::cout<<"    removing control character "<<(int)str[str.size()-1]<<std::endl;
            str.erase(str.size()-1);
        }
        
    }

}

class Debug_LW_AnimationPathCallback : public osg::AnimationPathCallback
{
public:
	Debug_LW_AnimationPathCallback(std::string name, osg::AnimationPath* ap) :
	  osg::AnimationPathCallback(ap)
	  {
		char fn[512];
		sprintf(fn, "%s_%i_DEBUG.txt", name.c_str(), (int)this);
		m_debug_file = fopen(fn, "w");
	  }

	  virtual ~Debug_LW_AnimationPathCallback(){fclose(m_debug_file);}

	  virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
	{
		const osg::FrameStamp* fs = nv->getFrameStamp();
		if (fs)
		{
			int frame = fs->getReferenceTime() * 30.0;
			fprintf(m_debug_file, "%i", frame);
		}
		osg::AnimationPathCallback::operator ()(node, nv);
		int currFrame = getAnimationTime() * 30.0;
		fprintf(m_debug_file, "(%i) ", currFrame);

		// must call any nested node callbacks and continue subgraph traversal.
		NodeCallback::traverse(node,nv);
	}

private:
	FILE* m_debug_file;
};


SceneLoader::Object_map SceneLoader::objects_;

SceneLoader::SceneLoader()
:    capture_obj_motion_(false),
    capture_cam_motion_(false), capture_obj_dissolve_(false)
{
}

SceneLoader::SceneLoader(const Options &options)
:    capture_obj_motion_(false),
    capture_cam_motion_(false),
    options_(options), capture_obj_dissolve_(false)
{
}

osg::Group *SceneLoader::load(const std::string &filename, const osgDB::ReaderWriter::Options *options, bool search)
{
    std::string fname;

    if (search) {
        fname = osgDB::findDataFile(filename, options);
        if (fname.empty()) return 0;
    } else {
        fname = filename;
    }

    std::ifstream ifs(fname.c_str());
    if (!ifs.is_open()) return 0;

    clear();

    std::string identifier;
    while (ifs >> identifier) {
        if (identifier == "{") {
            ifs >> identifier;
            std::ws(ifs);
            std::vector<std::string> data;
            std::string data_line;
            while (std::getline(ifs, data_line)) {
                trim(data_line);
                if (data_line == "}") {
                    std::ws(ifs);
                    break;
                }
                data.push_back(data_line);
                std::ws(ifs);
            }
            if (!data.empty()) {
                if (!parse_block(identifier, data, options)) {
                    return 0;
                }
            }
        } else {
            std::string data;
            std::getline(ifs, data);
            trim(data);
            std::istringstream iss(data);
            std::ws(iss);
            std::getline(iss, data);
            trim(data);
            if (!parse_block(identifier, data, options)) {
                return 0;
            }
        }
    }

    // build camera animations
    for (Scene_camera_list::iterator ci=scene_cameras_.begin(); ci!=scene_cameras_.end(); ++ci) {
        if (!ci->motion.keys.empty()) {
            osg::ref_ptr<osg::AnimationPath> ap = new osg::AnimationPath;
            for (Motion_envelope::Key_map::const_iterator j=ci->motion.keys.begin(); j!=ci->motion.keys.end(); ++j) {
                osg::Vec3 pos(options_.csf->fix_point(j->second.position));
                //const osg::Vec3 &ypr = j->second.ypr;
                osg::AnimationPath::ControlPoint cp(pos, osg::Quat(osg::PI_2, osg::Vec3(1, 0, 0)) * rotate_ypr(j->second.ypr), j->second.scale);
                osg::notify(osg::INFO)<<"scale = "<<j->second.scale<<std::endl;
                ap->insert(j->first, cp);
            }
            camera_animations_.push_back(ap.get());
        }
    }

    // build objects and object animations
    typedef std::map<int, osg::ref_ptr<osg::PositionAttitudeTransform> > PAT_map;
    PAT_map pats;
    int j = 0;
    for (Scene_object_list::iterator i=scene_objects_.begin(); i!=scene_objects_.end(); ++i, ++j) {
        osg::ref_ptr<osg::PositionAttitudeTransform> pat = pats[j];
        if (!pat.valid()) {
            pat = new osg::PositionAttitudeTransform;
            pats[j] = pat;
        }
	   pat->setName(i->name);

        pat->addChild(i->layer_node.get());
        pat->setPivotPoint(options_.csf->fix_point(i->pivot));

        // still
        if (!i->motion.keys.empty()) {
		   Motion_envelope::Key_map::const_iterator j=i->motion.keys.begin();
            pat->setPosition(options_.csf->fix_point(j->second.position));
		  pat->setPivotPoint(options_.csf->fix_point(i->pivot));
            pat->setAttitude(rotate_ypr(j->second.ypr, i->pivot_rot));
		  pat->setScale(j->second.scale);
		  pat->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
        }

        // animation
        if (i->motion.keys.size() > 1) {
            osg::ref_ptr<osg::AnimationPath> ap = new osg::AnimationPath;
            for (Motion_envelope::Key_map::const_iterator j=i->motion.keys.begin(); j!=i->motion.keys.end(); ++j) {
                osg::Vec3 pos(options_.csf->fix_point(j->second.position));
                osg::AnimationPath::ControlPoint cp(pos, rotate_ypr(j->second.ypr, i->pivot_rot), j->second.scale);
                ap->insert(j->first, cp);
            }
            osg::ref_ptr<osg::AnimationPathCallback> apc = new osg::AnimationPathCallback(ap.get());
			apc->_firstTime = 0.0;	// So that we do not use the silly time offset from animation path
            apc->setPivotPoint(options_.csf->fix_point(i->pivot));
            pat->setUpdateCallback(apc.get());
        }

		if (i->ObjectDissolve.singleVal > 0.0) {
			AnimatedDissolveCallback::ApplyDissolve(pat.get(), i->ObjectDissolve.singleVal);
		}
		else if (i->ObjectDissolve.keys.size() > 0){
			pat->addUpdateCallback(new AnimatedDissolveCallback(i->ObjectDissolve.keys));
		}

        if (i->parent == -1) {
            root_->addChild(pat.get());
        } else {
            if (i->parent < static_cast<int>(scene_objects_.size())) {
                osg::ref_ptr<osg::PositionAttitudeTransform> parent = pats[i->parent];
                if (!parent.valid()) {
                    parent = new osg::PositionAttitudeTransform;
                    pats[i->parent] = parent;
                }
                parent->addChild(pat.get());
            } else {
                osg::notify(osg::WARN) << "Warning: lwosg::SceneLoader: invalid parent" << std::endl;
            }
        }
    }

    return root_.get();
}

bool SceneLoader::parse_block(const std::string &name, const std::string &data, const osgDB::ReaderWriter::Options *options)
{
    std::istringstream iss(data);

	// We want to make sure we are not adding things that are really a part of the camera
	static bool CAMERA_BLOCK = true;
    if (name == "AddCamera") {
        scene_cameras_.push_back(Scene_camera());
		CAMERA_BLOCK = true;
    }

    if (name == "AddNullObject") {
        osg::ref_ptr<osg::Group> nullobjnode = new osg::Group;
        nullobjnode->setName(data);
        objects_[data] = nullobjnode;
        Scene_object so;
        so.layer_node = nullobjnode.get();
        scene_objects_.push_back(so);
		CAMERA_BLOCK = false;
    }

    if (name == "LoadObjectLayer") {
        unsigned layer;
        iss >> layer;
        std::ws(iss);
        std::string filename;
        std::getline(iss, filename);
        
        // trim any trailing control characters.        
        trim(filename);

        if (!filename.empty()) 
        {
            osg::ref_ptr<osg::Group> objnode;

            Object_map::const_iterator i = objects_.find(filename);

            if (i == objects_.end()) {

                osg::notify(osg::NOTICE) << "Loading object \"" << filename << "\"" << std::endl;

                objnode = dynamic_cast<osg::Group *>(osgDB::readNodeFile(filename, options));
                if (!objnode.valid())
				{
					// Rather than returning false, keep loading with an empty group as a place holder
					osg::notify(osg::WARN) << "Failed to load object \"" << filename << "\", using empty group" << std::endl;
					objnode = new osg::Group;
				}

                objects_[filename] = objnode;

            } else {
                objnode = i->second;
				osg::notify(osg::NOTICE) << "REUSING object \"" << filename << "\"" << std::endl;
            }


            Scene_object so;
            std::ostringstream oss;
            oss << filename << "." << layer;
            so.name = oss.str();
			if (layer > objnode->getNumChildren()) 
			{
				osg::notify(osg::WARN) << "Warning: lwosg::SceneLoader: layer " << layer << " does not exist in object, using empty GROUP" << filename << std::endl;
				so.layer_node = new osg::Group;
			}
			else
				so.layer_node = objnode->getChild(layer-1);
            if (so.layer_node.valid()) {            
                scene_objects_.push_back(so);
				CAMERA_BLOCK = false;
            }
			else
				CAMERA_BLOCK = true;
        }
    }

    if (name == "PivotPosition") {
        if (!scene_objects_.empty()) {
            osg::Vec3 pivot;
            iss >> pivot.x() >> pivot.y() >> pivot.z();
			if (!CAMERA_BLOCK)
				scene_objects_.back().pivot = pivot;
        }
    }

    if (name == "PivotRotation") {
        if (!scene_objects_.empty()) {
            osg::Vec3 pivot;
            iss >> pivot.x() >> pivot.y() >> pivot.z();
			if (!CAMERA_BLOCK)
				scene_objects_.back().pivot_rot = pivot * (osg::PI / 180.0f);
        }
    }

    if (name == "ParentItem") {
        if (!scene_objects_.empty()) {
            std::string id;
            iss >> id;
            if (id.length() == 8) {
                if (id[0] == '1') {
                    id.erase(0, 1);
                    if (!CAMERA_BLOCK) scene_objects_.back().parent = hex_to_int(id);
                }
            } else {
                if (!CAMERA_BLOCK) scene_objects_.back().parent = str_to_int(id);
            }
        }
    }

    if (name == "NumChannels") {
        iss >> channel_count_;
    }

    if (name == "Channel") {
        iss >> current_channel_;
    }

    if (name == "ObjectMotion") {
        capture_obj_motion_ = true;
    }

    if (name == "CameraMotion") {
        capture_cam_motion_ = true;
    }

	if (name == "ObjectDissolve")
	{
		std::string val;
		iss >> val;
		if (val != "(envelope)")
		{
			scene_objects_.back().ObjectDissolve.singleVal = atof(val.c_str());
			if (scene_objects_.back().ObjectDissolve.singleVal < 0.0)
				scene_objects_.back().ObjectDissolve.singleVal = 0.0;
			else if (scene_objects_.back().ObjectDissolve.singleVal > 1.0)
				scene_objects_.back().ObjectDissolve.singleVal = 1.0;
		}
		else
			capture_obj_dissolve_ = true;
	}

    return true;
}

bool SceneLoader::parse_block(const std::string &name, const std::vector<std::string> &data, const osgDB::ReaderWriter::Options *options)
{
    if (name == "Envelope") {
        if (((capture_obj_motion_ && !scene_objects_.empty()) ||
            (capture_cam_motion_ && !scene_cameras_.empty())) && 
            (data.size() >= 2)) {

            Motion_envelope::Key_map &keys = capture_obj_motion_ ? scene_objects_.back().motion.keys : scene_cameras_.back().motion.keys;

            if (current_channel_ >= (channel_count_ - 1)) {
                capture_obj_motion_ = false;
                capture_cam_motion_ = false;
            }

            for (unsigned i=1; i<data.size(); ++i) {
                std::istringstream iss(data[i]);
                std::string key_id;
                iss >> key_id;
                if (key_id == "Key") {
                    float value;
                    double time;
                    if (iss >> value >> time) {
						// Watch for floating point precision in time (sometimes LW writes the number SLIGHTLY off)
						int t = (int)(time*600.0+0.5);
						time = (double)t / 600.0;
                        switch (current_channel_) {
                            case 0: keys[time].position.x() = value; break;
                            case 1: keys[time].position.y() = value; break;
                            case 2: keys[time].position.z() = value; break;
                            case 3: keys[time].ypr.x() = value; break;
                            case 4: keys[time].ypr.y() = value; break;
                            case 5: keys[time].ypr.z() = value; break;
                            case 6: keys[time].scale.x() = value; break;
                            case 7: keys[time].scale.z() = value; break;	//< Note that y and z are FLIPPED!
                            case 8: keys[time].scale.y() = value; break;	//< This is because of LW being Y up?
                            default: ;
                        }
                    }
                }
            }
        }
		else if (capture_obj_dissolve_ && (data.size() >= 2)) {
			capture_obj_dissolve_ = false;
			Dissolve_Key_map &keys = scene_objects_.back().ObjectDissolve.keys;
			for (unsigned i=1; i<data.size(); ++i) {
				std::istringstream iss(data[i]);
				std::string key_id;
				iss >> key_id;
				if (key_id == "Key") {
					float value;
					double time;
					if (iss >> value >> time) {
						// Watch for floating point precision in time (sometimes LW writes the number SLIGHTLY off)
						int t = (int)(time*600.0+0.5);
						time = (double)t / 600.0;
						keys[time] = value;
					}
				}
			}
		}
    }
    return true;
}

void SceneLoader::clear()
{
    root_ = new osg::Group;
    // objects_.clear();
    scene_objects_.clear();
    scene_cameras_.clear();
    camera_animations_.clear();
    channel_count_ = 0;
    current_channel_ = 0;
}
