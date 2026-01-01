@echo off
REM ==========================================
REM MarcSLM Debug Build and Installation Script
REM ==========================================
REM This script automates the complete build and installation process
REM for MarcSLM in Debug mode with all dependencies properly deployed

setlocal enabledelayedexpansion

echo.
echo ============================================
echo   MarcSLM Debug Build System
echo ============================================
echo.

REM Get project root directory
set PROJECT_ROOT=%~dp0
cd /d "%PROJECT_ROOT%"

REM Remove trailing backslash if present
if "%PROJECT_ROOT:~-1%"=="\" set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"
