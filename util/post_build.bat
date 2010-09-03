@ECHO OFF
REM
REM                               Win32 ONLY
REM   ---------------------------------------------------------------------
REM 
REM   Use this script to keep a directory outside-of-GLOD in sync with your
REM   latest GLOD build. 
REM
REM   ---------------------------------------------------------------------
REM   The following configuration options are allowed:
REM
REM   Change BUILD_DLL_DEST to be the place where you want Glod.dll to go
SET       BUILD_DLL_DEST=none
REM
REM   Change BUILD_LIB_DEST to be the place where you want Glod.lib to go
SET       BUILD_LIB_DEST=none
REM
REM   Change BUILD_INC_DEST to the place where you want glod.h
SET       BUILD_INC_DEST=none

REM   ---------------------------------------------------------------------



REM   ---------------------------------------------------------------------
REM                       Do not edit beyond here or else.
REM   ---------------------------------------------------------------------

IF NOT EXIST ..\glodlib.dsw cd ..\..\util\
IF NOT EXIST ..\glodlib.dsw goto noglod_err

if "%1" == "" goto usage_err;

set PREFIX=%1

if "%BUILD_DLL_DEST%" == "none" goto copy_lib;
SET DLL_FILE=..\lib\%PREFIX%\glod.dll
if exist %DLL_FILE%	COPY %DLL_FILE% %BUILD_DLL_DEST%
ECHO Copied libraries to %BUILD_DLL_DEST%.

:copy_lib
if "%BUILD_LIB_DEST%" == "none" goto copy_inc;
SET LIB_FILE=..\lib\%PREFIX%\glod.lib
if exist %LIB_FILE%	COPY %LIB_FILE% %BUILD_LIB_DEST%
ECHO Copied libraries to %BUILD_LIB_DEST%.

:copy_inc
if "%BUILD_INC_DEST%" == "none" goto end;
SET INC_FILE1=..\include\glod.h
if exist %INC_FILE1%	COPY %INC_FILE1% %BUILD_INC_DEST%
ECHO Copied includes to %BUILD_INC_DEST%.
goto end


:usage_err
echo Usage:  post_build Debug
echo Usage:  post_build Release\
goto end

:noglod_err
echo Could not locate your copy of GLOD. Glodlib.dsw must be present in its standard location for this to work.

:end
