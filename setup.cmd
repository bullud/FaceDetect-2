@echo off
set PROJECT_ROOT=%~dp0
cd /d "%PROJECT_ROOT%"

if defined VS100COMNTOOLS (
set "VSTOOLS=%VS100COMNTOOLS%"
set VSVER=100
) else (
set "VSTOOLS=%VS120COMNTOOLS%"
set VSVER=120
)

call "%VSTOOLS%..\..\vc\vcvarsall.bat" x86