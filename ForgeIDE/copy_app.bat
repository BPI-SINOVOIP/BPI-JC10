@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM 配置：源工程根目录
REM ============================================================
set "SRC=."


REM ============================================================
REM 修改ServoControl_App.ftproj
REM ============================================================
echo ==== modify ServoControl_App.ftproj ====
set "FTSRC=%SRC%\ServoControl.ftproj"
set "FTDST=%SRC%\ServoControl_App.ftproj"

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0filter_ftproj.ps1" ^
  -srcProj "%FTSRC%" ^
  -dstProj "%FTDST%" 
  

if %errorlevel% neq 0 (
  echo [ERROR] Filter OPEN ftproj failed
  pause
  exit /b 1
)

echo ==== Done ====
exit /b 0
