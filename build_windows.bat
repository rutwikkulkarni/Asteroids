@echo off
if not exist build mkdir build
pushd build
cl /Zi /Fe:asteroids_win32.exe ../win32_main.cpp /link user32.lib opengl32.lib gdi32.lib ole32.lib winmm.lib
popd build