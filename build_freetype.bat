@echo off
cd /D "%~dp0"

set root_path=%cd%
set freetype_path=%root_path%\src\third_party\freetype-2.14.3

set includes=
set includes=%includes% /I%root_path%\src\third_party\freetype_build
set includes=%includes% /I%freetype_path%\include

set files=
set files=%files% %freetype_path%\src\base\ftsystem.c
set files=%files% %freetype_path%\src\base\ftinit.c
set files=%files% %freetype_path%\src\base\ftdebug.c
set files=%files% %freetype_path%\src\base\ftbase.c
set files=%files% %freetype_path%\src\base\ftbitmap.c
set files=%files% %freetype_path%\src\sfnt\sfnt.c
set files=%files% %freetype_path%\src\truetype\truetype.c
set files=%files% %freetype_path%\src\cff\cff.c
set files=%files% %freetype_path%\src\smooth\smooth.c
set files=%files% %freetype_path%\src\psaux\psaux.c
set files=%files% %freetype_path%\src\psnames\psnames.c
set files=%files% %freetype_path%\src\pshinter\pshinter.c

pushd %root_path%\build
del *.obj > NUL 2> NUL
call cl /c /nologo /Zi /DFT2_BUILD_LIBRARY %includes% %files%
call lib /nologo /out:freetype.lib *.obj
popd
