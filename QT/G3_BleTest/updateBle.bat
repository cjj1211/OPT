@echo off
echo ===== FOTA Upgrade =====

set /p BLE_NAME=Enter BLE device name: 
set /p APP_FILE=Enter App firmware file name: 
set /p NET_FILE=Enter Net firmware file name: 

python "%~dp0\fota_upgrade.py" "%BLE_NAME%" --app "%~dp0\%APP_FILE%" --net "%~dp0\%NET_FILE%"

pause
