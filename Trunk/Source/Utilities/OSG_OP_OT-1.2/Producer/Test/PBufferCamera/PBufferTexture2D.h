/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef PBUFFER_TEXTURE_2D_DEF
#define PBUFFER_TEXTURE_2D_DEF

#include <Producer/RenderSurface>
#include <osg/Texture2D>

class PBufferTexture2D : public osg::Texture2D
{
    public:
        PBufferTexture2D( Producer::RenderSurface *pbuffer ):
            _pbuffer(pbuffer) {}

        virtual void apply(osg::State& state) const
        {
            const unsigned int contextID = state.getContextID();

            TextureObject* textureObject = getTextureObject(contextID);
            if( textureObject == 0 )
            {
                GLuint format = 
                    _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBTexture ? GL_RGB:
                    _pbuffer->getRenderToTextureMode() == Producer::RenderSurface::RenderToRGBATexture ? GL_RGBA : 0 ;
                unsigned int width  = _pbuffer->getWindowWidth();
                unsigned int height = _pbuffer->getWindowHeight();

                _textureObjectBuffer[contextID] = textureObject = 
                    generateTextureObject( contextID, GL_TEXTURE_2D, 1, format, width, height, 1, 0 );

                textureObject->bind();
                applyTexParameters( GL_TEXTURE_2D, state);

                glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, 0 );
                textureObject->setAllocated(true);
            }
            else
            {
                textureObject->bind();
                _pbuffer->bindPBufferToTexture( Producer::RenderSurface::FrontBuffer );
            }
        }

    private:
        Producer::ref_ptr<Producer::RenderSurface> _pbuffer;
};

#endif

