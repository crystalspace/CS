#==============================================================================
# This is the default 2D graphics driver makefile for MacOS/X Server,
# OpenStep, and NextStep.
# Copyright (C)1998,1999,2000 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

DESCRIPTION.next2d = Crystal Space Apple/NeXT 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make next2d       Make the $(DESCRIPTION.next2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: next2d next2dclean
all plugins drivers drivers2d: next2d

next2d:
	$(MAKE_TARGET) MAKE_DLL=yes
next2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_2D_PATHS = \
  $(addprefix plugins/video/canvas/next/,$(NEXT.SEARCH_PATH))
CFLAGS.INCLUDE += $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_2D_PATHS))

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(NEXT.SOURCE_2D_PATHS)

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

INC.NEXT2D = $(wildcard $(addsuffix /*.h,$(NEXT.SOURCE_2D_PATHS)) \
  $(INC.COMMON.DRV2D))
SRC.NEXT2D = $(wildcard $(addsuffix /*.cpp,$(NEXT.SOURCE_2D_PATHS)) \
  $(SRC.COMMON.DRV2D))
OBJ.NEXT2D = $(addprefix $(OUT),$(notdir $(SRC.NEXT2D:.cpp=$O)))
DEP.NEXT2D =

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: next2d next2dclean

next2d: $(OUTDIRS) $(NEXT2D)

$(NEXT2D): $(OBJ.NEXT2D) $(LIB.NEXT2D)
	$(DO.PLUGIN)

clean: next2dclean
next2dclean:
	$(RM) $(NEXT2D) $(OBJ.NEXT2D) $(OUTOS)next2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)next2d.dep
$(OUTOS)next2d.dep: $(SRC.NEXT2D)
	$(DO.DEP)
else
-include $(OUTOS)next2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
