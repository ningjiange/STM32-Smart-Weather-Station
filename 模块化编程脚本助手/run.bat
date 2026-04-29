@echo off
REM STM32 CMake 配置脚本 - 简化版本
REM 这是最稳定的双击运行方式

cd /d "%~dp0"

REM 调用 PowerShell 脚本
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "& '%~dp0fix_cmake_user.ps1'; Write-Host ''; Read-Host '按 Enter 关闭窗口'"

exit /b 0
