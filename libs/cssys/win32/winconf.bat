@echo off

  rem *** If you expect to see a sophisticated detection script here,
  rem *** you opened a wrong file :-))) The so-called "batch command
  rem *** language" is so poor that we can only hope the user have the
  rem *** configuration we are expecting.

  echo extern "C" void* opendir(char const*);>conftest.cpp
  echo int main () { opendir(""); return 0; }>>conftest.cpp
  echo %%xdefine TEST>conftest.asm

  echo ###------------------------------------------------------------
  echo ### To successfully compile Crystal Space on Windows32 platform
  echo ### you should have the following utilites/toolsets installed
  echo ### on your system:
  echo ###

  if /%1/ == /mingw32/ goto mingw32
  if /%1/ == /msvc/ goto msvc
  goto nothing

:mingw32
  echo ### - Crystal Space MinGW32 Package 0.90 or later
  goto nothing

:msvc
  echo ### Microsoft Visual C command-line compiler
  echo ### zlib/libpng/libjpeg (msvc_libs0.90.zip)
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

if not /%1/ == /mingw32/ goto nomingwex
  echo CC = gcc -c>>config.tmp
  echo CXX = g++ -c>>config.tmp
  echo LINK = g++>>config.tmp

  rem *** As of gcc 3.x, -fvtable-thunks is deprecated and produces a harmless
  rem *** warning message, however it is still required for pre-gcc 3.x
  rem *** configurations, thus we must add it to CFLAGS.SYSTEM.
  echo # Remove the following line if compiler complains>>config.tmp
  echo # about -fvtable-thunks option.>>config.tmp
  echo CFLAGS.SYSTEM += -fvtable-thunks>>config.tmp

  echo ### Checking if -lmingwex is needed for linking...
  gcc conftest.cpp -lmingwex -o conftest.exe >nul
  if not exist conftest.exe goto nomingwex
  del conftest.exe >nul
  echo $$$ Yes, adding "-lmingwex" to LIBS.SYSTEM
  echo LIBS.SYSTEM += -lmingwex>>config.tmp

:nomingwex
  echo ### Testing whether you have (the right version of) NASM installed ...
  nasm -f win32 conftest.asm -o conftest.o
  if not exist conftest.o goto nonasm

  del conftest.o >nul
  echo $$$ O.K., setting NASM.AVAILABLE to "yes"
  echo NASM.AVAILABLE = yes>>config.tmp

:nonasm

  echo ### Checking whether you have makedep already built and installed ...
  makedep conftest.cpp -fconftest.o -c
  if not exist conftest.o goto nomakedep

  del conftest.o >nul
  echo $$$ O.K., setting MAKEDEP.AVAILABLE to "yes"
  echo MAKEDEP.AVAILABLE = yes>>config.tmp

:nomakedep
  del conftest.* >nul

  rem *** The following is carefully tuned. Seemingly 'nonfunctional'
  rem *** statements are not so. Leave them. They work around weird
  rem *** configurations

  echo ### Checking if you use cmd.exe or some fancy shell...
  echo testing>conftest.1
  make -f libs\cssys\win32\winconf.mak DO_WIN_TEST_ECHO=yes testecho
  cmp conftest.1 conftest.2
  if not errorlevel 1 goto noremovequote
  echo $$$ O.K. Setting to use cmd.exe settings
  type libs\cssys\win32\winconf.var >>config.tmp

:noremovequote
  del conftest.* >nul

  echo ZLIB.AVAILABLE = yes>>config.tmp
  echo ZLIB.LFLAGS = -lz>>config.tmp
  echo PNG.AVAILABLE = yes>>config.tmp
  echo PNG.LFLAGS = -lpng -lz>>config.tmp
  echo JPEG.AVAILABLE = yes>>config.tmp
  echo JPEG.LFLAGS = -ljpeg>>config.tmp
  echo SOCKET.AVAILABLE = yes>>config.tmp
  echo SOCKET.LFLAGS = -lwsock32>>config.tmp

:exit
