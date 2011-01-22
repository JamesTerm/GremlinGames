#include <stdio.h>
#include <Producer/Timer>
#include <Producer/Utils>
#include <Producer/PipeTimer>

#ifndef GL_TIME_ELAPSED_EXT
#define GL_TIME_ELAPSED_EXT               0x88BF
#endif

#ifndef GL_QUERY_RESULT
#define GL_QUERY_RESULT                   0x8866
#endif

#ifndef GL_QUERY_RESULT_AVAILABLE
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#endif


using namespace Producer;

PipeTimer *PipeTimer::_thePipeTimer = 0L;

class PipeTimerProxy {
    public:
        PipeTimerProxy(){ PipeTimer::_thePipeTimer = new PipeTimer; }
}_pipeTimerProxy;

PipeTimer::PipeTimer():
    _valid(false),
    _glGenQueries(NULL),
    _glDeleteQueries(NULL),
    _glBeginQuery(NULL),
    _glEndQuery(NULL),
    _glGetQueryObjectiv(NULL),
    _glGetQueryObjectui64v(NULL)
{
    setReturnType(seconds);
}

bool PipeTimer::_init()
{
    _glGenQueries =
        (PFNGLGENQUERIESPROC)Producer::getOpenGLProcAddress((GLubyte *)"glGenQueries");

    _glDeleteQueries =
        (PFNGLDELETEQUERIESPROC)Producer::getOpenGLProcAddress((GLubyte *)"glDeleteQueries");

    _glBeginQuery =
        (PFNGLBEGINQUERYPROC)Producer::getOpenGLProcAddress((GLubyte *)"glBeginQuery");

    _glEndQuery =
        (PFNGLENDQUERYPROC) Producer::getOpenGLProcAddress((GLubyte *)"glEndQuery");

    _glGetQueryObjectiv =
        (PFNGLGETQUERYOBJECTIVPROC)Producer::getOpenGLProcAddress((GLubyte *)"glGetQueryObjectiv");

    _glGetQueryObjectui64v =
        (PFNGLGETQUERYOBJECTUI64VEXTPROC)Producer::getOpenGLProcAddress((GLubyte *)"glGetQueryObjectui64v");

    _valid = (  (_glGenQueries != NULL) &&
                (_glDeleteQueries != NULL) &&
                (_glBeginQuery != NULL) &&
                (_glEndQuery != NULL) &&
                (_glGetQueryObjectiv != NULL) &&
                (_glGetQueryObjectui64v != NULL)  );

    return _valid;
}

void PipeTimer::setReturnType(ReturnType r)
{
    _returnType = r;
    switch( _returnType )
    {
        case nanoseconds:  _div = 1.0;    break;
        case microseconds: _div = 1.0e-3; break;
        case milliseconds: _div = 1.0e-6; break;
        case seconds:      _div = 1.0e-9; break;
    }
}

PipeTimer::ReturnType PipeTimer::getReturnType()
{
    return _returnType;
}

GLuint PipeTimer::genQuery()
{
    if( !(_valid || _init()) )
        return 0;

    GLuint query;
    (*_glGenQueries)( 1, &query );
    return query;
}

void PipeTimer::deleteQuery( GLuint *query )
{
    if( _valid )
        (*_glDeleteQueries)( 1, query );
}

void PipeTimer::genQueries(unsigned int n, GLuint *q)
{
    if( (_valid || _init()) )
        (*_glGenQueries)( n, q );
}

void PipeTimer::deleteQueries(unsigned int n, GLuint *queries)
{
    if( _valid )
        (*_glDeleteQueries)( n, queries );
}

void PipeTimer::begin( GLuint query )
{
    if( _valid )
        (*_glBeginQuery)( GL_TIME_ELAPSED_EXT, query );
}

void PipeTimer::end()
{
    if( _valid )
        (*_glEndQuery)( GL_TIME_ELAPSED_EXT );
}


double PipeTimer::getElapsedTime( GLuint query )
{
    if( !_valid )
        return 0.0;

    GLuint64EXT timeElapsed = 0;
    GLint available = 0;
    while( !available )
    {
        (*_glGetQueryObjectiv)( query, GL_QUERY_RESULT_AVAILABLE, &available );
    }

    (*_glGetQueryObjectui64v)(query, GL_QUERY_RESULT, &timeElapsed );

    return double(GLint64EXT(timeElapsed)) * _div;
}

double PipeTimer::getElapsedTime( GLuint query, double  &waitTime )
{
    if( !_valid )
    {
        waitTime = 0.0;
        return 0.0;
    }

    Producer::Timer_t t0 = Producer::Timer::instance()->tick();
    double qt = getElapsedTime( query );
    Producer::Timer_t t1 = Producer::Timer::instance()->tick();

    waitTime =
        _returnType == nanoseconds  ? Producer::Timer::instance()->delta_n( t0, t1 ) :
        _returnType == microseconds ? Producer::Timer::instance()->delta_u( t0, t1 ) :
        _returnType == milliseconds ? Producer::Timer::instance()->delta_m( t0, t1 ) :
        Producer::Timer::instance()->delta_s( t0, t1 );

    return qt;
}
