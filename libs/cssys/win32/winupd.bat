@echo off

  rem *** This batch file should verify if contents of two files differ.
  rem *** If they do, we copy file given as first argument into second.
  rem *** If they don't, we do nothing
  rem *** In any case we remove first file

  cmp -s %1 %2
  if errorlevel 1 goto docopy
  goto doremove

:docopy
  echo Updating %2
  cp -f %1 %2

:doremove
  del %1 >nul

  if /%WINDIR%/ == // goto nowindoze
  sleep 2

:nowindoze
