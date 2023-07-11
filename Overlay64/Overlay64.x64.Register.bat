@echo off
echo This script should be run with administrator privileges.
echo Right click - run as administrator.
echo Press any key if you're running it as administrator.
echo. 
pause

regsvr32 /s "%~dp0Overlay64.x64.dll"

if %errorlevel% neq 0 goto ERR
echo. 
echo SUCCESS
echo. 
echo You must restart your computer to apply these changes
echo (or kill+restart process explorer.exe).
echo. 
pause
exit

:ERR
echo. 
echo FAIL
echo. 
pause
exit /b %errorlevel%
