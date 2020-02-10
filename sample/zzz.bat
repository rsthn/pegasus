@echo off
bcc32 -w-8022 -w-8027 -w-8060 -w-8057 -I. -I\lib main.cpp
if errorlevel 1 goto :pause
goto :end

:pause
echo.
pause >nul

:end
del *.tds
del *.obj
