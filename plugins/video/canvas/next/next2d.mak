#==============================================================================
# This is the default 2D graphics driver makefile for MacOS/X, MacOS/X Server,
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
	$(MAKE_TARGET) MAKE_DLL=yes
next2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_2D_PATHS = \
  $(addprefix plugins/video/canvas/next/,$(NEXT.SEARCH_PATH))
CFLAGS.NEXT2D = $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_2D_PATHS))

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

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

$(OUT)%$O: plugins/video/canvas/next/shared/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/shared/%.m
	$(DO.COMPILE.M) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/nextstep/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/nextstep/%.m
	$(DO.COMPILE.M) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/openstep/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/openstep/%.m
	$(DO.COMPILE.M) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/macosxs/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/macosxs/%.m
	$(DO.COMPILE.M) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/macosx/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.NEXT2D)
 
$(OUT)%$O: plugins/video/canvas/next/macosx/%.m
	$(DO.COMPILE.M) $(CFLAGS.NEXT2D)
 
$(NEXT2D): $(OBJ.NEXT2D) $(LIB.NEXT2D)
	$(DO.PLUGIN)

clean: next2dclean
next2dclean:
	$(RM) $(NEXT2D) $(OBJ.NEXT2D)

ifdef DO_DEPEND
dep: $(OUTOS)next2d.dep
$(OUTOS)next2d.dep: $(SRC.NEXT2D)
	$(DO.DEP1) $(CFLAGS.NEXT2D) $(DO.DEP2)
else
-include $(OUTOS)next2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
