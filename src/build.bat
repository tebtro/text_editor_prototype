@echo off

mkdir ..\build
pushd ..\build

cp ..\src\libs\SDL2\lib\x64\SDL2.dll SDL2.dll
cp ..\src\libs\SDL2_ttf\lib\x64\SDL2_ttf.dll SDL2_ttf.dll
cp ..\src\libs\SDL2_ttf\lib\x64\libfreetype-6.dll libfreetype-6.dll
cp ..\src\libs\SDL2_ttf\lib\x64\zlib1.dll zlib1.dll

cl -diagnostics:column -diagnostics:caret /EHsc -FC -Zi /I D:\tebtro\iml\src /I..\src\libs\SDL2\include /I ..\src\libs\SDL2_ttf\include ..\src\text_editor.cpp user32.lib kernel32.lib glu32.lib gdi32.lib  opengl32.lib /link /SUBSYSTEM:CONSOLE /LIBPATH:..\src\libs\SDL2\lib\x64 SDL2.lib SDL2main.lib SDL2test.lib /LIBPATH:..\src\libs\SDL2_ttf\lib\x64 SDL2_ttf.lib
popd
