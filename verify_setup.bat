@echo off
echo ===================================================
echo E-Paper Display System - Verification Script
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

echo Checking PC Server installation...
cd pc_server

echo Checking for required dependencies...
if not exist node_modules (
    echo WARNING: Dependencies not installed.
    echo Run setup.bat first or manually install dependencies.
    pause
    exit /b 1
)

echo Checking for required files...
if not exist server.js (
    echo ERROR: server.js not found.
    pause
    exit /b 1
)

if not exist public\index.html (
    echo ERROR: public\index.html not found.
    pause
    exit /b 1
)

echo Checking for uploads directory...
if not exist uploads (
    echo Creating uploads directory...
    mkdir uploads
)

echo.
echo Checking sample content...
if exist public\sample_image.svg (
    echo Sample image found: public\sample_image.svg
) else (
    echo WARNING: Sample image not found.
)

echo.
echo ===================================================
echo Verification completed!
echo.
echo Your system appears to be correctly set up.
echo.
echo Next steps:
echo 1. Start the server: node server.js
echo 2. Access the web interface: http://localhost:3000
echo 3. Configure and upload the ESP32 firmware
echo 4. Test the display functionality
echo.
echo For detailed instructions, refer to README.md
echo ===================================================
echo.

pause