@echo off
echo Building the CMake project...
cmake --build .

echo Running the executable...
start Voortman3DRender.exe

echo Done.
pause