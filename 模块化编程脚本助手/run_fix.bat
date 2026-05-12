@echo off
echo ========================================
echo  一键修复:CMakeLists + freertos.c + FreeRTOSConfig.h
echo ========================================
echo.

python "%~dp0fix_all.py"

echo.
echo ========================================
echo  Done! You can now build the project.
echo ========================================
pause
