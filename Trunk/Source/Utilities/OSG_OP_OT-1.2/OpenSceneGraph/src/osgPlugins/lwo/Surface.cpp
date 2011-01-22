/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Surface.h"

#include <osg/Material>
#include <osg/CullFace>
#include <osg/Texture2D>
#include <osg/TexEnvCombine>
#include <osg/TexGen>
#include <osg/BlendFunc>
#include <osg/Notify>

#include <osgFX/SpecularHighlights>

#include <osgDB/ReadFile>

using namespace lwosg;
osg::ref_ptr<const osgDB::ReaderWriter::Options> Surface::db_options_;

namespace
{

    osg::Texture::WrapMode osg_wrap_mode(Image_map::Wrap_type w)
    {
        switch (w) {
            case Image_map::RESET: return osg::Texture::CLAMP;
            case Image_map::REPEAT: return osg::Texture::REPEAT;
            case Image_map::MIRROR: return osg::Texture::MIRROR;
            case Image_map::EDGE: return osg::Texture::CLAMP_TO_EDGE;
            default: return osg::Texture::REPEAT;
        };
    }

}

Surface::Surface()
:    base_color_(0.784f, 0.784f, 0.784f),
    diffuse_(1.0f),
    luminosity_(0),
    specularity_(0),
    reflection_(0),
    transparency_(0),
    translucency_(0),
    glossiness_(0.4f),
    sidedness_(FRONT_ONLY),
    max_smoothing_angle_(0),
	hasTransMap_(Block::NORMAL)
{
}

Surface::Surface(const lwo2::FORM::SURF *surf, Clip_map &clips)
:    base_color_(0.784f, 0.784f, 0.784f),
    diffuse_(1.0f),
    luminosity_(0),
    specularity_(0),
    reflection_(0),
    transparency_(0),
    translucency_(0),
    glossiness_(0.4f),
    sidedness_(FRONT_ONLY),
    max_smoothing_angle_(0),
	hasTransMap_(Block::NORMAL)
{
    compile(surf, clips);
}

