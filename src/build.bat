@echo off

IF NOT EXIST ..\run_tree   mkdir ..\run_tree
pushd ..\run_tree

ctime -begin .\logs\text_editor.ctm

del *.pdb > NUL 2> NUL

IF NOT EXIST SDL2.dll   cp ..\src\libs\SDL2\lib\x64\SDL2.dll SDL2.dll
IF NOT EXIST SDL2_ttf.dll   cp ..\src\libs\SDL2_ttf\lib\x64\SDL2_ttf.dll SDL2_ttf.dll
IF NOT EXIST libfreetype-6.dll   cp ..\src\libs\SDL2_ttf\lib\x64\libfreetype-6.dll libfreetype-6.dll
IF NOT EXIST zlib1.dll   cp ..\src\libs\SDL2_ttf\lib\x64\zlib1.dll zlib1.dll

:: shared libs folder path
set SharedLibsPath=D:\_shared_libs

:: -O2 optimization level 2
:: -Od for debbugging, no optimization
::  -WX -W4    I should be using those
:: -nologo   does not show the compiler, architecture (x64) and the output files (exe, libs, ...), ...
set CommonCompilerFlags=-diagnostics:column -diagnostics:caret -Od -WL -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 /EHsc
:: /EHsc because of SDL i guess

:: Additional Include Directories
set AdditionalIncludeDirectories=/I D:\tebtro\iml\src /I..\libs\SDL2\include /I ..\libs\SDL2_ttf\include

:: Additional Linker Options
set AdditionalLinkerFlags=-incremental:no -opt:ref /SUBSYSTEM:CONSOLE /LIBPATH:..\libs\SDL2\lib\x64 SDL2.lib SDL2main.lib SDL2test.lib /LIBPATH:..\libs\SDL2_ttf\lib\x64 SDL2_ttf.lib

:: Libraries
set Libraries=user32.lib gdi32.lib kernel32.lib glu32.lib gdi32.lib  opengl32.lib

:: cpp Files
set CppFiles=..\src\main.cpp ..\src\key_binding.cpp ..\src\base_commands.cpp ..\src\gap_buffer.cpp ..\src\theme.cpp ..\src\window.cpp ..\src\layout.cpp ..\src\editor.cpp ..\src\renderer.cpp


cl /Fe:text_editor %CommonCompilerFlags%  %AdditionalIncludeDirectories%  %CppFiles% /link %AdditionalLinkerFlags% %Libraries%


@echo.
ctime -end .\logs\text_editor.ctm 

popd
