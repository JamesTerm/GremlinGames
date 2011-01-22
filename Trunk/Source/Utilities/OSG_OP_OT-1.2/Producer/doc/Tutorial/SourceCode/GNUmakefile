
DIRS = \
	RenderSurface\
	Camera\
	CameraGroup\
	KeyboardMouse\
	Motif\


all :: 
	@for f in $(DIRS); do cd $$f; $(MAKE) $@ || exit 1; cd ..; done

clean clobber :
	@for f in $(DIRS); do cd $$f; $(MAKE) $@ || exit 1; cd ..; done
