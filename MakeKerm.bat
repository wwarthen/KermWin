@echo off

goto %1
goto exit

:ALL16
for %%t in (WIN16REL WIN16DBG) do call %0 %%t %2 %3 %4 %5 %6 %7 %8 %9
goto exit

:ALL32
for %%t in (WIN32REL WIN32DBG) do call %0 %%t %2 %3 %4 %5 %6 %7 %8 %9
goto exit

:WIN16REL
shift
nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 "Platform=WIN16" "Debug=no"
goto exit

:WIN16DBG
shift
nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 "Platform=WIN16" "Debug=yes"
goto exit

:WIN32REL
shift
nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 "Platform=WIN32" "Debug=no"
goto exit

:WIN32DBG
shift
nmake %1 %2 %3 %4 %5 %6 %7 %8 %9 "Platform=WIN32" "Debug=yes"
goto exit

:CLEAN
shift
rd /s /q Build
rd /s /q Dist
xcopy Source\ProjDirs . /s/v/e
goto exit

:ERROR
echo NO such target: %1
goto exit

:exit
