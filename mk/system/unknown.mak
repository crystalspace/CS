# This is an include file for all the makefiles which describes system specific
# settings. This file is the default for an unknown configuration.

#---------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

PROC=invalid
OS=invalid
COMP=invalid
NATIVE_COM=invalid

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#------------------------------------------------------------------ defines ---#
ifeq ($(MAKESECTION),defines)

EXE=invalid
DLL=invalid
LIB=invalid
LIB_PREFIX=invalid
LIBS.EXE=invalid
Z_LIBS=invalid
PNG_LIBS=invalid
JPG_LIBS=invalid
SOUND_LIBS=invalid
CFLAGS.INCLUDE=invalid
CFLAGS.GENERAL=invalid
CFLAGS.optimize=invalid
CFLAGS.debug=invalid
CFLAGS.profile=invalid
CFLAGS.DLL=invalid
LFLAGS.GENERAL=invalid
LFLAGS.debug=invalid
LFLAGS.profile=invalid
LFLAGS.DLL=invalid
SRC.SYS_EXE=invalid
OUTDLL=invalid
CC=invalid
CXX=invalid
LINK=invalid
AR=invalid
ARFLAGS=invalid
MKDIR=invalid
RM=invalid
RMDIR=invalid
SYS_SED_DEPEND=invalid

endif # ifeq ($(MAKESECTION),defines)

#---------------------------------------------------------------- configure ---#
ifeq ($(MAKESECTION),configure)

configure:

endif # ifeq ($(MAKESECTION),configure)
