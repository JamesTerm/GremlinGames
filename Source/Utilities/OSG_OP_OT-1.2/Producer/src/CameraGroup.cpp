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

#include <string>
#include <set>
#include <Producer/CameraGroup>
#include <Producer/CameraConfig>

using namespace Producer; 

// implementation now placed in .cpp to avoid Cygwin errors.
CameraGroup::StatsHandler::StatsHandler() {}
CameraGroup::StatsHandler::~StatsHandler() {}

void CameraGroup::_initVariables()
{
    _stack_size  = 0;
    _frame_count = 0;
    _sync_count  = 0;
    _block_on_vsync = false;
    char *ptr = getenv( "PRODUCER_CAMERA_BLOCK_ON_VSYNC" );
    if( ptr )
        _block_on_vsync = true;
    //_lens = new Camera::Lens;
    _realized    = false;
    _instrumented = false;

    _threadModel = getDefaultThreadModel();
    if( (ptr = getenv( "PRODUCER_THREAD_MODEL" )))
    {
        std::string tm(ptr);
        _threadModel = 
            tm == "SingleThreaded"          ? SingleThreaded :
            tm == "ThreadPerRenderSurface"  ?  ThreadPerRenderSurface :
            tm == "ThreadPerCamera"         ?  ThreadPerCamera : SingleThreaded;
    }

    ptr = getenv( "PRODUCER_STACK_SIZE" );
    if( ptr )
    {
        setStackSize(atoi(ptr));
    }
    else
    {
#if defined(__sgi)
        // IRIX pthreads has the default stack size of 128k, which
        // is way too small for typical OSG apps, so override this
        // and set to 8Mb to ensure that we don't run out of stack space.
        setStackSize(8388608); 
#endif        
    }

}

void CameraGroup::_initLens()
{
    if( _cfg->getNumberOfCameras() == 1 )
    {
        _lens = _cfg->getCamera(0)->getLens();
        return;
    }
    
    /*
    if( !_lens.valid() && _cfg->getNumberOfCameras() > 0 )
        _lens = _cfg->getCamera(0)->getLens();
     */

    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Camera *cam = _cfg->getCamera(i);
        if( !_lens.valid()  && cam->getShareLens() == true )
            _lens = cam->getLens();
        if( cam->getShareLens() )
            cam->setLens( _lens.get() );
    }
}

CameraGroup::CameraGroup( CameraConfig *cfg )
{
    _initVariables();
    _cfg = cfg;
    //_initLens();
}

CameraGroup::CameraGroup()
{
    _initVariables();
    _cfg = new CameraConfig;
    _cfg->defaultConfig();
    //_initLens();
}

CameraGroup::CameraGroup( const std::string& configFile )
{
    _initVariables();
    _cfg = new CameraConfig;
    if( configFile.empty() )
        _cfg->defaultConfig();
    else
    {
        std::string fullpath = CameraConfig::findFile( configFile );
        _cfg->parseFile( fullpath.c_str() );
    }
    //_initLens();
}

