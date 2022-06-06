@REM Used as a pre-build step to format the source code.
@REM Requires the clang-format executable.
@REM The configuration may be found in the file .clang-format
@REM at the root level of all projects.

@ECHO off

SET PROJECT_DIR=%1
@REM Set the correct location for the beautifier.
SET CLANG_FORMAT=%PROJECT_DIR%\..\clang-format.exe
SET FILE_TYPES=*.h,*.c,*.cpp,*.tpp

SETLOCAL ENABLEDELAYEDEXPANSION

@REM Pass through each directory of interest, building a string of file names.
@REM NOTE: This will fail at some point when we hit the maximum character length of a variable.

SET SOURCE_DIR=include
CD /d %PROJECT_DIR%\%SOURCE_DIR%
SET fileList=
FOR %%i IN (%FILE_TYPES%) DO SET fileList=!fileList! %SOURCE_DIR%\%%i

CD /d %PROJECT_DIR%

@REM Now we can beautifier the world!
@REM ECHO %CLANG_FORMAT% -i -style=file %fileList% > command.txt
%CLANG_FORMAT% -i -style=file %fileList%

SET SOURCE_DIR=include\EmbeddedSerialFiller
CD /d %PROJECT_DIR%\%SOURCE_DIR%
SET fileList=
FOR %%i IN (%FILE_TYPES%) DO SET fileList=!fileList! %SOURCE_DIR%\%%i

CD /d %PROJECT_DIR%

@REM Now we can beautifier the world!
@REM ECHO %CLANG_FORMAT% -i -style=file %fileList% > command.txt
%CLANG_FORMAT% -i -style=file %fileList%

SET SOURCE_DIR=src
CD /d %PROJECT_DIR%\%SOURCE_DIR%
SET fileList=
FOR %%i IN (%FILE_TYPES%) DO SET fileList=!fileList! %SOURCE_DIR%\%%i

CD /d %PROJECT_DIR%

@REM Now we can beautifier the world!
@REM ECHO %CLANG_FORMAT% -i -style=file %fileList% > command.txt
%CLANG_FORMAT% -i -style=file %fileList%

@REM Cleanup any residual temporary files.
DEL /S *.tmp

ENDLOCAL

@ECHO on
