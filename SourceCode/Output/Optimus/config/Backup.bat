@echo off
cd /d "%~dp0" 
cd ..
cd bin
set EXE_PATH=%CD%\NxBackupService.exe
start "" %EXE_PATH%
exit