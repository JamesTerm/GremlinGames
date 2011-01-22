
Debug_TARGETS = \
		..\..\debug_bin\osgdb_lwod.dll \
		..\..\debug_bin\osgdb_lwod.pdb \
		..\..\debug_bin\osgdb_lwsd.dll \
		..\..\debug_bin\osgdb_lwsd.pdb 
		
Release_TARGETS = \
		..\..\release_bin\osgdb_lwo.dll \
		..\..\release_bin\osgdb_lwo.pdb \
		..\..\release_bin\osgdb_lws.dll \
		..\..\release_bin\osgdb_lws.pdb 

rebuild: rebuild-Debug rebuild-Release
clean: clean-Debug clean-Release
build: build-Debug build-Release

clean-Debug: 
	-erase $(Debug_TARGETS)
clean-Release: 
	-erase $(Release_TARGETS)

rebuild-Debug: clean-Debug build-Debug
rebuild-Release: clean-Release build-Release

build-Debug: $(Debug_TARGETS)
build-Release: $(Release_TARGETS)

..\..\debug_bin\osgdb_lwod.dll: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwod.dll
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwod.dll $@
	
..\..\debug_bin\osgdb_lwod.pdb: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwod.pdb
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwod.pdb $@
	
..\..\debug_bin\osgdb_lwsd.dll: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwsd.dll
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwsd.dll $@
	
..\..\debug_bin\osgdb_lwsd.pdb: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwsd.pdb
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwsd.pdb $@
	
..\..\release_bin\osgdb_lwo.dll: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwo.dll
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwo.dll $@
	
..\..\release_bin\osgdb_lwo.pdb: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwo.pdb
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lwo.pdb $@
	
..\..\release_bin\osgdb_lws.dll: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lws.dll
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lws.dll $@
	
..\..\release_bin\osgdb_lws.pdb: ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lws.pdb
	copy ..\Utilities\OSG_OP_OT-1.2\OpenSceneGraph\bin\win32\osgdb_lws.pdb $@