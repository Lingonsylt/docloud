@echo off
:10
regsvr32 /s /u x64\debug\doCloudExt.dll
taskkill /F /im explorer.exe
msbuild doCloudExt.vcxproj /property:PlatformToolset=v110;Configuration=Debug;Platform=X64
regsvr32 /s x64\debug\doCloudExt.dll
explorer .
pause
goto 10
