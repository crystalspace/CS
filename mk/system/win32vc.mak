# The system submakefile for building on Win32 using MSVC

# Friendly names for building environment
DESCRIPTION.win32vc = Win32 with MSVC

# Choose which drivers you want to build/use
# cs2d/ddraw6 cs2d/openglwin cs3d/direct3d5 cs3d/direct3d6 cs3d/opengl
#
DRIVERS=cs2d/ddraw \
  cs3d/software \
  csnetdrv/null csnetman/null csnetman/simple \
  cssnddrv/null cssndrdr/null cssndrdr/software

# Uncomment the following to get an startup console window
#CONSOLE_FLAGS = -DWIN32_USECONSOLE

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll .lib .res .rc

# Processor type. Not tested on non-x86
PROC=INTEL

# Operating system
OS=WIN32

# Compiler
COMP=VC

# Command to update a target
UPD=bin/dosupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Save the LIB variable for MSVC linker
MSVCLIB:=$(LIB)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=.dll

# Typical extension for static libraries
LIB=.lib
AR=lib
ARFLAGS=-nologo
ARFLAGS.@=-out:$@

# Typical prefix for library filenames
LIB_PREFIX=

# Where can the Zlib library be found on this system?
Z_LIBS=-libpath:libs/zlib zdll.lib

# Where can the PNG library be found on this system?
PNG_LIBS=-libpath:libs/libpng pngdll.lib

# Where can the JPG library be found on this system?
JPG_LIBS=-libpath:libs/libjpeg jpegdll.lib

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-Ilibs/zlib -Ilibs/libpng -Ilibs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-MD -W3 -nologo -DWINDOWS -DZLIB_DLL $(CONSOLE_FLAGS)

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-Ox -G5

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-Zi

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-Zi

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=-GD

# Flags for compiler to direct output to rule target file
CFLAGS.@=-Fo$@

# Flags for compiler to define a macro
CFLAGS.D=-D

# Flags for compiler to define a include directory
CFLAGS.I=-I

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=-nologo -implib:$(OUT)null.lib \
  -libpath:$(subst ;,-libpath,$(MSVCLIB)) \
  gdi32.lib user32.lib ole32.lib advapi32.lib uuid.lib

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-release

# Flags for the linker which are used when debugging.
LFLAGS.debug=-debug

# Flags for the linker which are used when profiling.
LFLAGS.profile=-debug

# Flags for building a Crystal Space executable
LFLAGS.EXE=-subsystem:windows
# ... a DLL
LFLAGS.DLL=-dll -entry:_DllMainCRTStartup@12 $(OUT)dllentry.obj
# ... a console executable
LFLAGS.CONSOLE.EXE=-subsystem:console
 
# Flags for linker to direct output to rule target file
LFLAGS.@=-out:$(subst /,\,$@)

# Flags for indicating to linker an additional library path
LFLAGS.L=-libpath:

# Flags for indicating an additional library to linker
LFLAGS.l=$(OUT)

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f win32 -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/win32/printf.cpp libs/cssys/win32/timing.cpp \
  libs/cssys/win32/fopen.cpp libs/cssys/win32/dir.cpp \
  libs/cssys/win32/win32.cpp libs/cssys/win32/loadlib.cpp \
  support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler.
CC=cl -c

# The C++ compiler.
CXX=cl -c

# The linker.
DEFFILE=$(subst /,\,$(OUT)$*.def)
define LINK
  @bin\win32link.bat $(DEFFILE) "$(DESCRIPTION.$@)" $(filter %.exe,$@)
  link -def:$(DEFFILE)
endef

# The Resource Compiler. Older versions do not like the "-n" flag
RC=rc
RCFLAGS=-r

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
MKDIR = mkdir $(subst /,\,$(@:/=))

# The command to remove all specified files.
RM=rm -f

# The command to remove a directory tree.
RMDIR=rm -rf

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.o/$$O/g"

# Object file extension
O=.obj

# We don't need separate directories for dynamic libraries
OUTSUFX.yes=

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need a slightly different approach to specify libraries
L^=$(filter %$(LIB),$+)

# How to bind resources to a DLL or executable
DO.BIND.RES = $(RC) $(RCFLAGS) $(subst /,\,$(filter %.res,$^)) $@
ifeq ($(USE_DLL),no)
DO.LINK.CONSOLE.EXE+=$(CR)$(DO.BIND.RES)
DO.LINK.EXE+=$(CR)$(DO.BIND.RES)
endif

# Can someone find a better way to generate dependencies and such?
# Maybe somebody has ported makedepend or makedep or something like it?
DO.DEP = gcc -MM $(filter-out $(CFLAGS.GENERAL) $(CFLAGS.optimize) $(CFLAGS.debug),\
  $(subst -d,-D,$(CFLAGS))) $(CFLAGS.INCLUDE) $^ | sed $(SED_DEPEND) >$(subst /,\,$@)

endif # ifeq ($(MAKESECTION),postdefines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq ($(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)),)
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make win32vc      Prepare for building under and for $(DESCRIPTION.win32vc)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin\win32conf.bat

endif # ifeq ($(ROOTCONFIG),config)

ifeq ($(ROOTCONFIG),volatile)

# Does this OS have native COM support?
NATIVE_COM=yes

endif # ifeq ($(ROOTCONFIG),volatile)
