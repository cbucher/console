@echo off
setlocal
call vc2013 amd64
msbuild Console2.sln /t:Rebuild /p:Configuration="Release Legacy" /p:Platform=x64 /property:BoostIncludePath=C:\Libs\boost\1.58.0
endlocal
