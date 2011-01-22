#include "PBufferTexture2D.h"

PBufferTexture2D::PBufferTexture2D( Producer::RenderSurface *pbuffer ):
    _pbuffer(pbuffer) 
{

}

void PBufferTexture2D::apply(osg::State& state) const
{
    const unsigned int contextID = state.getContextID();

    TextureObject* textureObject = getTextureObject(contextID);
    if( textureObject == 0 )
    {
        GLuint format =
            _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBTexture ? GL_RGB:
            _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBATexture ? GL_RGBA : 0 ;

format = GL_DEPTH_COMPONENT;

        unsigned int width  = _pbuffer->getWindowWidth();
        unsigned int height = _pbuffer->getWindowHeight();

        _textureObjectBuffer[contextID] = textureObject =
            generateTextureObject( contextID, GL_TEXTURE_2D, 1, format, width, height, 1, 0 );

        textureObject->bind();
        applyTexParameters( GL_TEXTURE_2D, state);

        //glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, 0 );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 );
        textureObject->setAllocated(true);
    }
    else
    {
        textureObject->bind();
        _pbuffer->bindPBufferToTexture( Producer::RenderSurface::FrontBuffer );
    }
}


