set EXE="C:\Program Files\WinRAR\WinRAR.exe"

call %EXE% a -afzip -ep1 -r GitDeps.zip "Debug" "Release"
pause