@echo off
setlocal
:PROMPT
SET /P AREYOUSURE=Are you sure (Y/[N])?
IF /I "%AREYOUSURE%" NEQ "Y" GOTO END

REM // SET PATHS //
set launcher="C:\Program Files (x86)\Epic Games\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"

REM // DELETE ALL FILES //
rmdir .idea /s /q
rmdir .vs /s /q
rmdir Saved /s /q
rmdir Intermediate /s /q
rmdir Binaries /s /q
del *.sln

REM // FIND PROJECT FILE AND GENERATE PROJECT FILE
for %%f in (*.uproject) do %launcher% /projectfiles %cd%\%%f

:END
endlocal