void Surface::compile(const lwo2::FORM::SURF *surf, Clip_map &clips)
{
    // invalidate the stateset so it will be rebuilt
    stateset_ = 0;

    name_ = surf->name;

    for (iff::Chunk_list::const_iterator j=surf->attributes.begin(); j!=surf->attributes.end(); ++j) {

        const lwo2::FORM::SURF::COLR *colr = dynamic_cast<const lwo2::FORM::SURF::COLR *>(*j);
        if (colr) base_color_ = osg::Vec3(colr->base_color.red, colr->base_color.green, colr->base_color.blue);

        const lwo2::FORM::SURF::DIFF *diff = dynamic_cast<const lwo2::FORM::SURF::DIFF *>(*j);
        if (diff) diffuse_ = diff->intensity.fraction;

        const lwo2::FORM::SURF::LUMI *lumi = dynamic_cast<const lwo2::FORM::SURF::LUMI *>(*j);
        if (lumi) luminosity_ = lumi->intensity.fraction;

        const lwo2::FORM::SURF::SPEC *spec = dynamic_cast<const lwo2::FORM::SURF::SPEC *>(*j);
        if (spec) specularity_ = spec->intensity.fraction;

        const lwo2::FORM::SURF::REFL *refl = dynamic_cast<const lwo2::FORM::SURF::REFL *>(*j);
        if (refl) reflection_ = refl->intensity.fraction;

        const lwo2::FORM::SURF::TRAN *tran = dynamic_cast<const lwo2::FORM::SURF::TRAN *>(*j);
        if (tran) transparency_ = tran->intensity.fraction;

        const lwo2::FORM::SURF::TRNL *trnl = dynamic_cast<const lwo2::FORM::SURF::TRNL *>(*j);
        if (trnl) translucency_ = trnl->intensity.fraction;

        const lwo2::FORM::SURF::GLOS *glos = dynamic_cast<const lwo2::FORM::SURF::GLOS *>(*j);
        if (glos) glossiness_ = glos->glossiness.fraction;

        const lwo2::FORM::SURF::SIDE *side = dynamic_cast<const lwo2::FORM::SURF::SIDE *>(*j);
        if (side) sidedness_ = static_cast<Sidedness>(side->sidedness);

        const lwo2::FORM::SURF::SMAN *sman = dynamic_cast<const lwo2::FORM::SURF::SMAN *>(*j);
        if (sman) max_smoothing_angle_ = sman->max_smoothing_angle.radians;

        const lwo2::FORM::SURF::VCOL *vcol = dynamic_cast<const lwo2::FORM::SURF::VCOL *>(*j);
        if (vcol) {
            color_map_intensity_ = vcol->intensity.fraction;
            color_map_type_ = std::string(vcol->vmap_type.id, 4);
            color_map_name_ = vcol->name;
        }

        const lwo2::FORM::SURF::BLOK *blok = dynamic_cast<const lwo2::FORM::SURF::BLOK *>(*j);
        if (blok) {
            Block new_block(blok);
            if (new_block.get_type() == "IMAP") {
                Clip_map::iterator i = clips.find(new_block.get_image_map().image_map);
                if (i != clips.end()) {
                    new_block.get_image_map().clip = &i->second;
                } else {
                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: cannot find clip number " << new_block.get_image_map().image_map << std::endl;
                }                
            }
            blocks_.insert(Block_map::value_type(new_block.get_ordinal(), new_block));
        }
    }

	// Loop through the blocks again to find the COLR and TRAN blocks
	Block* colrBlock = NULL;
	Block* tranBlock = NULL;
	for (Block_map::iterator i=blocks_.begin(); i!=blocks_.end(); ++i) {

		Block &block = i->second;
		if (!block.enabled()) {
			continue;
		}

		if (block.get_type() == "IMAP") {
			if (block.get_channel() == "COLR") {
				colrBlock = &block;
			}
			else if (block.get_channel() == "TRAN") {
				tranBlock = &block;
			}
		}
	}

	hasTransMap_ = Block::NORMAL;
	if (tranBlock)
	{
		if (colrBlock)
		{
			// Create a new clip in the map
			int tempIndex = Image_map::GetNextTempClipIndex();
			Clip* newClip = &(clips[tempIndex]);

			// Provide the trans image as the alpha channel for the existing color map
			osg::ref_ptr<osg::Image> transImage = tranBlock->get_image_map().clip->getStillImage(db_options_.get());
			if (newClip->ProvideAlphaChannel(colrBlock->get_image_map().clip, db_options_.get(), transImage.get()))
			{
				colrBlock->get_image_map().image_map = tempIndex;
				colrBlock->get_image_map().clip = newClip;

				// TODO: It would be good to check to make sure the mappings are the same and warn the user if not
				// At this point, we will just always warn them
				osg::notify(osg::NOTICE) << "Warning: Providing Trans map for the Color Alpha Channel, ignoring TRANS mapping and using COLOR mapping" << std::endl;
				hasTransMap_ = tranBlock->opacity_type_;
				if (hasTransMap_ != Block::ADDITIVE)
					hasTransMap_ = Block::ALPHA;
				colrBlock->opacity_type_ = tranBlock->opacity_type_;
			}
		}
		else
		{
			// Create a new clip in the map
			int tempIndex = Image_map::GetNextTempClipIndex();
			Clip* newClip = &(clips[tempIndex]);

			// Convert the transparency image to a color map
			if (newClip->ProvideSolidColor(tranBlock->get_image_map().clip, base_color_, db_options_.get()))
			{
				tranBlock->get_image_map().image_map = tempIndex;
				tranBlock->get_image_map().clip = newClip;
				osg::notify(osg::INFO) << "Warning: Replacing Solid Color with TRANS Image in Alpha Channel" << std::endl;
				tranBlock->ChangeToColorBlock();
				hasTransMap_ = tranBlock->opacity_type_;
				if (hasTransMap_ != Block::ADDITIVE)
					hasTransMap_ = Block::ALPHA;
			}
		}
	}
}

