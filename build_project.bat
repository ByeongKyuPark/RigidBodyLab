@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

REM Change directory to the project's root folder
cd /d %~dp0

REM Initialize and update git submodules
echo Initializing and updating git submodules...
git submodule init
git submodule update

REM Navigate to the RigidBodyLab/RigidBodyLab directory
cd RigidBodyLab

REM Create and navigate to the build directory
if not exist build mkdir build
cd build

REM Run CMake to generate the Visual Studio solution
echo Running CMake to generate Visual Studio solution...
cmake .. -G "Visual Studio 17 2022"

echo Build process completed. Check the build directory for output.
pause
