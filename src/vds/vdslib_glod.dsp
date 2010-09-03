# Microsoft Developer Studio Project File - Name="vdslib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=vdslib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vdslib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vdslib.mak" CFG="vdslib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vdslib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vdslib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vdslib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /vd0 /Zi /O2 /Ob2 /I "..\include" /I "..\..\include" /I "..\xbs\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\Release\vdslib.lib"

!ELSEIF  "$(CFG)" == "vdslib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /Gm /vd0 /GX /ZI /Od /I ".\\" /I "..\glod\include" /I "..\glod\xbs\\" /I "..\include" /I "..\..\include" /I "..\xbs\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i ".." /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\Debug\vdslib.lib"

!ENDIF 

# Begin Target

# Name "vdslib - Win32 Release"
# Name "vdslib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cut.cpp
# End Source File
# Begin Source File

SOURCE=.\forest.cpp
# End Source File
# Begin Source File

SOURCE=.\forestbuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\freelist.cpp
# End Source File
# Begin Source File

SOURCE=.\manager.cpp
# End Source File
# Begin Source File

SOURCE=.\node.cpp
# End Source File
# Begin Source File

SOURCE=.\nodequeue.cpp
# End Source File
# Begin Source File

SOURCE=.\primtypes.cpp
# End Source File
# Begin Source File

SOURCE=.\renderer.cpp
# End Source File
# Begin Source File

SOURCE=.\simplifier.cpp
# End Source File
# Begin Source File

SOURCE=.\threads.cpp
# End Source File
# Begin Source File

SOURCE=.\tri.cpp
# End Source File
# Begin Source File

SOURCE=.\vif.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cut.h
# End Source File
# Begin Source File

SOURCE=.\forest.h
# End Source File
# Begin Source File

SOURCE=.\forestbuilder.h
# End Source File
# Begin Source File

SOURCE=.\freelist.h
# End Source File
# Begin Source File

SOURCE=.\manager.h
# End Source File
# Begin Source File

SOURCE=.\node.h
# End Source File
# Begin Source File

SOURCE=.\nodequeue.h
# End Source File
# Begin Source File

SOURCE=.\primtypes.h
# End Source File
# Begin Source File

SOURCE=.\renderer.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\simplifier.h
# End Source File
# Begin Source File

SOURCE=.\threads.h
# End Source File
# Begin Source File

SOURCE=.\tri.h
# End Source File
# Begin Source File

SOURCE=.\vds.h
# End Source File
# Begin Source File

SOURCE=.\vdsaux.h
# End Source File
# Begin Source File

SOURCE=.\vif.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CutNotes.txt
# End Source File
# Begin Source File

SOURCE=.\ForestNotes.txt
# End Source File
# Begin Source File

SOURCE=.\MemoryManagerNotes.txt
# End Source File
# Begin Source File

SOURCE=.\RendererNotes.txt
# End Source File
# Begin Source File

SOURCE=.\SimplifierNotes.txt
# End Source File
# Begin Source File

SOURCE=".\VIF format.txt"
# End Source File
# End Target
# End Project
