#include <unistd.h>
#include <Producer/RenderSurface>
#ifdef __MINGW32__
#include <stdlib.h>
#define sleep(t)  _sleep((t)*1000)
#endif

int main()
{
    Producer::RenderSurface *rs = new Producer::RenderSurface;

    rs->setWindowRectangle( 100, 100, 400, 400 );
    rs->setWindowName( "Window -1" );

    rs->realize();
    int count = 0;
    for( ;; )
    {
        char buff[25];
        sleep(1);
        sprintf( buff, "Window %d", count++ );
        rs->setWindowName( buff );
    }
}
