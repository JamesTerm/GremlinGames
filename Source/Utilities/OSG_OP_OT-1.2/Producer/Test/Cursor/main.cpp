#include <iostream>
#include <Producer/RenderSurface>

#ifdef X_H
#include <X11/cursorfont.h>
#endif

#if defined(WIN32) && ! defined(__CYGWIN32__)
#define sleep(x)	Sleep(x*1000)
#endif



enum Action{
    SetDefaultCursor,
    TurnOffCursor,
    TurnOnCursor,
    SetCursorWithCursorField,
    ENDOFLIST
};

struct CursorChanges{
    char *comment;
    Action action;
    Producer::Cursor cursor;
} cursorChanges[] = {
    { "Default Cursor", SetDefaultCursor, 0 },
    { "Turn Off Cursor with useCursor(false)", TurnOffCursor, 0 },
    { "Turn On Cursor with useCursor(true)", TurnOnCursor, 0 },
    { "SetCursor with setCursor(cursor)", SetCursorWithCursorField, 0 },
    { "Turn Off Cursor with setCursor(NULL)", SetCursorWithCursorField, 0 },
    { "", ENDOFLIST, 0 },
};

int main(int /*argc*/, char ** /*argv*/ )
{
    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
    rs->setWindowRectangle( 100, 100, 400, 400 );
    rs->realize();

#ifdef X_H
    cursorChanges[3].cursor = XCreateFontCursor( rs->getDisplay(), XC_coffee_mug );
#endif

#ifdef WIN32
	cursorChanges[3].cursor = LoadCursor( NULL, IDC_WAIT );
#endif
    std::cout << "Note: this is a non-interactive, looping example" << std::endl;
    std::cout << "  with a 5-second pause between settings..." << std::endl;

    for( ;; )
    {
        CursorChanges *p = cursorChanges;
        while( p && p->action != ENDOFLIST )
        {
            std::cout << p->comment << std::endl;
            switch( p->action )
            {
                case SetDefaultCursor:
                    rs->setCursorToDefault();
                    break;

                case TurnOffCursor:
                    rs->useCursor(false);
                    break;

                case TurnOnCursor:
                    rs->useCursor(true);
                    break;

                case SetCursorWithCursorField:
                    rs->setCursor(p->cursor);
                    break;

		case ENDOFLIST:
		    /* to avoid compiler warnings */
		    break;
            }
            rs->swapBuffers();
            sleep(5);
            p++;
        }
    }

    return 0;
}
