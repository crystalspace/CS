#==============================================================================
# This is the default 2D graphics driver makefile for MacOS/X Server,
# OpenStep, and NextStep.  It is based upon the AppKit/Cocoa API.
# Copyright (C)1998-2001 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

DESCRIPTION.next2d = Crystal Space $(NEXT.DESCRIPTION) 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make next2d       Make the $(DESCRIPTION.next2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: next2d next2dclean
all softcanvas plugins drivers drivers2d: next2d

next2d:
	$(MAKE_TARGET) MAKE_DLL=yes DO_NEXT2D=yes
next2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_2D_PATHS = \
  $(addprefix plugins/video/canvas/next/,$(NEXT.SEARCH_PATH))
NEXT.HEADER_2D_PATHS = $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_2D_PATHS))

# Only add header search paths if actually building this plug-in or if
# USE_PLUGINS=no, in which case this module might be built as the dependency
# of some other module (rather than being built explicitly by the `next2d'
# target).
ifeq ($(USE_PLUGINS),no)
  CFLAGS.INCLUDE += $(NEXT.HEADER_2D_PATHS)
else
ifeq ($(DO_NEXT2D),yes)
  CFLAGS.INCLUDE += $(NEXT.HEADER_2D_PATHS)
endif
endif

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(NEXT.SOURCE_2D_PATHS)
vpath %.m   $(NEXT.SOURCE_2D_PATHS)

ifeq ($(USE_PLUGINS),yes)
  NEXT2D = $(OUTDLL)next2d$(DLL)
  LIB.NEXT2D = $(foreach d,$(DEP.NEXT2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NEXT2D)
else
  NEXT2D = $(OUT)$(LIB_PREFIX)next2d$(LIB)
  DEP.EXE += $(NEXT2D)
  SCF.STATIC += next2d
  TO_INSTALL.STATIC_LIBS += $(NEXT2D)
endif

INC.NEXT2D = $(wildcard $(INC.COMMON.DRV2D) \
  $(addsuffix /*.h,$(NEXT.SOURCE_2D_PATHS)))
SRC.NEXT2D = $(wildcard $(SRC.COMMON.DRV2D) \
  $(addsuffix /*.cpp,$(NEXT.SOURCE_2D_PATHS)) \
  $(addsuffix /*.m,$(NEXT.SOURCE_2D_PATHS)))
OBJ.NEXT2D = $(addprefix $(OUT), \
  $(notdir $(subst .cpp,$O,$(SRC.NEXT2D:.m=$O))))
DEP.NEXT2D = CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: next2d next2dclean

next2d: $(OUTDIRS) $(NEXT2D)

$(NEXT2D): $(OBJ.NEXT2D) $(LIB.NEXT2D)
	$(DO.PLUGIN)

clean: next2dclean
next2dclean:
	$(RM) $(NEXT2D) $(OBJ.NEXT2D)

ifdef DO_DEPEND
dep: $(OUTOS)next2d.dep
$(OUTOS)next2d.dep: $(SRC.NEXT2D)
	$(DO.DEP1) $(NEXT.HEADER_2D_PATHS) $(DO.DEP2)
else
-include $(OUTOS)next2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
