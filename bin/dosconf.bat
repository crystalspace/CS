@echo off
  rem For eyecatching we use the following convention:
  rem   Informational messages begins with '###'
  rem   Error messages begins with '***'
  rem   Notification messages begins with '$$$'

  echo ### Sniffing your build environment ...
  if not .%DJGPP%. == .. goto djgppok

  echo.
  echo *** It looks like you don't have a valid DJGPP installation
  echo *** since the DJGPP environment variable is missing.
  echo.
  echo *** Please install DJGPP properly before configuring.
  echo *** Exiting ...
  exit 1

:djgppok
  echo int main () {} >conftest.cpp
  echo %%xdefine TEST >conftest.asm

  echo ### Testing whenever your version of GNU C/C++ supports exceptions ...
  gcc -c -fno-exceptions conftest.cpp -o conftest.o
  if not exist conftest.o goto noexceptions

  del conftest.o >nul
  echo $$$ O.K., using the -fno-exceptions switch to get smaller executables
  echo CFLAGS.SYSTEM += -fno-exceptions>>config.tmp

:noexceptions

  echo ### Testing whenever your version of GNU C/C++ supports RTTI ...
  gcc -c -fno-rtti conftest.cpp -o conftest.o
  if not exist conftest.o goto nortti

  del conftest.o >nul
  echo $$$ O.K., using the -fno-rtti switch to get smaller executables
  echo CFLAGS.SYSTEM += -fno-rtti>>config.tmp

:nortti

  echo ### Testing whenever your version of GNU C/C++ supports P5 architecture ...
  gcc -c -mpentium -march=pentium conftest.cpp -o conftest.o
  if not exist conftest.o goto nop5

  del conftest.o >nul
  echo $$$ O.K., using -mpentium -march=pentium switches
  echo CFLAGS.SYSTEM += -mpentium -march=pentium>>config.tmp

:nop5

  echo ### Testing whenever you have (the right version of) NASM installed ...
  nasm -f coff conftest.asm -o conftest.o
  if not exist conftest.o goto nonasm

  del conftest.o >nul
  echo $$$ O.K., setting USE_NASM to "yes"
  echo USE_NASM = yes>>config.tmp

:nonasm

  del conftest.* >nul

  if /%WINDIR%/ == // goto nowindoze
  sleep 2

:nowindoze

  exit 0
