# This is an include file for all the makefiles which describes system specific
# settings. Also have a look at mk/user.mak.

# Friendly names for building environment
DESCRIPTION.linux = Linux
DESCRIPTION.OS.linux = Linux

# Choose which 2D/3D driver combinations you want to build/use
PLUGINS+=video/canvas/softx
PLUGINS+=video/renderer/opengl video/canvas/openglx
PLUGINS+=video/canvas/linex

# video support
# formats (this is the wrapping format for the video data)
# Microsofts AVI
PLUGINS+=video/format/avi
# CODECS (some formats are dynamic, that is they need codecs to encod/decode
# data) OpenDivX : you need an additional library you can get from
# www.projectmayo.com
#PLUGINS+=video/format/codecs/opendivx
PLUGINS+=video/format/codecs/rle

# Uncomment the following to build GGI 2D and/or SDL drivers
#PLUGINS+=video/canvas/ggi 
#PLUGINS+=video/canvas/sdl 

# Uncomment the following to build sound drivers
PLUGINS+=sound/driver/oss sound/renderer/software

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

# Processor is autodetected and written to config.mak
#PROC=

# Operating system. Can be one of:
# NEXT, SOLARIS, LINUX, IRIX, BSD, UNIX, DOS, MACOS, WIN32, OS2, BE
OS=LINUX

# Operating system family: UNIX (for Unix or Unix-like platforms), WIN32, etc.
OS_FAMILY=UNIX

# Compiler. Can be one of: GCC, MPWERKS, VC (Visual C++), UNKNOWN
COMP=GCC

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/unix.mak

# Extra libraries needed on this system.
LIBS.EXE+=$(LFLAGS.l)dl $(LFLAGS.l)m  

# Socket library
LIBS.SOCKET.SYSTEM=

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.l)jpeg

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=

# General flags for the compiler which are used in any case.
# -fno-exceptions and -fno-rtti have effect only for gcc >= 2.8.x
CFLAGS.GENERAL=-Wall -Wunused -W $(CFLAGS.SYSTEM)

# Flags for the compiler which are used when optimizing.
ifeq ($(PROC),X86)
  CFLAGS.optimize=-O2 -fomit-frame-pointer -ffast-math
else
  CFLAGS.optimize=-O -fomit-frame-pointer
endif

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

LFLAGS.optimize=-s

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for the linker which are used when building a shared library.
#LFLAGS.DLL=-Wl,-shared -nostdlib $(LFLAGS.l)gcc
LFLAGS.DLL=-Wl,-shared

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f elf

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = \
  libs/cssys/unix/unix.cpp \
  libs/cssys/unix/utiming.cpp \
  libs/cssys/unix/loadlib.cpp \
  libs/cssys/unix/instpath.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/getopt.cpp

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
  $(NEWLINE)echo $"  make linux        Prepare for building on $(DESCRIPTION.linux)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),rootdefines) # Makefile includes us twice with valid
ifeq ($(ROOTCONFIG),config)	  # ROOTCONFIG, but we only need to run once.

SYSCONFIG += $(NEWLINE)sh bin/unixconf.sh linux $(INSTALL_DIR)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

MAKE_VOLATILE_H += \
  $(NEWLINE)echo $"\#define CS_UNIX_PLUGIN_REQUIRES_MAIN$">>volatile.tmp

endif # ifeq ($(ROOTCONFIG),volatile)
endif # ifeq ($(MAKESECTION),rootdefines)
