# Microsoft Developer Studio Project File - Name="glodlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=glodlib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "glodlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "glodlib.mak" CFG="glodlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "glodlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "glodlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "glodlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLODLIB_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\vds\\" /I "..\mt" /I "..\xbs" /I ".\\" /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLODLIB_EXPORTS" /D "GLOD" /FD /c /Tp
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib mt.lib ply.lib xbs.lib vdslib.lib ply.lib /nologo /dll /machine:I386 /out:"..\..\lib\Release\glod.dll" /implib:"..\..\lib\Release\glod.lib" /libpath:"..\..\lib\Release"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Post build step...
PostBuild_Cmds=COPY ..\..\lib\Release\glod.dll ..\..\bin\ > NUL	..\..\util\post_build.bat Release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "glodlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GLODLIB_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "..\..\include" /I "..\include" /I "..\vds" /I "..\mt" /I "..\xbs" /I ".\\" /D "_WINDOWS" /D "_USRDLL" /D "GLODLIB_EXPORTS" /D "GLOD" /D "WIN32" /D "_DEBUG" /D "_MBCS" /FD /GZ /c /Tp
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 opengl32.lib mt.lib ply.lib xbs.lib vdslib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /dll /debug /machine:I386 /out:"..\..\lib\Debug\glod.dll" /implib:"..\..\lib\Debug\glod.lib" /pdbtype:sept /libpath:"..\..\lib\Debug" /libpath:"..\vds\Debug"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying DLL to bin
PostBuild_Cmds=COPY ..\..\lib\Debug\glod.dll ..\..\bin\ > NUL	..\..\util\post_build.bat Debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "glodlib - Win32 Release"
# Name "glodlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\glod_core.cpp
# End Source File
# Begin Source File

SOURCE=.\glod_glext.cpp
# End Source File
# Begin Source File

SOURCE=.\glod_group.cpp
# End Source File
# Begin Source File

SOURCE=.\glod_noop_funcs.cpp
# End Source File
# Begin Source File

SOURCE=.\glod_objects.cpp
# End Source File
# Begin Source File

SOURCE=.\GroupParams.cpp
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\ObjectParams.cpp
# End Source File
# Begin Source File

SOURCE=.\Raw.cpp
# End Source File
# Begin Source File

SOURCE=.\RawConvert.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\glod.h
# End Source File
# Begin Source File

SOURCE=..\include\glod_core.h
# End Source File
# Begin Source File

SOURCE=..\include\glod_glext.h
# End Source File
# Begin Source File

SOURCE=..\include\glod_group.h
# End Source File
# Begin Source File

SOURCE=..\include\glod_raw.h
# End Source File
# Begin Source File

SOURCE=..\include\hash.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ply.h
# End Source File
# Begin Source File

SOURCE=..\include\vds_callbacks.h
# End Source File
# End Group
# End Target
# End Project
