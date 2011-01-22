#ifndef GUI_H
#define GUI_H
#include <X11/Intrinsic.h>
#include <Producer/VisualChooser>

class Gui {
    public :

	class IdleCallback {
	    public:
		IdleCallback() {}
		virtual void operator()() = 0;
	};

	class GinitCallback {
	    public:
		GinitCallback() {}
		virtual void operator()(Window) = 0;
	};

	enum Mode {
	    Embedded,
	    Detached
 	};

	Gui()
	{
	    icb = NULL;
	    _vc = NULL;
	    _mx = _my = 0.0;
	    _button_state = 0;
	    _mode = Embedded;
	}
	void setMode( Mode mode ) { _mode = mode; }
	void init(int *argc, char **argv, GinitCallback *ginitCB =0 );
	void mainLoop(IdleCallback *cb=NULL);

	void setVisualChooser( Producer::VisualChooser *vc )
	{
	    _vc = vc;
	}

	Window getRenderWindow() 
	{ 
	    if( !XtIsWidget(glw) ) return 0L;
	    return XtWindow(glw);
	}

	static void s_quitCB( Widget, XtPointer, XtPointer );
	static void s_exposeCB(Widget, XtPointer, XtPointer );
	static void s_resizeCB(Widget, XtPointer, XtPointer );
	static void s_ginitCB(Widget, XtPointer, XtPointer );
	static void s_inputCB(Widget, XtPointer, XtPointer );
	static bool s_idle( XtPointer );

	void quitCB( Widget, XtPointer );
	void exposeCB( Widget, XtPointer );
	void resizeCB( Widget, XtPointer );
	void ginitCB( Widget, XtPointer );
	void inputCB( Widget, XtPointer );
	void idle();


	float mx() { return _mx; }
	float my() { return _my; }
	unsigned int buttonState() { return _button_state; }


    private : 

        XtAppContext  app_context;
	Widget root,
		mainWindow,
		  menuBar,
		    fileMenu,
        	      newButton,
        	      openButton,
        	      closeButton,
        	      quitButton,
        	    fileCascade,
        	    editMenu,
        	    editCascade,
        	    utilsMenu,
                    utilsCascade,
                    helpMenu,
        	    helpCascade,
        	    mainForm,
		    frame,
		    glw;

	unsigned int glwWidth, glwHeight;
	IdleCallback *icb;
	GinitCallback *gicb;
	Producer::VisualChooser *_vc;

	float _mx, _my;
	unsigned int _button_state;
	Mode _mode;

};

#endif
