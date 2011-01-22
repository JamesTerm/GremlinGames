#ifndef PBUFFER_TEXTURE_2D_H
#define PBUFFER_TEXTURE_2D_H

#include <Producer/RenderSurface>

#include <osg/Texture2D>
#include <osg/State>

class PBufferTexture2D : public osg::Texture2D
{
    public:
        PBufferTexture2D( Producer::RenderSurface *pbuffer );
        virtual void apply(osg::State& state) const;

    private:
        Producer::ref_ptr<Producer::RenderSurface> _pbuffer;
};

#endif
