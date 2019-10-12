@echo off
call BuildSLN.bat Release %1
call BuildSLN.bat ShippingReleasePackage %1
call "..\Binaries\Release\Core.exe" -Cook %1
echo Cook complete
pause