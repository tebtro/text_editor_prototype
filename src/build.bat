@echo off

IF NOT EXIST ..\run_tree   mkdir ..\run_tree
pushd ..\run_tree

ctime -begin .\logs\text_editor.ctm

del *.pdb > NUL 2> NUL

cp ..\src\libs\SDL2\lib\x64\SDL2.dll SDL2.dll
cp ..\src\libs\SDL2_ttf\lib\x64\SDL2_ttf.dll SDL2_ttf.dll
cp ..\src\libs\SDL2_ttf\lib\x64\libfreetype-6.dll libfreetype-6.dll
cp ..\src\libs\SDL2_ttf\lib\x64\zlib1.dll zlib1.dll

:: -O2 optimization level 2
:: -Od for debbugging, no optimization
set CommonCompilerFlags=-FC -Zi /EHsc -Od -diagnostics:column -diagnostics:caret

:: Additional Include Directories
set AdditionalIncludeDirectories=/I D:\tebtro\iml\src /I..\src\libs\SDL2\include /I ..\src\libs\SDL2_ttf\include

:: Additional Linker Options
set AdditionalLinkerFlags=-incremental:no /SUBSYSTEM:CONSOLE /LIBPATH:..\src\libs\SDL2\lib\x64 SDL2.lib SDL2main.lib SDL2test.lib /LIBPATH:..\src\libs\SDL2_ttf\lib\x64 SDL2_ttf.lib

:: Libraries
set Libraries=user32.lib gdi32.lib kernel32.lib glu32.lib gdi32.lib  opengl32.lib

:: cpp Files
set CppFiles=..\src\main.cpp


cl /Fe:text_editor %CommonCompilerFlags%  %AdditionalIncludeDirectories%  %CppFiles% /link %AdditionalLinkerFlags% %Libraries%


@echo.
ctime -end .\logs\text_editor.ctm 

popd
