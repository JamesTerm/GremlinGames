@echo off

REM ### Get the project language code. Automatically takes ###
REM ###  care of pathing and adding extension.             ###
set /P code=Enter new language code (i.e. en-US):

REM ### Get the project name. Automatically takes care ###
REM ###  of pathing and adding extension.              ###
set /P project=Enter project name (..\..\*.csproj file):
echo.
echo Creating new satellite DLL for project %project%.csproj and language %code%:
pause

mkdir %code%
LocBaml.exe /generate /trans:%code%.csv /out:%code%\ /cul:%code% en-US\%project%.resources.dll
pause

echo.
echo ALL DONE.
echo A new satellite DLL, %project%.resources.dll, should be in folder %code%.
pause