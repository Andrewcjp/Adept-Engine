@echo off
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" "..\Intermediate\%1.vcxproj" /property:Configuration=%2 /property:Platform=%3