void Surface::generate_stateset(int max_tex_units, bool force_arb_compression) const
{
    if (!stateset_.valid()) {

        stateset_ = new osg::StateSet;

        osg::ref_ptr<osg::Material> material = new osg::Material;
        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(base_color_ * diffuse_, 1-transparency_));
        material->setAmbient(osg::Material::FRONT_AND_BACK, material->getDiffuse(osg::Material::FRONT_AND_BACK));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(specularity_, specularity_, specularity_, 1));
        material->setShininess(osg::Material::FRONT_AND_BACK, powf(2, 10 * glossiness_ + 2));
        material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(base_color_ * luminosity_, 1-transparency_));
        stateset_->setAttributeAndModes(material.get());

        if (!color_map_name_.empty()) {
            material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
        }

		if (transparency_ > 0 || (hasTransMap_!=Block::NORMAL)) {
            osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
            bf->setFunction(osg::BlendFunc::SRC_ALPHA, 
				(hasTransMap_==Block::ADDITIVE) ? osg::BlendFunc::ONE : osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
            stateset_->setAttributeAndModes(bf.get());
            stateset_->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }

        if (sidedness_ == FRONT_AND_BACK) {
            stateset_->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
        } else {
            osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
            switch (sidedness_) {
                case NONE:  cf->setMode(osg::CullFace::FRONT_AND_BACK); break;
                case FRONT_ONLY: cf->setMode(osg::CullFace::BACK); break;
                case BACK_ONLY:  cf->setMode(osg::CullFace::FRONT); break;
                default: ;
            }
            stateset_->setAttributeAndModes(cf.get());
        }

        int unit = 0;
        for (Block_map::const_iterator i=blocks_.begin(); i!=blocks_.end(); ++i) {

            const Block &block = i->second;
            if (!block.enabled()) {
                continue;
            }

            if (block.get_type() == "IMAP") {
                if (block.get_channel() == "COLR") {
                    if (block.get_image_map().clip) {
						osg::ref_ptr<osg::Image> image = block.get_image_map().clip->getStillImage(db_options_.get());
						if (image.valid()) {

                            if (unit >= max_tex_units && max_tex_units > 0) {
                                osg::notify(osg::WARN) << "Warning: lwosg::Surface: maximum number of texture units (" << max_tex_units << ") has been reached, skipping incoming blocks" << std::endl;
                                break;
                            }

                            osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
                            if (force_arb_compression)
                                texture->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
                            texture->setImage(image.get());
                            texture->setWrap(osg::Texture::WRAP_S, osg_wrap_mode(block.get_image_map().width_wrap));
                            texture->setWrap(osg::Texture::WRAP_T, osg_wrap_mode(block.get_image_map().height_wrap));
                            texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
                            texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
                            stateset_->setTextureAttributeAndModes(unit, texture.get());

                            osg::ref_ptr<osg::TexEnvCombine> tec = new osg::TexEnvCombine;
                            switch (block.get_opacity_type()) {
                                case Block::NORMAL:
                                    {
                                        float s = block.get_opacity_amount();
                                        if (unit == 0) {
                                            tec->setCombine_RGB(osg::TexEnvCombine::MODULATE);
                                            osg::Vec3 color(diffuse_, diffuse_, diffuse_);
                                            color = color * s + base_color_ * (1 - s);
                                            material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(color, 1-transparency_));
                                            material->setAmbient(osg::Material::FRONT_AND_BACK, material->getDiffuse(osg::Material::FRONT_AND_BACK));
                                            material->setColorMode(osg::Material::OFF);
                                        } else {
                                            tec->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
                                            tec->setConstantColor(osg::Vec4(s, s, s, s));
                                        }
                                    }
                                    break;

                                case Block::ADDITIVE:
                                    tec->setCombine_RGB(osg::TexEnvCombine::ADD);
                                    break;

                                case Block::SUBTRACTIVE:
                                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: 'Subtractive' blending mode is not supported, falling back to 'Difference' mode" << std::endl;
                                case Block::DIFFERENCE:
                                    tec->setCombine_RGB(osg::TexEnvCombine::SUBTRACT);
                                    break;

                                case Block::MULTIPLY:
                                    tec->setCombine_RGB(osg::TexEnvCombine::MODULATE);
                                    break;

                                case Block::DIVIDE:
                                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: 'Divide' blending mode is not supported" << std::endl;
                                    break;

                                case Block::ALPHA:
                                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: 'Alpha' blending mode is not supported" << std::endl;
                                    break;

                                case Block::TEXTURE_DISPLACEMENT:
                                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: 'Texture Displacement' blending mode is not supported" << std::endl;
                                    break;

                                default: ;
                            };

                            stateset_->setTextureAttributeAndModes(unit, tec.get());
                            ++unit;
                        }
                    }
                } 
				else if (block.get_channel() == "TRAN") {
					osg::notify(osg::WARN) << "Warning: lwosg::Surface: texture channels of type 'TRAN' are compiled into the COLR map" << std::endl;
				}
				else {
                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: texture channels of type '" << block.get_channel() << "' are not supported, this block will be ignored" << std::endl;
                }
            }
        }
    }
}

