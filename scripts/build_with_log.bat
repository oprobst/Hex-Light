@echo off
REM Build-Script mit Logging für Windows

set LOGFILE=build.log
echo ===================================== >> %LOGFILE%
echo Build started: %date% %time% >> %LOGFILE%
echo ===================================== >> %LOGFILE%

REM PlatformIO Build ausführen und in Log schreiben
pio run 2>&1 | tee -a %LOGFILE%

echo. >> %LOGFILE%
echo Build finished: %date% %time% >> %LOGFILE%
echo. >> %LOGFILE%
