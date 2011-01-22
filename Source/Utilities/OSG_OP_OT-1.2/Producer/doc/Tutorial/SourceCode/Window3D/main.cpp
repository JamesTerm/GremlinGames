#include <iostream>

#include <Producer/Window3D>

#include <GL/gl.h>
#include <GL/glu.h>

#include "MyGraphics"

using namespace Producer;

#if 0
// Handle to an OpenGL display list that draws a cube

static void draw( Window3D &w3d, GLuint object )
{
    // Set up the viewport
    glViewport( 0, 0, w3d.width(), w3d.height() );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set up the Projection matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    double aspect = double(w3d.width())/double(w3d.height());
    gluPerspective( 45, aspect, 0.1, 100.0 );

    // Set up the model view matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( 0, 0, 7, 0, 0, 0, 0, 1, 0 );

    // Transform the sceen by the trackball matrix and
    // draw.
    glPushMatrix();
    glMultMatrixf( w3d.getTrackballMatrix() );
    glCallList( object );
    glPopMatrix();
}
#endif


class kb : public Window3D::KeyboardCallback {
    public:
	kb() : Window3D::KeyboardCallback() {}
	void operator() (KeySymbol key )
	{
		std::cout << "Key: " << key << std::endl;
	};
};

int main()
{
    Window3D w3d( "Testing Window 3D - with OpenGL", 20, 20, 800, 600 );

    w3d.enableTrackball();
    w3d.setKeyboardCallback( new kb );

	MyGraphics gfx;

	/*
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glEnable( GL_LIGHTING );
    GLfloat position[] = { 0.0, 0.0, 1.0, 0.0 };
    glEnable( GL_LIGHT0 );
    glLightfv( GL_LIGHT0, GL_POSITION, position );
    glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );

    cube = MakeCube();
	*/

    while( !w3d.done() )
    {
        w3d.sync();
	    gfx.draw( w3d );
        w3d.swapBuffers();
    }
    return 0;
}
