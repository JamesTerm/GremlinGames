@echo off

echo The 3 files in this archive, Loc1_MakeDLLandExtract.bat,
echo Loc2_AddSatelliteDLL.bat, and LocBaml.exe should be copied
echo to the Debug or Release folder of the app that you are
echo localizing and run from there.
echo.
echo Be sure to first:
echo 1. Add ^<UICulture^>en-US^<^/UICulture^> under ^<PropertyGroup^> in *.csproj
echo        to specify fallback language.
echo 2. Un-comment the NeutralResourcesLanguage assembly setting in AssemblyInfo.cs
echo        to allow the application to work with localization.
echo 3. Add functionality to the app to select or change CurrentUICulture
echo        for supported languages in the CurrentThread.
echo.

REM ### Path to msbuild.exe - usually in .NET framework. ###
set msbuildPath=C:\Windows\Microsoft.NET\Framework\v3.5

REM ### Get the project name. Automatically takes care ###
REM ###  of pathing and adding extension.              ###
set INPUT=
set /P project=Enter project name (..\..\*.csproj file):
echo.
echo Adding Uids to csproj file and verifying...
pause

%msbuildPath%\msbuild.exe /t:updateuid ..\..\%project%.csproj
pause
%msbuildPath%\msbuild.exe /t:checkuid ..\..\%project%.csproj
echo.
echo Creating DLL for en-US...
pause

%msbuildPath%\msbuild.exe ..\..\%project%.csproj
echo.
echo Extracting values into en-US.csv file...
pause

LocBaml.exe /parse en-US\%project%.resources.dll /out:en-US.csv
echo.
echo ALL DONE...
echo Copy en-US.csv to xx-YY.csv, translate pertinent values, and run LocGenerate.bat to create satellite DLL.
pause