CameraGroup::~CameraGroup()
{
    // first dispatch cancel
    unsigned int i;
    for( i= 0; i < getNumberOfCameras(); i++ )
    {
        if( getCamera(i)->isRunning() )
        {
            //std::cout<<"canclling camera "<<i<<std::endl;
            getCamera(i)->cancel();
            //getCamera(i)->join();
        }
    }
    
    
#if 1    
    //
    // Brut force stoppage.
    //
    bool stopped = false;
    while (!stopped)
    {
        if (_syncBarrier.valid()) _syncBarrier->release();
        if (_frameBarrier.valid()) _frameBarrier->release();

        unsigned int numStopped = 0;
        for(unsigned int i=0; i<getNumberOfCameras(); ++i)
        {
            if(getCamera(i)->isRunning())
            {
                // do nothing
                OpenThreads::Thread::YieldCurrentThread();
            }
            else 
            {
                ++numStopped;
            }
        }
        if(numStopped == getNumberOfCameras())
        {
            stopped = true;
        }
    }
    
#else        

    // then wait for all the threads to stop running.
    Timer timer;
    for( i= 0; i < getNumberOfCameras(); i++ )
    {
        Timer_t start_wait = timer.tick();
        double waitTime = 1.0; // in seconds

        // while the camera is running wait for it to stop naturally.
        // However, under OSX this seems to be wiating for ever, so we'll cut our losses and just wait for
        // a fixed period, then let the CameraGroup destruct, this might cause an app crash
        // due to deleting data while a thread is still running, but at least the app will exit...    
        // Clearly this needs to be fixed in a more robust way, possible thoughts for the cause of this wait
        // for ever on exit is a bug in OSX pthreads.
        // Robert Osfield, September 13th 2003.
        while(getCamera(i)->isRunning() && timer.delta_s(start_wait,timer.tick())<waitTime)
        {
            //if (_syncBarrier.valid()) _syncBarrier->reset();
            //if (_frameBarrier.valid()) _frameBarrier->reset();   

            //std::cout<<"Waiting for camera "<<i<<" to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
        
        if (getCamera(i)->isRunning())
        {
            std::cout<<"Wait time ("<<waitTime<<" seconds) for camera "<<i<<" to cancel has expired, but "<<std::endl
                     <<"camera thread still running, continuing CameraGroup destructor regardless."<<std::endl;
        }
    }
    //std::cout<<"cancelled camera threads"<<std::endl;

#endif
}


void CameraGroup::setStackSize( size_t size )
{
    if( _realized )
    {
        std::cerr << "CameraGroup::setStackSize(): Camera Group is already realized.\n"
                     "SetStackSize may only be called before CameraGroup::realize();\n";
        return ;
    }
    _stack_size = size;
}


bool CameraGroup::realize( ThreadModel threadModel )
{
    if( _realized ) return _realized;
    _threadModel = threadModel;
    _cfg->setThreadModelDirective( _threadModel );
    return realize();
}

bool CameraGroup::realize()
{
    if( _realized ) return _realized;

    _frameStats._frameTimeStampSets.resize(_cfg->getNumberOfCameras());
    _initTime = _timer.tick();

    _threadModel = _cfg->getThreadModelDirective();

    if( _threadModel == SingleThreaded )
    {
        for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
        {
            RenderSurface *rs = _cfg->getCamera(i)->getRenderSurface();
#ifndef WIN32
            rs->useConfigEventThread(false);
#endif
            rs->realize();
            _cfg->getCamera(i)->setBlockOnVsync(_block_on_vsync);
            _cfg->getCamera(i)->setInitTime( _initTime );
        }
        _realized = true;
    }
    else if( _threadModel == ThreadPerCamera )
    {
#ifdef _X11_IMPLEMENTATION
        RenderSurface::initThreads();
#endif
        int nthreads = _cfg->getNumberOfCameras();

        _syncBarrier  = new RefBarrier( nthreads + 1 );
        _frameBarrier = new RefBarrier( nthreads + 1 );

        for( int i = 0; i < nthreads; i++ )
        {
            Camera *cam = _cfg->getCamera(i);
            cam->setSyncBarrier( _syncBarrier.get() );
            cam->setFrameBarrier( _frameBarrier.get() );
            if( _stack_size != 0 )
                cam->setStackSize( _stack_size );
            cam->setInitTime( _initTime );
            cam->setBlockOnVsync(_block_on_vsync);
            cam->startThread();
        }
        _realized = true;
    }
    else
    {
        std::cout << "CameraGroup::realize() : Thread model unsupported\n";
        return _realized ;
    }
    _initLens();
    waitForRealize();
    return _realized;
}

void CameraGroup::setViewByLookat(  float eyex, float eyey, float eyez,
                                    float centerx, float centery, float centerz,
                                    float upx, float upy, float upz )
{
    setViewByLookat( Vec3(eyex, eyey, eyez),
                     Vec3(centerx, centery, centerz ),
                     Vec3(upx, upy, upz) );
}

void CameraGroup::setViewByLookat(  const Vec3& eye,
                                    const Vec3& center,
                                    const Vec3& up )
{
    Matrix m;
    m.makeLookAt(eye,center,up);
    setViewByMatrix( m );
}

void CameraGroup::setViewByMatrix(  const Producer::Matrix &mat )
{
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        Camera *cam = _cfg->getCamera(i);
        if( cam->getShareView() )
            cam->setViewByMatrix( mat );
    }
}

