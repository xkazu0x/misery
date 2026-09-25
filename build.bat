@echo off
setlocal enabledelayedexpansion
cd /D "%~dp0"

:: --- Unpack Arguments ----------------------------------------
for %%a in (%*) do set "%%a=1"
if not "%msvc%"=="1" if not "%clang%"=="1" set msvc=1
if not "%release%"=="1" set debug=1
if "%debug%"=="1"   set release=0 && echo [debug mode]
if "%release%"=="1" set debug=0 && echo [release mode]
if "%msvc%"=="1"    set clang=0 && echo [msvc compile]
if "%clang%"=="1"   set msvc=0 && echo [clang compile]

:: --- Unpack Command Line Build Arguments --------------------
set auto_compile_flags=
if "%asan%"=="1"  set auto_compile_flags=%auto_compile_flags% -fsanitize=address && echo [asan enabled]
if "%ubsan%"=="1" set auto_compile_flags=%auto_compile_flags% -fsanitize=undefined && echo [ubsan enabled]

:: --- Compile/Link Definitions --------------------------------
set cl_common=      /I..\src\ /nologo /FC /Z7 /Zc:preprocessor /D_CRT_SECURE_NO_WARNINGS /W4 /wd4061 /wd4062 /wd4100 /wd4191 /wd4200 /wd4201 /wd4310 /wd4668 /wd4702 /wd4820 /wd5045
set cl_debug=       call cl /Od /Ob1 /DBUILD_DEBUG=1 %cl_common% %auto_compile_flags%
set cl_release=     call cl /O2 /DBUILD_DEBUG=0 %cl_common% %auto_compile_flags%
set cl_link=        /link /incremental:no /opt:ref /opt:icf
set cl_out=         /out:
set cl_out_obj=     /Fo:
set cl_linker=

set clang_common=   -I..\src\ -fdiagnostics-absolute-paths -D_CRT_SECURE_NO_WARNINGS -Wall -Wextra -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-initializer-overrides -Wno-missing-braces -Wno-missing-field-initializers -Wno-switch
set clang_debug=    call clang -O0 -g -DBUILD_DEBUG=1 %clang_common% %auto_compile_flags%
set clang_release=  call clang -O2 -g -DBUILD_DEBUG=0 %clang_common% %auto_compile_flags%
set clang_link=     -Xlinker /opt:ref -Xlinker /opt:icf
set clang_out=      -o
set clang_out_obj=  -o
set clang_linker=   -Xlinker

:: --- Per-Build Settings --------------------------------------
set link_icon=logo.res
if "%msvc%"=="1"  set linker=%cl_linker%
if "%clang%"=="1" set linker=%clang_linker%
if "%msvc%"=="1"  set only_compile=/c
if "%clang%"=="1" set only_compile=-c
if "%msvc%"=="1"  set rc=call rc
if "%clang%"=="1" set rc=call llvm-rc
if "%msvc%"=="1"  set link_dll=/link /DLL
if "%clang%"=="1" set link_dll=-Xlinker -DLL

:: --- External Libraries --------------------------------------
set link_freetype=-I..\src\third_party\freetype-2.14.3\include freetype.lib

:: --- Choose Compile/Link -------------------------------------
if "%msvc%"=="1"    set cc_debug=%cl_debug%
if "%msvc%"=="1"    set cc_release=%cl_release%
if "%msvc%"=="1"    set cc_link=%cl_link%
if "%msvc%"=="1"    set out=%cl_out%
if "%msvc%"=="1"    set out_obj=%cl_out_obj%

if "%clang%"=="1"   set cc_debug=%clang_debug%
if "%clang%"=="1"   set cc_release=%clang_release%
if "%clang%"=="1"   set cc_link=%clang_link%
if "%clang%"=="1"   set out=%clang_out%
if "%clang%"=="1"   set out_obj=%clang_out_obj%

if "%debug%"=="1"   set compile=%cc_debug%
if "%release%"=="1" set compile=%cc_release%

:: --- Prep Directories -------------------------------------------------------
if not exist build mkdir build

:: --- Produce Logo Icon File -------------------------------------------------
REM pushd build
REM %rc% /nologo /fo logo.res ..\data\logo.rc || exit /b 1
REM popd

:: --- Build FreeType ---------------------------------------------------------
REM if not exist build\freetype.lib set freetype=1
REM if "%freetype%"=="1" call build_freetype.bat || exit /b 1

:: --- Build ------------------------------------------------------------------
pushd build
%compile% ..\src\scratch_main.c %cc_link% %out%scratch.exe || exit /b 1
if "%run%"=="1" call scratch.exe
popd
