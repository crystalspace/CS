@echo off
  rem For eyecatching we use the following convention:
  rem   Informational messages begins with '###'
  rem   Error messages begins with '***'
  rem   Notification messages begins with '$$$'

  echo ### Sniffing your build environment ...
  if not .%DJGPP%. == .. goto ok

  echo.
  echo *** It looks like you don't have a valid DJGPP installation
  echo *** since the DJGPP environment variable is missing.
  echo.
  echo *** Please install DJGPP properly before configuring.
  echo *** Exiting ...
  exit 1

:ok
  echo int main () {} >conftest.cpp
  echo %%xdefine TEST >conftest.asm

  echo ### Testing whenever your version of GNU C/C++ supports exceptions ...
  gcc -c -fno-exceptions conftest.cpp -o conftest.o
  if not exist conftest.o goto noexceptions

  del conftest.o
  echo $$$ O.K., using the -fno-exceptions switch to get smaller executables
  echo CFLAGS.SYSTEM += -fno-exceptions >>config.mak

:noexceptions

  echo ### Testing whenever you have (the right version of) NASM installed ...
  nasm -f coff conftest.asm -o conftest.o
  if not exist conftest.o goto nonasm

  del conftest.o
  echo $$$ O.K., setting USE_NASM to "yes"
  echo USE_NASM = yes >>config.mak

:nonasm

  del conftest.*

  exit 0
