@echo off

mkdir ..\build
pushd ..\build
cl -diagnostics:column -diagnostics:caret /EHsc -FC -Zi /I D:\tebtro\iml\src /I..\src\libs\SDL2\include ..\src\text_editor.cpp user32.lib kernel32.lib glu32.lib gdi32.lib  opengl32.lib /link /SUBSYSTEM:CONSOLE /LIBPATH:..\src\libs\SDL2\lib\x64 SDL2.lib SDL2main.lib SDL2test.lib
popd