void CameraGroup::setInstrumentationMode( bool mode )
{
    _instrumented = mode;
    for( unsigned int i = 0 ; i < _cfg->getNumberOfCameras(); i++ )
        _cfg->getCamera(i)->setInstrumentationMode(_instrumented);
}


void CameraGroup::frame()
{
    if( _instrumented )
        _frameInstrumented();
    else
        _frame();
}

void CameraGroup::_singleThreadedFrame()
{
    // Keep a unique set of RenderSurfaces.
    // Since there is not necesarily a one-to-one mapping of Cameras to
    // RenderSurfaces (there can be multiple Cameras per RenderSurface), we
    // don't want to swap buffer every Camera's RenderSurface on advance(),
    // because we can potentially swap a buffer more than once per frame.
    std::set<RenderSurface *> u;
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        u.insert( _cfg->getCamera(i)->getRenderSurface() );
        _cfg->getCamera(i)->frame(false);
    }
    
    std::set<RenderSurface *>::iterator p;
    
    /*
    bool _syncronizeSwapBuffers = true;
    if (_syncronizeSwapBuffers && u.size()>1)
    {
        for( p = u.begin(); p != u.end(); p++ )
        {
            (*p)->makeCurrent();
            glFinish();
        }
    }
    */
    
    for( p =  u.begin(); p != u.end(); p++ )
    {
        /*
extern Timer_t B4_SWAP;
B4_SWAP = Producer::Timer::instance()->tick();
*/
        (*p)->swapBuffers();
    }
}

void CameraGroup::_threadPerCameraFrame()
{
    if (_frameBarrier.valid())
        _frameBarrier->block();
    else
        std::cout << "CameraGroup::_frame() :  _frameBarrier not created cannot block\n";
}

void CameraGroup::_frame()
{
    if( !_realized )
    {
        std::cerr << "CameraGroup::frame() : Please call realize() first\n";
        return;
    }
    if( _threadModel == SingleThreaded )
    {
        _singleThreadedFrame();
    }
    else if( _threadModel == ThreadPerCamera )
    {
        if( _sync_count == _frame_count ) 
            sync();

        _threadPerCameraFrame();
    }
    else
    {
        std::cout << "CameraGroup::frame() : Thread model unsupported\n";
    }
    _frame_count++;
}


void CameraGroup::_frameInstrumented()
{
    if( !_realized )
    {
        std::cerr << "CameraGroup::frame() : Please call realize() first\n";
        return;
    }

    _endOfUpdate = _timer.tick();
    bool markStartOfUpdate = false;
    if( _sync_count == _frame_count ) 
    {
        _syncInstrumented(false);
        markStartOfUpdate = true;
    }
    _startOfFrame = _timer.tick(); 

    if( _threadModel == SingleThreaded )
    {
        //_endOfUpdate = _timer.tick();
        //_startOfFrame = _timer.tick(); 
        _singleThreadedFrame();

        if( markStartOfUpdate )
            _startOfUpdate = _timer.tick();
    }
    else if( _threadModel == ThreadPerCamera )
    {
        //_endOfUpdate =  _timer.tick();
        //_startOfFrame = _timer.tick();
        _threadPerCameraFrame();
    }
    else
    {
        std::cout << "CameraGroup::frame() : Thread model unsupported\n";
    }

    _frame_count++;
}

void CameraGroup::sync()
{
    if( _instrumented )
        _syncInstrumented(true);
    else
        _sync();
}

