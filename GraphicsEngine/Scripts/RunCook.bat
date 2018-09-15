@echo off
call BuildSLN.bat Release
call "..\Binaries\Release\Core.exe" -Cook
echo Cook complete
pause