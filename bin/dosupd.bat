@echo off

  rem *** This batch file should verify if contents of two files differ.
  rem *** If they do, we copy file given as first argument into second.
  rem *** If they don't, we do nothing
  rem *** In any case we remove first file

  if not exist %2 goto docopy
  cmp -s %1 %2
  if errorlevel 1 goto docopy
  goto doremove

:docopy
  cp -f %1 %2

:doremove
  rm -f %1

  if /%WINDIR%/ == // goto nowindoze
  sleep 2

:nowindoze
