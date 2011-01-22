#ifdef WIN32
#include <windows.h>
#define usleep(x)    Sleep((x)/1000)
#else
#include <unistd.h>
#endif

#if defined( __APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>

using namespace Producer;

class KBMCB : public KeyboardMouseCallback
{
    public:
        KBMCB(): 
            _toggleFullScreen(false),
            _toggleResize(false) ,
            _toggleCursor(false) ,
            _toggleBorder(false),
            _quit(false)
            {}

        void specialKeyPress( KeyCharacter key )
        {
            switch( key )
            {
                case KeyChar_Escape:
                    _quit = true;
                    break;
    
                default:
                    break;
            }
        }

        void keyPress( KeyCharacter key )
        {
            switch( key )
            {
                case 'B':
                case 'b':
                    _toggleBorder = true;
                    break;
            
                case 'C' :
                case 'c':
                    _toggleCursor = true;
                    break;
            
                case 'R' :
                case 'r' :
                    _toggleResize = true;
                    break;
            
                case 'F' :
                case 'f' :
                    _toggleFullScreen = true;
                    break;
            
                case KeyChar_Escape:
                    _quit = true;
                    break;
    
                default:
                    break;
            }
        }
    
        bool toggleFullScreen() { return checkToggle( _toggleFullScreen ); }
        bool toggleResize() { return checkToggle(_toggleResize); }
        bool toggleCursor() { return checkToggle(_toggleCursor); }
        bool toggleBorder() { return checkToggle(_toggleBorder); }
        bool quit() { return _quit; }

    protected:
        virtual ~KBMCB() {}

    private:
        bool _toggleFullScreen;
        bool _toggleResize;
        bool _toggleCursor;
        bool _toggleBorder;
        bool _quit;

        bool checkToggle( bool &toggle )
        {
            if( toggle == true )
            {
                toggle = false;
                return true;
            }
            return false;
        }
};

int main(int /*argc*/, char ** /*argv*/ )
{
    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
    rs->setWindowRectangle( 100, 200, 640, 480 );
    rs->useBorder(false);
    rs->useCursor(false);
    rs->realize();
    
    
    Producer::ref_ptr<KBMCB> kbmcb = new KBMCB;
    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(rs.get());
    kbm->setCallback( kbmcb.get() );
    kbm->startThread();
    
    bool resizeToggle = false;
    bool cursorToggle = true;
    
    glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );
    while( !kbmcb->quit() )
    {
        usleep(25000);
        if( kbmcb->toggleFullScreen() )
        {
            if( rs->isFullScreen() )
                rs->fullScreen(false);
            else
                rs->fullScreen(true);
        }
        
        if( kbmcb->toggleResize() )
        {
            resizeToggle = !resizeToggle;
            if( resizeToggle )
                rs->setWindowRectangle( 300, 300, 100, 100 );
            else
                rs->setWindowRectangle( 100, 200, 640, 480 );
        }
        
        if( kbmcb->toggleCursor() )
        {
            cursorToggle = !cursorToggle;
            rs->useCursor( cursorToggle );
        }
        
        if( kbmcb->toggleBorder() )
        {
            if( rs->usesBorder() )
                rs->useBorder(false);
            else
                rs->useBorder(true);
        }
        
        glClear( GL_COLOR_BUFFER_BIT );
        rs->swapBuffers();
    }
}
