@echo off

  rem *** If you expect to see a sophisticated detection script here,
  rem *** you opened a wrong file :-))) The so-called "batch command
  rem *** language" is so poor that we can only hope the user have the
  rem *** configuration we are expecting.

  echo ###------------------------------------------------------------
  echo ### To successfully compile Crystal Space on Windows32 platform
  echo ### you should have the following utilites/toolsets installed
  echo ### on your system:
  echo ###

  if /%1/ == /mingw32/ goto mingw32
  if /%1/ == /msvc/ goto msvc
  goto nothing

:mingw32
  echo ### - Crystal Space MinGW32 Package 0.4 or later
  goto nothing

:msvc
  echo ### Microsoft Visual C command-line compiler
  echo ### zlib/libpng/libjpeg (win32libs.zip)
  goto nothing

:nothing
  echo ###
  echo ### - makedep (optional)
  echo ###
  echo ### - nasm (optional, nasm-0.98E-bin-w32.zip)
  echo ###
  echo ###------------------------------------------------------------
  echo ###      ... press any alphanumeric key to continue ...
  echo.
  pause >nul

  echo int main () {} >conftest.cpp
  echo %%xdefine TEST >conftest.asm

  echo ### Testing whenever you have (the right version of) NASM installed ...
  nasm -f win32 conftest.asm -o conftest.o
  if not exist conftest.o goto nonasm

  del conftest.o >nul
  echo $$$ O.K., setting NASM.INSTALLED to "yes"
  echo NASM.INSTALLED = yes>>config.tmp

:nonasm

  echo ### Checking whenever you have makedep already compiled and installed ...
  makedep conftest.cpp -fconftest.o -c
  if not exist conftest.o goto nomakedep

  del conftest.o >nul
  echo $$$ O.K., setting MAKEDEP.INSTALLED to "yes"
  echo MAKEDEP.INSTALLED = yes>>config.tmp

:nomakedep
  del conftest.* >nul

  echo ### Checking if you use cmd.exe or some fancy shell...
  echo $"testing$">conftest.1
  make -f bin\win32conf.mak testecho
  cmp conftest.1 conftest.2
  if not errorlevel 1 goto noremovequote
  echo $$$ O.K. Setting to use cmd.exe settings
  type bin\win32conf.var >>config.tmp

:noremovequote
  del conftest.* >nul

:exit
