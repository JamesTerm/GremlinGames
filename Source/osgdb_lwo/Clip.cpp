/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Clip.h"
#include <osgDB/ReadFile>
#include <osg/Notify>

using namespace lwosg;

Clip::Clip(const lwo2::FORM::CLIP *clip)
{
    if (clip) {
        compile(clip);
    }
}

void Clip::compile(const lwo2::FORM::CLIP *clip)
{
    for (iff::Chunk_list::const_iterator j=clip->attributes.begin(); j!=clip->attributes.end(); ++j) {
        const lwo2::FORM::CLIP::STIL *stil = dynamic_cast<const lwo2::FORM::CLIP::STIL *>(*j);
        if (stil) still_filename_ = stil->name.name;
    }
}

osg::Image* Clip::getStillImage(const osgDB::ReaderWriter::Options* db_options) const
{
	if (still_image_.valid())
		return still_image_.get();
	else if (!still_filename_.empty())
	{
		osg::notify(osg::NOTICE) << "Reading Image: " << still_filename_ << std::endl;
		osg::Image* ret = osgDB::readImageFile(still_filename_, db_options);
		if (!ret)
			osg::notify(osg::FATAL) << "Failed To Read Image: " << still_filename_ << std::endl;
		return ret;
	}
	else
		return NULL;
}

bool Clip::isStillImageValid() const
{
	if (still_image_.valid())
		return true;
	else if (!still_filename_.empty())
		return true;
	else
		return false;
}

bool Clip::ProvideAlphaChannel(Clip* origClip, const osgDB::ReaderWriter::Options* db_options, osg::Image* transImage)
{
	if (!origClip) return false;
	if (!transImage) return false;
	if (!origClip->still_image_.valid())
	{
		osg::notify(osg::NOTICE) << "Reading Image: " << origClip->still_filename_ << std::endl;
		origClip->still_image_ = osgDB::readImageFile(origClip->still_filename_, db_options);
		if (!origClip->still_image_.valid())
			osg::notify(osg::FATAL) << "Failed To Read Image: " << origClip->still_filename_ << std::endl;
	}
	if (!origClip->still_image_.valid()) return false;

	// Get the size of the old image
	osg::Image* oldImage = origClip->still_image_.get();
	int s = oldImage->s();
	int t = oldImage->t();

	// Resize the trans Image to make sure it is the same size
	transImage->scaleImage(s, t, transImage->r());

	// Make a new image that has an alpha
	osg::ref_ptr<osg::Image> newImage = new osg::Image();
	newImage->allocateImage(s, t, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	for (int row = 0; row < t; ++row)
	{
		for (int col = 0; col < s; ++col)
		{
			unsigned char* newImageData = newImage->data(col, row);
			unsigned char* oldImageData = oldImage->data(col, row);
			unsigned char* transImageData = transImage->data(col, row);

			unsigned char r = oldImageData[0];
			unsigned char g = oldImageData[1];
			unsigned char b = oldImageData[2];
			unsigned char a = (unsigned char)255 - transImageData[0];

			newImageData[0] = r;
			newImageData[1] = g;
			newImageData[2] = b;
			newImageData[3] = a;
		}
	}

	// This is now MY image, I will replace origClip
	still_image_ = newImage.get();

	return true;
}

bool Clip::ProvideSolidColor(Clip* origClip, osg::Vec3 base_color, const osgDB::ReaderWriter::Options* db_options)
{
	if (!origClip) return false;
	if (!origClip->still_image_.valid())
	{
		osg::notify(osg::WARN) << "Reading Image: " << origClip->still_filename_ << std::endl;
		origClip->still_image_ = osgDB::readImageFile(origClip->still_filename_, db_options);
		if (!origClip->still_image_.valid())
			osg::notify(osg::FATAL) << "Failed To Read Image: " << origClip->still_filename_ << std::endl;
	}
	if (!origClip->still_image_.valid()) return false;

	// Create a new image  of the same size with RGBA and allocate it
	osg::Image* oldImage = origClip->still_image_.get();
	int s = oldImage->s();
	int t = oldImage->t();
	osg::ref_ptr<osg::Image> newImage = new osg::Image();
	newImage->allocateImage(s, t, 1, GL_RGBA, GL_UNSIGNED_BYTE);

	// We are going to set all of the colors the same for each pixel
	unsigned char r = (unsigned char)(base_color[0] * 255.0f);
	unsigned char g = (unsigned char)(base_color[1] * 255.0f);
	unsigned char b = (unsigned char)(base_color[2] * 255.0f);

	for (int row = 0; row < t; ++row)
	{
		for (int col = 0; col < s; ++col)
		{
			unsigned char* newImageData = newImage->data(col, row);
			unsigned char* oldImageData = oldImage->data(col, row);

			// Set the other 3 to the base color provided, remember it is BGRA
			newImageData[0] = r;
			newImageData[1] = g;
			newImageData[2] = b;

			// Set the ALPHA to the original's r value (assumes all are the same), but flip so black (0) is 255 alpha
			newImageData[3] = 255 - oldImageData[0];
		}
	}

	// Replace MY image, I will replace origClip
	still_image_ = newImage.get();

	return true;
}