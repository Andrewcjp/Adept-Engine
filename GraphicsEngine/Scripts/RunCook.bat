@echo off
call "..\Binaries\Win64\Release\Game.exe" -Cook %1 
xcopy /S /Q /Y /F "..\readme\Manual.txt" "..\build\win64\ReleasePackage\Manual.txt*"
echo Cook complete
pause