# This is the makefile for Borland compiler (bcc32 for Win32)

# Friendly names for building environment
DESCRIPTION.borland = Windows with Borland
DESCRIPTION.OS.borland = Win32

# Choose which drivers you want to build/use
# video/canvas/ddraw6 video/canvas/openglwin video/renderer/direct3d5 
# video/renderer/direct3d6 video/renderer/opengl
#
PLUGINS+=video/canvas/ddraw cssndrdr/software \
  video/renderer/opengl video/canvas/openglwin video/renderer/direct3d5

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll

# Processor type.
PROC=INTEL

# Operating system
OS=WIN32

# Compiler
COMP=BCC32

# Command to update a target
#UPD=bin/dosupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
LIBS.EXE=dxextra.lib

# Socket library
LIBS.SOCKET.SYSTEM=$(LFLAGS.l)wsock32

# Where can the Zlib library be found on this system?
Z_LIBS=zlib.lib

# Where can the PNG library be found on this system?
PNG_LIBS=png.lib

# Where can the JPG library be found on this system?
JPG_LIBS=libjpeg.lib

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Flags for C compiler to direct output to the rule target
CFLAGS.@ = -o$@

# Flags for linker to direct output to the rule target
LFLAGS.@ = -e$@

# Indicate where special include files can be found.
CFLAGS.INCLUDE=$(CFLAGS.I)include/cssys/win32 $(CFLAGS.I)libs/csterr

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-w-8027 $(CFLAGS.D)WIN32_VOLATILE -q -x- $(CFLAGS.SYSTEM)

586=-5 -OS
686=-6 -OS
ULTRAOPTIMIZE=-Oi -Ov
ifdef USE_CODEGUARD
CODEGUARD=-vG
endif

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-d -ff -O2

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-Od -v -vi- -y $(CODEGUARD)

# Flags for the compiler which are used when profiling.
CFLAGS.profile=

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=-q

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=

# Flags for the linker which are used when debugging.
LFLAGS.debug=-Od -v -vi- -y $(CODEGUARD)

# Flags for the linker which are used when profiling.
LFLAGS.profile=

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=-WD

# Typical extension for objects and static libraries
LIB=.lib
define AR
  @rm -f $@
endef
#For debug mode
ARFLAGS=/P128
ARFLAGS.@=$(subst /,\,$@)

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/win32/win32.cpp libs/cssys/win32/dir.cpp \
  libs/cssys/general/printf.cpp libs/cssys/win32/timing.cpp \
  libs/cssys/win32/loadlib.cpp libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp
SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp

# The C compiler
CC=bcc32 -c $(CFLAGS.D)OS_WIN32

# The C++ compiler
CXX=bcc32 -c $(CFLAGS.D)OS_WIN32

# The linker.
LINK=bcc32

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
  MKDIR = mkdir $(subst /,\,$(@:/=))
else
  MKDIR = mkdir $@
endif

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
LFLAGS.EXE=-W
LFLAGS.CONSOLE.EXE=-WC

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

L^=$+
DO.COMPILE.C = $(CC) $(CFLAGS) $(CFLAGS.INCLUDE) $(CFLAGS.@) $(<<)
DO.COMPILE.CPP = $(CXX) $(CFLAGS) $(CFLAGS.INCLUDE) $(CFLAGS.@) $(<<)
DO.SHARED.LIBRARY = $(LINK) $(LFLAGS.DLL) $(LFLAGS) $(LFLAGS.@) $(^^) $(L^) $(LIBS)
DO.SHARED.PLUGIN = $(LINK) $(LFLAGS.DLL) $(LFLAGS) $(LFLAGS.@) $(^^) $(L^) $(LIBS)

define DO.STATIC.LIBRARY
	$(AR)
	$(foreach curobj,$(^^),tlib $(ARFLAGS) "$(ARFLAGS.@)" "+$(subst /,\,$(curobj))";)
endef

endif # ifeq ($(MAKESECTION),postdefines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make borland      Prepare for building on $(DESCRIPTION.borland)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

#SYSCONFIG=bin/win32conf.bat

endif # ifeq ($(ROOTCONFIG),config)
