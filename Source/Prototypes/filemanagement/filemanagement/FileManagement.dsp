# Microsoft Developer Studio Project File - Name="FileManagement" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=FileManagement - Win32 Debug 25
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FileManagement.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FileManagement.mak" CFG="FileManagement - Win32 Debug 25"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FileManagement - Win32 Release 25" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FileManagement - Win32 Debug 25" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Controls/FileManagement", NLBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FileManagement - Win32 Release 25"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "FileManagement___Win32_Release_25"
# PROP BASE Intermediate_Dir "FileManagement___Win32_Release_25"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../T[3] Build/Plugins/Toaster/"
# PROP Intermediate_Dir "FileManagement___Win32_Release_25"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /G6 /MD /W3 /GR /GX /Zi /O2 /I "../Additional/Toaster2/Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RPFILEMANAGEMENT_EXPORTS" /YX /FD /G7 /O3 /c
# ADD CPP /nologo /G6 /MD /W3 /GR /GX /Zi /O2 /I "../Additional/Toaster2/Include" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RPFILEMANAGEMENT_EXPORTS" /D "MAKE25BUILD" /D "WIN32" /YX /FD /G7 /O3 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nt_tools.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../T[2] Build/Plugins/Toaster/FileManagement.toast" /libpath:"../../Release" /libpath:"../Additional/Toaster2/Lib/i386" /libpath:"../Additional/RTVLib" /libpath:"../../T[2] Build/Plugins/Toaster/" /libpath:"../../T[2] Build/"
# ADD LINK32 nt_tools.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../T[3] Build/Plugins/Toaster/FileManagement.toast" /libpath:"../../Release25" /libpath:"../../T[3] Build/Plugins/Toaster/" /libpath:"../../T[3] Build/" /libpath:"../Additional/Toaster2/Lib/i386"

!ELSEIF  "$(CFG)" == "FileManagement - Win32 Debug 25"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "FileManagement___Win32_Debug_25"
# PROP BASE Intermediate_Dir "FileManagement___Win32_Debug_25"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Debug25/"
# PROP Intermediate_Dir "FileManagement___Win32_Debug_25"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
F90=df.exe
# ADD BASE CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /ZI /Od /I "../Additional/Toaster2/Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RPFILEMANAGEMENT_EXPORTS" /YX /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GR /GX /ZI /Od /I "../Additional/Toaster2/Include" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "RPFILEMANAGEMENT_EXPORTS" /D "MAKE25BUILD" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 nt_tools.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Debug/FileManagement.toast" /pdbtype:sept /libpath:"../../Debug" /libpath:"../Additional/Toaster2/Lib/i386" /libpath:"../Additional/RTVLib"
# ADD LINK32 nt_tools.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../Debug25/FileManagement.toast" /pdbtype:sept /libpath:"../../Debug25" /libpath:"../Additional/Toaster2/Lib/i386" /libpath:"../Additional/RTVLib"

!ENDIF 

# Begin Target

# Name "FileManagement - Win32 Release 25"
# Name "FileManagement - Win32 Debug 25"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DateChecker.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\DirectoryLayout_ContextMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBin.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBin_DetailsView.cpp
# End Source File
# Begin Source File

SOURCE=.\FileBin_Favorites.cpp
# End Source File
# Begin Source File

SOURCE=.\FileButton.cpp
# End Source File
# Begin Source File

SOURCE=.\FileButton_ContextMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\FileButton_ShellExecute.cpp
# End Source File
# Begin Source File

SOURCE=.\FileButton_ViewMode.cpp
# End Source File
# Begin Source File

SOURCE=.\FileButton_ViewMode_ContextMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\FileClicked_BaseLoadSave.cpp
# End Source File
# Begin Source File

SOURCE=.\FileClicked_ToasterScript.cpp
# End Source File
# Begin Source File

SOURCE=.\FileCollection.cpp
# End Source File
# Begin Source File

SOURCE=.\FileCopyMove.cpp
# End Source File
# Begin Source File

SOURCE=.\FileManagement.rc
# End Source File
# Begin Source File

SOURCE=.\FileProperties.cpp
# End Source File
# Begin Source File

SOURCE=.\FilePropertiesDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FolderClick.cpp
# End Source File
# Begin Source File

SOURCE=.\PathLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\ResolveFile.cpp
# End Source File
# Begin Source File

SOURCE=.\stdafx.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AnimatedIcon_Interface.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryLayout.h
# End Source File
# Begin Source File

SOURCE=.\DirectoryLayout_ContextMenu.h
# End Source File
# Begin Source File

SOURCE=.\FileBin.h
# End Source File
# Begin Source File

SOURCE=.\FileBin_DetailsView.h
# End Source File
# Begin Source File

SOURCE=.\FileBin_Favorites.h
# End Source File
# Begin Source File

SOURCE=.\FileButton.h
# End Source File
# Begin Source File

SOURCE=.\FileButton_ContextMenu.h
# End Source File
# Begin Source File

SOURCE=.\FileButton_PreviewInterface.h
# End Source File
# Begin Source File

SOURCE=.\FileButton_ShellExecute.h
# End Source File
# Begin Source File

SOURCE=.\FileButton_ViewMode.h
# End Source File
# Begin Source File

SOURCE=.\FileButton_ViewMode_ContextMenu.h
# End Source File
# Begin Source File

SOURCE=.\FileClicked_BaseLoadSave.h
# End Source File
# Begin Source File

SOURCE=.\FileClicked_ToasterScript.h
# End Source File
# Begin Source File

SOURCE=.\FileCollection.h
# End Source File
# Begin Source File

SOURCE=.\FileCopyMove.h
# End Source File
# Begin Source File

SOURCE=.\FileDropSaveInterface.h
# End Source File
# Begin Source File

SOURCE=.\FileManagement_Colors.h
# End Source File
# Begin Source File

SOURCE=.\FileProperties.h
# End Source File
# Begin Source File

SOURCE=.\FilePropertiesDlg.h
# End Source File
# Begin Source File

SOURCE=.\FolderClick.h
# End Source File
# Begin Source File

SOURCE=.\PathLayout.h
# End Source File
# Begin Source File

SOURCE=.\ResolveFile.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdafx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\icon2.ico
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Debug\Skins\FileBin\Favorites.txt
# End Source File
# End Target
# End Project
