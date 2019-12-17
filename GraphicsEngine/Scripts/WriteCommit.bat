@echo off
set Build=""
set Ver=0
SET /p Build=< ../Source/Core/GitCommitVersion.h
rem echo %Build%

for /f "delims=" %%x in ('git rev-list --count HEAD') do set Ver=%%x
echo Git Commit is %Ver%

set Def=#define GITCOMMIT %Ver%
rem echo %Def%
if /I "%Def%" NEQ "%Build%" (
	@echo %Def% >../Source/Core/GitCommitVersion.h
	)