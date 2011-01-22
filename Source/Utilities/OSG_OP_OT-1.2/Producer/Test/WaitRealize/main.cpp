/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

#include <stdio.h>
#include <unistd.h>
#ifdef __MINGW32__
#include <stdlib.h>
#define sleep(t)  _sleep((t)*1000)
#endif
#include <OpenThreads/Thread>
#include <Producer/RenderSurface>

using namespace Producer;

class MyThread : public OpenThreads::Thread
{
    public:
        MyThread( RenderSurface &rs): _rs(rs) {}

        void run( void )
        {
            printf( "Thread %d running, waiting for realize...\n", getpid() );
            _rs.waitForRealize();
            printf( "Thread %d: waitForRealize returned\n", getpid());
            for( ;; ) sleep(1);
        }

    private :
        RenderSurface &_rs;
};

class RealizeCallback : public RenderSurface::Callback
{
    public:
        void operator()(const RenderSurface & )
        {
            printf( "Realize callback called\n" );
        }
};

int main()
{
    ref_ptr<RenderSurface> rs = new RenderSurface();
    rs->setWindowRectangle(0,0,200,200);
    rs->setRealizeCallback( new RealizeCallback );
    MyThread y(*rs.get());
    printf( "Master thread %d running\n", getpid() );

    y.startThread();

    sleep(2);
    printf( "master realizing render surface\n" );
    rs->realize();

    sleep(2);
    printf( "master canceling slave thread\n" );
    y.cancel();
    y.join();

    puts( "DONE" );
    return 0;

}
