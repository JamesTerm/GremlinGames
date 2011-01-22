#include <stdio.h>
#include <Producer/Utils>

#if defined( __linux )
#include <dlfcn.h>
#endif

namespace Producer {

void (*getProcAddress(const GLubyte *procName))(void)
{
#if defined ( __linux )
    static void *handle = dlopen( (const char *)0L, RTLD_LAZY );
    return (void (*)(void))dlsym(handle,(const char *)procName);
#elif defined( WIN32 )
    return (void (*)(void))wglGetProcAddress( (LPCSTR)procName );
#else
    return 0L;
#endif
}

void (*getOpenGLProcAddress(const GLubyte *procName))(void)
{
    void (*symbol)(void) = getProcAddress( procName);

    // If not the base name, try an ARB extension
    if( symbol == NULL )
    {
        char arbName[128];
        sprintf(arbName, "%sARB", procName );
        symbol = getProcAddress( (const GLubyte *)arbName );
    }
    // If not an ARB extension  name, try an EXT extension
    if( symbol == NULL )
    {
        char extName[128];
        sprintf(extName, "%sEXT", procName );
        symbol = getProcAddress( (const GLubyte *)extName);
    }

    return symbol;
}

}

