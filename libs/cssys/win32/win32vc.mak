# The system submakefile for building on Win32 using MSVC

# Friendly names for building environment
DESCRIPTION.win32vc = Win32 with MSVC

# Choose which drivers you want to build/use
# video/canvas/ddraw6 video/canvas/openglwin video/renderer/direct3d5
# video/renderer/direct3d6 video/renderer/opengl
#
PLUGINS+=video/canvas/ddraw video/renderer/software sound/renderer/software \
  video/canvas/ddraw61 video/renderer/direct3d61 video/renderer/opengl

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

include mk/dos.mak

# Save the LIB variable for MSVC linker
MSVCLIB:=$(LIB_SUFFIX)

# Static library command and flags
AR=lib
ARFLAGS=-nologo
ARFLAGS.@=-out:$@

# Where can the Zlib library be found on this system?
Z_LIBS=zdll.lib

# Where can the PNG library be found on this system?
PNG_LIBS=pngdll.lib

# Where can the JPG library be found on this system?
JPG_LIBS=jpegdll.lib

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-MD -W3 -nologo -DWINDOWS -DZLIB_DLL

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
SRC.SYS_CSSYS = libs/cssys/win32/win32.cpp libs/cssys/win32/dir.cpp \
  libs/cssys/general/printf.cpp libs/cssys/win32/timing.cpp \
  libs/cssys/win32/loadlib.cpp libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp
SRC.SYS_CSSYS_EXE=libs/cssys/win32/exeentry.cpp
SRC.SYS_CSSYS_DLL=libs/cssys/win32/dllentry.cpp

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

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.o/$$O/g"

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need a slightly different approach to specify libraries
L^=$(filter %$(LIB_SUFFIX),$+)

# How to bind resources to a DLL or executable
DO.BIND.RES = $(RC) $(RCFLAGS) $(subst /,\,$(filter %.res,$^)) $@
ifeq ($(USE_PLUGINS),no)
DO.LINK.CONSOLE.EXE+=$(CR)$(DO.BIND.RES)
DO.LINK.EXE+=$(CR)$(DO.BIND.RES)
endif

endif # ifeq ($(MAKESECTION),postdefines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq ($(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)),)
"=
|=³
endif

SYSHELP += \
  $(NEWLINE)echo $"  make win32vc      Prepare for building on $(DESCRIPTION.win32vc)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=bin\win32conf.bat msvc

endif # ifeq ($(ROOTCONFIG),config)
