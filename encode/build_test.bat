@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d D:\tmp\encode
del /q main.exe
cl /nologo /W3 /Fe:main.exe main.c chacha20.c
if errorlevel 1 exit /b 1
main.exe

