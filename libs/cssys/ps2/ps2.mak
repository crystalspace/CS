# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.ps2 = PS2

# Choose which 2D/3D driver combinations you want to build/use
#PLUGINS+=video/renderer/software
#PLUGINS+=video/renderer/opengl
PLUGINS+=video/renderer/opengl video/canvas/ps2d

# uncomment the following to build sound drivers
#PLUGINS+=sound/renderer/software

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor is autodetected and written to config.mak
PROC=EE

# Operating system. Can be one of: SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, AMIGAOS, WIN32, OS2, BE
OS=PS2

# Compiler. Can be one of: GCC, WCC (Watcom C++), MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

override USE_PLUGINS = no

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Does this system require libsocket.a?
NEED_SOCKET_LIB=no

# Extra libraries needed on this system.
DEP.EXE += /code/peed/ps2/libGL.a
LIBS.EXE+=-nostartfiles -L/usr/local/sce/ee/lib -L/code/peed/ps2 -T/usr/local/sce/ee/lib/app.cmd crt0.o -L.. -lpng -lz -ljpeg -lm -lgraph -ldev -ldma -lpkt -lpeed -lSDL -lGL -lcdvd -lvu0 -lpad -lmc -lmtap
LIBS.SORT+=-lpeed -lSDL -lGL -lvu0 -lgraph -ldma -lcdvd -lpad -lmc -lmtap

# Where can the Zlib library be found on this system?
Z_LIBS=

# Where can the PNG library be found on this system?
PNG_LIBS=

# Where can the JPG library be found on this system?
JPG_LIBS=

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-I/usr/local/sce/ee/include -I/code/peed/src

# General flags for the compiler which are used in any case.
# -fno-exceptions and -fno-rtti have effect only for gcc >= 2.8.x
CFLAGS.GENERAL=-Wall -Wunused -W $(CFLAGS.SYSTEM) -fno-rtti -fno-exceptions

# Flags for the compiler which are used when optimizing.
ifeq ($(PROC),INTEL)
  CFLAGS.optimize=-O6 -fomit-frame-pointer -ffast-math
else
  CFLAGS.optimize=-O -fomit-frame-pointer
endif

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

CFLAGS.memdebug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=NOT SUPPORTED

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g3

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
#LFLAGS.DLL=-Wl,-shared -nostdlib -lgcc
LFLAGS.DLL=NOT SUPPORTED

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f elf

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = \
  libs/cssys/ps2/ps2dummy.cpp \
  libs/cssys/ps2/ps2.cpp \
  libs/cssys/ps2/ptiming.cpp \
  libs/cssys/ps2/loadlib.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/instpath.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/getopt.cpp

# The C compiler.
CC=ee-g++ -c

# The C++ compiler.
CXX=ee-g++ -c

# The linker.
LINK=ee-g++

EXE=.elf

endif # ifeq ($(MAKESECTION),defines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make ps2          Prepare for building on $(DESCRIPTION.ps2)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=echo PROC=ee >>config.tmp; echo OS=PS2 >>config.tmp; echo COMP=GCC >>config.tmp; echo CS_LITTLE_ENDIAN=1 >>config.tmp; echo override USE_PLUGINS=$(USE_PLUGINS) >>config.tmp 

endif # ifeq ($(ROOTCONFIG),config)
