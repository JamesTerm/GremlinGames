/** \page OSGV_Overview	OSGV Project Overview
\section Vision	Vision

Artists will need to test their art and events in a real game environment.  
They will also need to be able to script actions, impulses, effects (sound and particle effects) 
and events.  Much of the code, particularly the GG_Framework::ActorsActions code, will be reusable 
in the game itself and should be placed in reusable libraries. 

\section Features	Features
-	Provide a background scene to work in with Fluffy File Open Dialogs along with 
		command line input, remembering last options chosen.
-	Provide a script that defines actions, impulses, effects (sound and particle effects) and events
-	Trackball around an object, changing the view-set center to various nodes
-	View frame-rate
-	Use keyboard to fire events (tied to scripted impulses, actions)
-	Common Reusable Functionality  Organize library to facilitate reuse
	-	Sound Effects and 3D positional sound effects stubbed now
	-	Actors / Actions / Impulse / Events
	-	Particle Effects (Smoke and Explosions) stubbed now
	-	All LWO / LWS reader functionality (image transparency to be added)
	-	Anything we add to shaders & texturing (bump-map and specularity)
	-	Lens Flares
	-	Lighting
	-	Glows
	-	User interface (actors parented to a camera with mouse callbacks and dynamically drawn text and shapes)
-  Use these base commands:
	-	h or ? shows a help screen (show the version number and all kb combos)
	-	See the current VSS, its state (solid, wf, point), and lighting (shaded, non-shaded, inherit shaded or non-shaded)
	-	Arrow keys change the view-set center (up parent, dn child, LR, siblings)
	-	esc key exits the proghram (or the help window if open)
	-	b changes background color to blue, grey, black (default blue)
	-	space resets the trackball
	-	d toggle Show the frame number of the current action on the current 
		objects and play/pause state (default on) ???
	-	a toggle display animation time ???
	-	l toggle through lighting options of shaded, non-shaded, inherit from parent (default)
	-	s toggles framerate display ???
	-	t toggle trackball modes ???????  Show the current trackball mode and distance
	-	w toggles through polygon modes (solid(def), wireframe, pointcloud)
	-	., to pause and play forward
	-	0 to reset timer and pause
- Ctrl LMB drag <-> to pause and scrub time

*/