void CameraGroup::_sync()
{
    if( _sync_count > _frame_count ) return;

    if( _threadModel == SingleThreaded )
    {
        ; // ignored
    }
    else if( _threadModel == ThreadPerCamera )
    {
        if (_syncBarrier.valid())
        {
            _syncBarrier->block();
        }
        else
        {
            std::cout << "CameraGroup::_sync() :  _syncBarrier not created cannot block\n";
        }
    }
    else
    {
        std::cout << "CameraGroup::_sync() : Thread model unsupported\n";
    }
    _sync_count = _frame_count + 1;
}

void CameraGroup::_updateStats()
{
    _frameStats._frameNumber = _frame_count;
    _frameStats._startOfUpdate = _timer.delta_s(_initTime, _startOfUpdate );
    _frameStats._endOfUpdate = _timer.delta_s(_initTime, _endOfUpdate );
    _frameStats._startOfFrame = _timer.delta_s( _initTime, _startOfFrame );
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        _frameStats._frameTimeStampSets[i] = _cfg->getCamera(i)->getFrameStats();
        _frameStats._frameTimeStampSets[i].setFrameNumber(_frameStats._frameNumber);
    }

    if( _statsHandler.valid() )
        (*_statsHandler)(*this);
}

void CameraGroup::_syncInstrumented( bool markStartOfUpdate )
{
    if( _sync_count > _frame_count ) return;
    _sync_count = _frame_count + 1;

    if( _threadModel == ThreadPerCamera )
    {
        if (_syncBarrier.valid())
        {
            _syncBarrier->block();
        }
        else
        {
            std::cout << "CameraGroup::_syncInstrumented() :  _syncBarrier not created cannot block\n";
        }
    }

    _updateStats();
    if( markStartOfUpdate )
        _startOfUpdate = _timer.tick();
}


void CameraGroup::advance()
{
    /*
    for( unsigned int i = 0; i < _cfg->getNumberOfRenderSurfaces(); i++ )
    {
        _cfg->getRenderSurface(i)->makeCurrent();
        _cfg->getRenderSurface(i)->swapBuffers();
    }
    */

    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        _cfg->getCamera(i)->advance();
    }
}

Camera *CameraGroup::getCamera(int i) 
{ 
    return _cfg->getCamera(i); 
}

const Camera *CameraGroup::getCamera(int i)  const
{ 
    return _cfg->getCamera(i); 
}

unsigned int CameraGroup::getNumberOfCameras() const
{
    return _cfg->getNumberOfCameras();
}

CameraConfig *CameraGroup::getCameraConfig()
{
    return _cfg.get();
}

const CameraConfig *CameraGroup::getCameraConfig() const
{
    return _cfg.get();
}

void CameraGroup::setSceneHandler(Camera::SceneHandler *sh )
{
    for( unsigned int i = 0; i <  _cfg->getNumberOfCameras(); i++ )
    _cfg->getCamera(i)->setSceneHandler(sh);    
}


#if 0
bool CameraGroup::convertLensToOrtho( float d) 
{ 
    bool retval = true;

    for( unsigned int i = 0; i <  _cfg->getNumberOfCameras(); i++ )
        if( _cfg->getCamera(i)->convertLensToOrtho(d) == false )
        retval = false;

    return retval;
}

bool CameraGroup::convertLensToPerspective( float d )
{
    bool retval = true;

    for( unsigned int i = 0; i <  _cfg->getNumberOfCameras(); i++ )
        if( _cfg->getCamera(i)->convertLensToPerspective(d) == false )
        retval = false;

    return retval;
}
#endif


bool CameraGroup::waitForRealize()
{
    bool retval = true;
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        if( _cfg->getCamera(i)->getRenderSurface()->waitForRealize() == false )
            retval = false;
    }
    return retval;
}

bool CameraGroup::validForRendering() const
{
    bool retval = true;
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
    {
        if( _cfg->getCamera(i)->getRenderSurface()->isRealized() == false )
        {
            retval = false;
            break;
        }
    }
    return retval;
}

void CameraGroup::setBlockOnVsync(bool block )
{
    _block_on_vsync = block;
    for( unsigned int i = 0; i < _cfg->getNumberOfCameras(); i++ )
        _cfg->getCamera(i)->setBlockOnVsync(_block_on_vsync);
}
