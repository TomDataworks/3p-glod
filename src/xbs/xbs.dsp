# Microsoft Developer Studio Project File - Name="xbs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=xbs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "xbs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xbs.mak" CFG="xbs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xbs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "xbs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "xbs - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\vds\\" /I "..\mt" /I "..\xbs" /I ".\\" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "GLOD" /FD /TP /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\Release\xbs.lib"

!ELSEIF  "$(CFG)" == "xbs - Win32 Debug"

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
# ADD CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "..\vds\\" /I "..\mt" /I "..\..\include" /I "..\include" /I "..\vdslib" /I ".\\" /I "..\xbs" /D "_LIB" /D "GLOD" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FR /YX /FD /GZ /TP /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\Debug\xbs.lib"

!ENDIF 

# Begin Target

# Name "xbs - Win32 Release"
# Name "xbs - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Continuous.C
# End Source File
# Begin Source File

SOURCE=.\Discrete.C
# End Source File
# Begin Source File

SOURCE=.\DiscretePatch.C
# End Source File
# Begin Source File

SOURCE=.\Heap.C
# End Source File
# Begin Source File

SOURCE=.\Hierarchy.C
# End Source File
# Begin Source File

SOURCE=.\Metric.C
# End Source File
# Begin Source File

SOURCE=.\MLBPriorityQueue.C
# End Source File
# Begin Source File

SOURCE=.\Model.C
# End Source File
# Begin Source File

SOURCE=.\ModelShare.C
# End Source File
# Begin Source File

SOURCE=.\Operation.C
# End Source File
# Begin Source File

SOURCE=.\PermissionGrid.C
# End Source File
# Begin Source File

SOURCE=.\SimpQueue.C
# End Source File
# Begin Source File

SOURCE=.\vds_callbacks.cpp
# End Source File
# Begin Source File

SOURCE=.\View.C
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Continuous.h
# End Source File
# Begin Source File

SOURCE=.\Discrete.h
# End Source File
# Begin Source File

SOURCE=.\DiscretePatch.h
# End Source File
# Begin Source File

SOURCE=.\Heap.h
# End Source File
# Begin Source File

SOURCE=.\Hierarchy.h
# End Source File
# Begin Source File

SOURCE=.\Metric.h
# End Source File
# Begin Source File

SOURCE=.\MLBPriorityQueue.h
# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\PermissionGrid.h
# End Source File
# Begin Source File

SOURCE=.\Point.h
# End Source File
# Begin Source File

SOURCE=.\Sample.h
# End Source File
# Begin Source File

SOURCE=.\view.h
# End Source File
# Begin Source File

SOURCE=.\xbs.h
# End Source File
# End Group
# End Target
# End Project
