@echo off
call "..\Binaries\Release\Game.exe" -Cook %1
xcopy /S /Q /Y /F "..\readme\Manual.txt" "..\build\windows\ShippingReleasePackage\Manual.txt*"
echo Cook complete
pause