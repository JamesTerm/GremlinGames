#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/XmAll.h>
#include <GL/GLwMDrawA.h>


#include "gui.h"

void Gui::init(int *argc, char **argv, GinitCallback *ginit )
{
    Arg args[64];
    int n;

    gicb = ginit;

    n = 0;
    XtSetArg( args[n], XmNallowShellResize, True );  n++;
    root = XtAppInitialize( &app_context, "MotifTest", NULL, 0, argc, argv, NULL, args, n );

    n = 0;
    mainWindow = XmCreateMainWindow( root, "mainWindow", args, n );
    XtManageChild( mainWindow );

    n = 0;
    menuBar = XmCreateMenuBar( mainWindow, "menuBar", args, n );
    XtManageChild( menuBar );

    n = 0;
    fileMenu = XmCreatePulldownMenu( menuBar, "fileMenu", args, n );

    n = 0;
    quitButton = XmCreatePushButtonGadget( fileMenu, "quitButton", args, n );
    XtManageChild( quitButton );
    XtAddCallback( quitButton, XmNactivateCallback, s_quitCB, this );

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, fileMenu );  n++;
    fileCascade = XmCreateCascadeButtonGadget( menuBar, "fileCascade", args, n );
    XtManageChild( fileCascade );

    n = 0;
    editMenu = XmCreatePulldownMenu( menuBar, "editMenu", args, n );

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, editMenu );  n++;
    editCascade = XmCreateCascadeButtonGadget( menuBar, "editCascade", args, n );
    XtManageChild( editCascade );

    n = 0;
    utilsMenu = XmCreatePulldownMenu( menuBar, "utilsMenu", args, n );

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, utilsMenu );  n++;
    utilsCascade = XmCreateCascadeButtonGadget( menuBar, "utilsCascade", args, n );
    XtManageChild( utilsCascade );

    n = 0;
    helpMenu = XmCreatePulldownMenu( menuBar, "helpMenu", args, n );

    n = 0;
    XtSetArg( args[n], XmNsubMenuId, helpMenu );  n++;
    helpCascade = XmCreateCascadeButtonGadget( menuBar, "helpCascade", args, n );
    XtManageChild( helpCascade );

    n = 0;
    XtSetArg( args[n], XmNmenuHelpWidget, helpCascade );  n++;
    XtSetValues( menuBar, args, n );

    n = 0;
    mainForm = XmCreateForm( mainWindow, "mainForm", args, n );
    XtManageChild( mainForm );

    if( _mode == Embedded )
    {
        n = 0;
        frame = XmCreateFrame( mainForm, "frame", args, n );
        XtManageChild( frame );
    
        n = 0;
        XtSetArg( args[n], XmNwidth, 700 );  n++;
        XtSetArg( args[n], XmNheight, 700 );  n++;
        XtSetArg( args[n], XmNallowResize, TRUE );  n++;
    
        if( _vc != NULL )
        {
    	XVisualInfo *visinfo = _vc->choose( XtDisplay(root), 0 );
        	XtSetArg( args[n], GLwNvisualInfo, visinfo); n++;
        }
        else
        {
            XtSetArg( args[n], GLwNrgba, TRUE ); n++;
            XtSetArg( args[n], GLwNdoublebuffer, TRUE ); n++;
            XtSetArg( args[n], GLwNdepthSize, 24 ); n++;
        }
        XtSetArg( args[n], XmNtraversalOn, TRUE ); n++;
        glw = GLwCreateMDrawingArea( frame, "glw", args, n );
        XtAddCallback(glw, GLwNexposeCallback, s_exposeCB, this);
        XtAddCallback(glw, GLwNresizeCallback, s_resizeCB, this);
        XtAddCallback(glw, GLwNginitCallback, s_ginitCB, this);
        XtAddCallback(glw, GLwNinputCallback, s_inputCB, this);
        XtManageChild( glw );
    }

}

void Gui::mainLoop( IdleCallback *cb )
{
    if( cb != NULL )
    {
	icb = cb;
	XtAppAddWorkProc( app_context, (XtWorkProc)s_idle, this );
    }

    XtRealizeWidget( root );
    XtAppMainLoop( app_context );
}

void Gui::s_quitCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    if(This != NULL )
	    This->quitCB( w, call_data );
}

void Gui::quitCB( Widget w, XtPointer call_data )
{
    XtAppSetExitFlag( app_context );
}


void Gui::s_exposeCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    if( This != NULL )
	This->exposeCB(w,call_data);
}

void Gui::s_resizeCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    if( This != NULL )
	This->resizeCB(w,call_data);
}

void Gui::s_ginitCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    if( This != NULL )
	This->ginitCB(w,call_data);
}

void Gui::s_inputCB( Widget w, XtPointer client_data, XtPointer call_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    if( This != NULL )
	This->inputCB(w,call_data);
}

void Gui::exposeCB( Widget w, XtPointer call_data)
{
    Arg args[2];
    short ww, wh;

    XtSetArg( args[0], XmNwidth, &ww );
    XtSetArg( args[1], XmNheight, &wh );
    XtGetValues( w, args, 2 );

    glwWidth  = ww;
    glwHeight = wh;

}

void Gui::resizeCB( Widget w, XtPointer call_data)
{
    Arg args[2];
    short ww, wh;

    XtSetArg( args[0], XmNwidth, &ww );
    XtSetArg( args[1], XmNheight, &wh );
    XtGetValues( w, args, 2 );

    glwWidth  = ww;
    glwHeight = wh;
}

void Gui::ginitCB( Widget w, XtPointer call_data)
{
    /*
    Arg args[1];
    XVisualInfo *vi;

    XtSetArg(args[0], GLwNvisualInfo, &vi );
    XtGetValues( w, args, 1 );
    glx_context = glXCreateContext( XtDisplay(w), vi, 0, GL_FALSE );
    */
    if( gicb != NULL )
	(*gicb)( XtWindow(w) );
}


void Gui::inputCB( Widget w, XtPointer call_data)
{
    GLwDrawingAreaCallbackStruct *cdata = (GLwDrawingAreaCallbackStruct *)call_data;
    XEvent *ev = cdata->event;

    switch( ev->type )
    {
	case MotionNotify :
                _mx = 2.0 * (((float)ev->xmotion.x/(float)glwWidth)-0.5);
                _my = 2.0 * (((float)(glwHeight-ev->xmotion.y)/(float)glwHeight)-0.5);
		break;

	case ButtonPress :
                _mx = 2.0 * (((float)ev->xbutton.x/(float)glwWidth)-0.5);
                _my = 2.0 * (((float)(glwHeight-ev->xbutton.y)/(float)glwHeight)-0.5);
		_button_state |= (1 << (ev->xbutton.button - 1)); 
		break;

	case ButtonRelease :
                _mx = 2.0 * (((float)ev->xbutton.x/(float)glwWidth)-0.5);
                _my = 2.0 * (((float)(glwHeight-ev->xbutton.y)/(float)glwHeight)-0.5);
		_button_state &= ~(1 << (ev->xbutton.button - 1)); 
		break;
    }

}


bool Gui::s_idle( XtPointer client_data )
{
    Gui *This = static_cast<Gui *>(client_data);
    This->idle();
    return false;
}

void Gui::idle()
{

	/*
    //GLwDrawingAreaMakeCurrent( glw, glx_context );
    if( icb != NULL )
	(*icb)();
    //GLwDrawingAreaSwapBuffers( glw );
cout << "THIS BETTER NOT PRINT\n";
*/
}

