@echo off
echo ===================================================
echo E-Paper Display System - Setup Script
echo ===================================================
echo.

echo Checking for Node.js installation...
where node >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Node.js is not installed or not in PATH.
    echo Please install Node.js from https://nodejs.org/
    echo and run this script again.
    pause
    exit /b 1
)

echo Node.js found. Version:
node --version
echo.

echo Setting up PC Server...
cd pc_server

echo Installing dependencies...
echo This may take a moment...
call npm install
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to install dependencies.
    pause
    exit /b 1
)

echo Creating uploads directory if it doesn't exist...
if not exist uploads mkdir uploads

echo.
echo ===================================================
echo Setup completed successfully!
echo.
echo To start the server:
echo   cd pc_server
echo   node server.js
echo.
echo Then access the web interface at:
echo   http://localhost:3000
echo.
echo For ESP32 setup instructions, please refer to:
echo   QUICK_START.md
echo ===================================================
echo.

pause