#------------------------------------------------------------------------------#
#                        This is the makefile for OS2/EMX
#                       *** (tested only with pgcc-1.0) ***
#------------------------------------------------------------------------------#

#---------------------------------------------------
# NOTE: OS/2 makefiles for libjpeg, libpng and zlib
# can be found in system/os2 subdirectory
#---------------------------------------------------

# Friendly names for building environment
DESCRIPTION.os2wcc = OS/2 with Watcom C

# Choose which drivers you want to build/use
DRIVERS=cs2d/csdive cs3d/software csnetdrv/null csnetman/null csnetman/simple \
  cssnddrv/null cssndrdr/null cssndrdr/software

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll .lib .res .rc

# CMD.EXE is faster than BASH, but crashes on very long (>1024 chars)
# command lines. You SHOULD replace this by a Unix-like shell (say bash)
# when generating dependencies! Note that you can do it from the command
# line as well, i.e.: make depend SHELL=bash
# The best results can be achieved with ash which is a very small Unix-like
# shell, which can be found for instance on hobbes.nmsu.edu; it works faster
# than bash and better than cmd.exe
#SHELL=cmd.exe
#SHELL=bash.exe
SHELL=sh.exe

# Processor type. Most OS/2 systems runs on Intel's, I'm not sure IBM
# will ever ressurect the PowerPC version ... :-(
PROC=INTEL

# Operating system
OS=OS2

# Compiler
COMP=WCC

################################################################## temporary ###
DO_ASM=no

# The command to update target
UPD=cmd /c bin\\os2upd.cmd $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

# Typical extension for executables on this system (e.g. EXE=.exe)
EXE=.exe

# Typical extension for dynamic libraries on this system.
DLL=.dll

# Typical extension for static libraries
LIB=.lib
AR=wlib
ARFLAGS=-b -c -fo -n

# Typical prefix for library filenames
LIB_PREFIX=

# Extra libraries needed on this system.
ifeq ($(USE_SHARED_PLUGINS),no)
  LIBS.EXE+=LIBP $(OUTOS) L csos2
endif

# Where can the Zlib library be found on this system?
Z_LIBS=LIBP libs/zlib L zdll

# Where can the PNG library be found on this system?
PNG_LIBS=LIBP libs/libpng L pngdll

# Where can the JPG library be found on this system?
JPG_LIBS=LIBP libs/libjpeg L jpegdll

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
CFLAGS.INCLUDE=-i=libs/zlib -i=libs/libpng -i=libs/libjpeg

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-bm -w3 -5s -fp5 -zc -zld -zq

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-obmikl -s

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-d3

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-d3

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=-bd

# Flags for compiler to direct output to rule target file
CFLAGS.@=-fo=$(subst /,\,$@)

# Flags for compiler to define a macro
CFLAGS.D=-d

# Flags for compiler to define a include directory
CFLAGS.I=-i=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=LIBP %WATCOM%\lib386 LIBP %WATCOM%\lib386\os2 \
  OPT dosseg OPT quiet OPT caseexact

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=

# Flags for the linker which are used when debugging.
LFLAGS.debug=DEBUG Watcom all

# Flags for the linker which are used when profiling.
LFLAGS.profile=DEBUG Watcom all

# Flags for building a Crystal Space executable
LFLAGS.EXE=FORM os2 lx pm
# ... a DLL
LFLAGS.DLL=FORM os2 lx dll initi termi \
  EXPORT DllInitialize,DllCanUnloadNow,DllGetClassObject,\
  DllRegisterServer,DllUnregisterServer
# ... a console executable
LFLAGS.CONSOLE.EXE=FORM os2 lx pmcomp
 
# Flags for linker to direct output to rule target file
LFLAGS.@=NAME $(subst /,\,$@)

# Flags for indicating to linker an additional library path
LFLAGS.L=LIBP$(SPACE)

# Flags for indicating an additional library to linker
LFLAGS.l=L$(SPACE)

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f obj -DEXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = libs/cssys/general/printf.cpp libs/cssys/general/timing.cpp \
  libs/cssys/os2/csos2.cpp libs/cssys/os2/loadlib.cpp \
  libs/cssys/os2/scancode.cpp support/gnu/getopt.c support/gnu/getopt1.c
SRC.SYS_CSSYS_DLL=libs/cssys/os2/dllentry.cpp

# Where to put the dynamic libraries on this system?
OUTDLL=

# The C compiler.
CC=wcc386

# The C++ compiler.
CXX=wpp386

# The linker.
LINKERFILE=_____tmp.lnk
define LINK
  @rem $(shell del $(LINKERFILE))\
       $(foreach objfile,$(subst /,\,$(filter-out %.res,$(filter-out %$(LIB_SUFFIX),$^))),$(shell echo FILE $(objfile)>>$(LINKERFILE)))
  wlink
endef

# The Resource Compiler. Older versions do not like the "-n" flag
RC=rc
RCFLAGS=-n -I .

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

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Our own definition for $(^^) and $(<<)
^^=@$(LINKERFILE)
<<=$(subst /,\,$<)

# How to bind resources to a DLL or executable
DO.BIND.RES = $(RC) $(RCFLAGS) $(subst /,\,$(filter %.res,$^)) $@
ifeq ($(USE_SHARED_PLUGINS),no)
DO.LINK.CONSOLE.EXE+=$(CR)$(DO.BIND.RES)
DO.LINK.EXE+=$(CR)$(DO.BIND.RES)
endif

endif # ifeq ($(MAKESECTION),postdefines)

#--------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

SYSHELP += \
  $(NEWLINE)echo $"  make os2wcc       Prepare for building under and for $(DESCRIPTION.os2wcc)$"

endif # ifeq ($(MAKESECTION),confighelp)

#---------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=cmd /c bin\\os2conf.cmd SHELL $(SHELL)>>config.tmp

endif # ifeq ($(ROOTCONFIG),config)
