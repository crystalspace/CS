# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.freebsd = FreeBSD
DESCRIPTION.OS.freebsd = FreeBSD

# Choose which drivers you want to build/use
PLUGINS+=video/canvas/softx video/renderer/software \
  sound/renderer/software sound/driver/oss

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor. Can be one of: INTEL, SPARC, POWERPC, M68K, UNKNOWN
PROC=INTEL

# Operating system. Can be one of:
# NEXT, SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, WIN32, OS2, BE
OS=BSD

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY=UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Extra libraries needed on this system.
LIBS.EXE+=-L/usr/local/lib -lm

# Where can the Zlib library be found on this system?
Z_LIBS=-lz

# Where can the PNG library be found on this system?
PNG_LIBS=-lpng

# Where can the JPG library be found on this system?
JPG_LIBS=-ljpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-I/usr/local/include

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-O6 -fomit-frame-pointer -malign-loops=2 -malign-jumps=2 \
  -malign-functions=2 -ffast-math

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g3 -gstabs

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=-Wl,-shared

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f aoutb -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/unix/unix.cpp libs/cssys/unix/utiming.cpp \
  libs/cssys/unix/loadlib.cpp libs/cssys/general/findlib.cpp \
  libs/cssys/general/printf.cpp libs/cssys/general/getopt.cpp
SRC.SYS_CSSYS_DLL = libs/cssys/unix/dummy.cpp

# The C compiler.
#CC=gcc -c

# The C++ compiler.
#CXX=g++ -c

# The linker.
#LINK=gcc

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make freebsd      Prepare for building on $(DESCRIPTION.freebsd)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure --#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin/unixconf.sh freebsd >>config.tmp

endif # ifeq ($(ROOTCONFIG),config)
