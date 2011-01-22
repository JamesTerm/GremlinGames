//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The MyGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <Producer/RenderSurface>
#include "MyGraphics"

int main(int argc, char **argv )
{
    // Declare a RenderSurface
    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;

    // Optional.  Set the window size.  
    // Arguments are x, y, width, height
    // If this call is not made, the RenderSurface
    // will occupy the entire screen with no border
    rs->setWindowRectangle( 100, 100, 640, 480 );
    // Give the window a name in the border
    rs->setWindowName( "Producer Example using Render Surface" );

    // required.  This creates the window and
    // sets up the graphics context.
    rs->realize();

    // Add your own rendering code.
    MyGraphics gfx(*rs.get());
    gfx.init();

    while( rs->isRealized() )
    {
       	// If possible, RenderSurface will synchronize
    	// the application run-time to the vertical 
        // retrace signal
    	// rs->sync();

    	// Call your own draw routine
    	gfx.draw();

    	// Swap Buffers
    	rs->swapBuffers();
    }
    return 0;
}
