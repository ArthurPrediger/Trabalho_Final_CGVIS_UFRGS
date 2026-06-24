@echo off
echo ========================================
echo Cleaning old build folder...
echo ========================================

if exist build (
    rmdir /s /q build
    echo Old build folder removed.
) else (
    echo No previous build folder found.
)

echo.
echo ========================================
echo Configuring CMake with Visual Studio 2022 x64...
echo ========================================

cmake -B build\vs2022 -S . -G "Visual Studio 17 2022" -A x64

echo.
echo ========================================
echo Done!
echo You can now open: build\SlotcarGame.sln
echo ========================================

pause