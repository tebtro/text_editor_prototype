@echo off


call "D:\text_editor\misc\shell.bat"

cmd /k "%ConEmuDir%\..\init.bat" -new_console:d:D:\text_editor\src  -new_console:t:text_editor

