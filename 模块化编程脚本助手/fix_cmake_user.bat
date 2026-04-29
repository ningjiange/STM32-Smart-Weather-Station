@echo off
REM STM32 CMake 自动配置脚本 - 批处理启动器
REM 双击此文件运行脚本，自动修复 CMakeLists.txt

setlocal enabledelayedexpansion
cd /d "%~dp0"

color 0A
echo.
echo =========================================
echo  STM32 CMake 自动配置脚本
echo =========================================
echo.

REM 运行 PowerShell 脚本
powershell -NoProfile -ExecutionPolicy Bypass -Command "& '%~dp0fix_cmake_user.ps1'; if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }"

REM 检查脚本执行结果
if %ERRORLEVEL% EQU 0 (
    color 2F
    echo.
    echo =========================================
    echo  脚本执行成功！
    echo =========================================
) else (
    color 4F
    echo.
    echo =========================================
    echo  脚本执行失败！错误代码: %ERRORLEVEL%
    echo =========================================
)

echo.
echo 按任意键关闭此窗口...
pause >nul
exit /b %ERRORLEVEL%