osg::Group *Surface::apply(osg::Geometry *geo, const VertexMap_map *texture_maps, const VertexMap_map *rgb_maps, const VertexMap_map *rgba_maps, int max_tex_units, bool use_osgfx, bool force_arb_compression, const VertexMap_binding_map &texmap_bindings) const
{
    int num_points = 0;

    if (geo->getVertexArray()) {
        num_points = static_cast<int>(geo->getVertexArray()->getNumElements());
    }

    generate_stateset(max_tex_units, force_arb_compression);
    geo->setStateSet(stateset_.get());

    int unit = 0;
    for (Block_map::const_iterator i=blocks_.begin(); i!=blocks_.end(); ++i) {
        const Block &block = i->second;
        if (block.get_type() == "IMAP" && block.get_channel() == "COLR" && block.get_image_map().clip) {
            if (block.get_image_map().clip->isStillImageValid()) {
                if (block.get_image_map().projection == Image_map::UV) {
                    VertexMap_map::const_iterator i = texture_maps->find(block.get_image_map().uv_map);
                    if (i != texture_maps->end()) {
                        geo->setTexCoordArray(unit, i->second->asVec2Array(num_points));
                    } else {
                        osg::notify(osg::WARN) << "Warning: lwosg::Surface: surface '" << name_ << "' needs texture map named '" << block.get_image_map().uv_map << "' but I can't find it" << std::endl;
                    }
                }
                ++unit;
            }
        }
    }

    for (VertexMap_binding_map::const_iterator vi=texmap_bindings.begin(); vi!=texmap_bindings.end(); ++vi)
    {
        for (VertexMap_map::const_iterator j=texture_maps->begin(); j!=texture_maps->end(); ++j)
        {
            if (j->first == vi->first)
            {
                if (geo->getTexCoordArray(vi->second) != 0)
                {
                    osg::notify(osg::WARN) << "Warning: lwosg::Surface: explicing binding of texture map '" << vi->first << "' to texunit " << vi->second << " will replace existing texture map" << std::endl;
                }
                geo->setTexCoordArray(vi->second, j->second->asVec2Array(num_points));
            }
            else
            {
                osg::notify(osg::WARN) << "Warning: lwosg::Surface: explicit binding of texture map '" << vi->first << "' to texunit " << vi->second << " was requested but there is no such map in this LWO file" << std::endl;
            }
        }
    }
    
    osg::Vec4 color = osg::Vec4(base_color_, 1-transparency_);

    const VertexMap_map *color_maps = 0;

    if (color_map_type_ == "RGB ") {
        color_maps = rgb_maps;
    }

    if (color_map_type_ == "RGBA") {
        color_maps = rgba_maps;
    }

    if (color_maps) {
        VertexMap_map::const_iterator i = color_maps->find(color_map_name_);
        if (i != color_maps->end() && !i->second->empty()) {
            geo->setColorArray(i->second->asVec4Array(num_points, color * color_map_intensity_, color * color_map_intensity_));
            geo->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        } else {
            osg::notify(osg::WARN) << "Warning: lwosg::Surface: surface '" << name_ << "' needs color map named '" << color_map_name_ << "' but I can't find it" << std::endl;
        }
    }

    // create osgFX specularity if needed
    if (use_osgfx && glossiness_ > 0 && specularity_ > 0) {
        if (unit >= max_tex_units && max_tex_units > 0) {
            osg::notify(osg::WARN) << "Warning: lwosg::Surface: can't apply osgFX specular lighting: maximum number of texture units (" << max_tex_units << ") has been reached" << std::endl;
        } else {
            osg::ref_ptr<osgFX::SpecularHighlights> sh = new osgFX::SpecularHighlights;
            sh->setTextureUnit(unit);
            osg::Material *material = dynamic_cast<osg::Material *>(stateset_->getAttribute(osg::StateAttribute::MATERIAL));
            if (material) {
                sh->setSpecularColor(material->getSpecular(osg::Material::FRONT_AND_BACK));
                sh->setSpecularExponent(powf(2, 10 * glossiness_ + 2));
                material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 0));
                material->setShininess(osg::Material::FRONT_AND_BACK, 0);
            }
            return sh.release();
        }
    }

    return 0;
}
