@echo off
set EXE_PATH=%CD%\Backup.bat
echo %EXE_PATH%
set TASK_NAME=NxBackup
set START_TIME=00:00  
set DAYS_INTERVAL=7  
set START_IN=%CD%\
echo Creating scheduled task...

schtasks /create /tn "%TASK_NAME%" /tr "\"%EXE_PATH%\"" /sc daily /st %START_TIME% /mo %DAYS_INTERVAL% 

echo Scheduled task created successfully.

pause
