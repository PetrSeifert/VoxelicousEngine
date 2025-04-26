@echo off
setlocal enabledelayedexpansion

:: Set the base path
set BASE_DIR=%~dp0
set BASE_DIR=%BASE_DIR:~0,-1%

:: Define build types
set BUILD_TYPES=Debug Release Dist

:: Process command line arguments
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug
set FOUND=0

:: Validate build type
for %%i in (%BUILD_TYPES%) do (
    if /i "%BUILD_TYPE%"=="%%i" set FOUND=1
)

if %FOUND%==0 (
    echo Invalid build type: %BUILD_TYPE%
    echo Available build types: %BUILD_TYPES%
    exit /b 1
)

:: Create build directory if it doesn't exist
set BUILD_DIR=%BASE_DIR%\build-%BUILD_TYPE%
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

:: Configure and build
cd "%BUILD_DIR%" || exit /b 1
echo Configuring %BUILD_TYPE% build...
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% .. || exit /b 1

echo Building %BUILD_TYPE%...
cmake --build . || exit /b 1

echo %BUILD_TYPE% build completed successfully.
echo Output files are in %BUILD_DIR%\bin\%BUILD_TYPE%
exit /b 0 