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

  echo ###------------------------------------------------------------
  echo ### The configuration script expects the following tools to be
  echo ### installed on your system:
  echo ###
  echo ### makedep (optional)
  echo ### nasm (optional, nasm-0.98E-bin-dos.zip)
  echo ### libdl (optional, for dynamic loader support, dxe2-djgpp-0.1.1.zip)
  echo ### sleep (GNU shell utilites, shl112b.zip)
  echo ### cmp (GNU diff utilites, dif271b.zip)
  echo ### cp (GNU file utilites, fil316b.zip)
  echo ###------------------------------------------------------------
  echo ###      ... press any alphanumeric key to continue ...
  echo.
  pause >nul

  echo int main () {} >conftest.cpp
  echo %%xdefine TEST >conftest.asm

  echo ### Testing whenever your version of GNU C/C++ supports exceptions ...
  gcc -c -fno-exceptions conftest.cpp -o conftest.o
  if not exist conftest.o goto noexceptions

  attrib -r conftest.o
  del conftest.o
  echo $$$ O.K., using the -fno-exceptions switch to get smaller executables
  echo CFLAGS.SYSTEM += -fno-exceptions>>config.tmp

:noexceptions

  echo ### Testing whenever your version of GNU C/C++ supports RTTI ...
  gcc -c -fno-rtti conftest.cpp -o conftest.o
  if not exist conftest.o goto nortti

  del conftest.o
  echo $$$ O.K., using the -fno-rtti switch to get smaller executables
  echo CFLAGS.SYSTEM += -fno-rtti>>config.tmp

:nortti

  echo ### Testing whenever your version of GNU C/C++ supports P5 architecture ...
  gcc -c -mpentium -march=pentium conftest.cpp -o conftest.o
  if not exist conftest.o goto nop5

  del conftest.o
  echo $$$ O.K., using -mpentium -march=pentium switches
  echo CFLAGS.SYSTEM += -mpentium -march=pentium>>config.tmp

:nop5

  echo ### Testing whenever you have (the right version of) NASM installed ...
  nasm -f coff conftest.asm -o conftest.o
  if not exist conftest.o goto nonasm

  attrib -r conftest.o
  del conftest.o
  echo $$$ O.K., setting NASM.INSTALLED to "yes"
  echo NASM.INSTALLED = yes>>config.tmp

:nonasm

  echo ### Checking whenever you have makedep already compiled and installed ...
  makedep conftest.cpp -fconftest.o -c
  if not exist conftest.o goto nomakedep

  attrib -r conftest.o
  del conftest.o
  echo $$$ O.K., setting MAKEDEP.INSTALLED to "yes"
  echo MAKEDEP.INSTALLED = yes>>config.tmp

:nomakedep

  echo ### Checking whenever you have libdl installed (dynamic module loader) ...
  set SH=no
  dxe2gen --help >conftest.o
  if not exist conftest.o goto nodxe2gen

  attrib -r conftest.o
  del conftest.o
  echo $$$ O.K., setting USE_PLUGINS to "yes"
  set SH=yes

    echo ### Checking if you have dynamic version of Zlib ...
    gcc conftest.cpp -o conftest.exe -lz_i
    if not exist conftest.exe goto nozlib_i

    del conftest.exe >nul
    echo $$$ O.K., will use dynamic version of Zlib
    echo ZLIB.SUFFIX = _i>>config.tmp

  :nozlib_i

    echo ### Checking if you have dynamic version of libPNG ...
    gcc conftest.cpp -o conftest.exe -lpng_i
    if not exist conftest.exe goto nopng_i

    del conftest.exe >nul
    echo $$$ O.K., will use dynamic version of libPNG
    echo LIBPNG.SUFFIX = _i>>config.tmp

  :nopng_i

    echo ### Checking if you have dynamic version of libJPEG ...
    gcc conftest.cpp -o conftest.exe -ljpeg_i
    if not exist conftest.exe goto nojpeg_i

    del conftest.exe >nul
    echo $$$ O.K., will use dynamic version of libJPEG
    echo LIBJPEG.SUFFIX = _i>>config.tmp

  :nojpeg_i

:nodxe2gen
  echo USE_PLUGINS = %SH%>>config.tmp
  del conftest.* >nul

  rem Under windoze we do a 2-sec pause, otherwise it will tell us
  rem that "config.mak" has timestamp "in the future". Ha-ha.

  rem Another reason to launch "sleep" is to clear "exit code"
  rem of the batch file. The ugly DOS uses the exit code of last program
  rem run from a batch file as the exit code of the batchfile itself,
  rem thus "exit 0" you see at the end is purely for ahestetical reasons.

  set SH=0
  if /%WINDIR%/ == // set SH=2
  sleep %SH%

  echo All done

  exit 0
