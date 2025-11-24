@echo off
echo Compiling project...

g++ src/*.cpp -o gamw.exe ^
 -Iinclude ^
 -IC:\Tools\SDL2main\include ^
 -LC:\Tools\SDL2main\lib ^
 -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf ^
 -Wl,-subsystem,console

echo.
echo ---- Build Selesai ----
pause