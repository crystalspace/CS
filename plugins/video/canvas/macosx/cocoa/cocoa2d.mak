#==============================================================================
# This is the makefile for a pure-Cocoa 2D graphics driver for MacOS/X.  This
# driver is extremely high-level and avoids specialized technologies such as
# CoreGraphics and OpenGL.  Consequently, it is much more portable, though
# necessarily slower.
#
# Copyright (C)1998-2002 by Eric Sunshine <sunshine@sunshineco.com>
#==============================================================================

DESCRIPTION.cocoa2d = Crystal Space MacOS/X Cocoa 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make cocoa2d      Make the $(DESCRIPTION.cocoa2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cocoa2d cocoa2dclean
all softcanvas plugins drivers drivers2d: cocoa2d

cocoa2d:
	$(MAKE_TARGET) MAKE_DLL=yes
cocoa2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  COCOA2D = $(OUTDLL)/cocoa2d$(DLL)
  LIB.COCOA2D = $(foreach d,$(DEP.COCOA2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(COCOA2D)
else
  COCOA2D = $(OUT)/$(LIB_PREFIX)cocoa2d$(LIB)
  DEP.EXE += $(COCOA2D)
  SCF.STATIC += cocoa2d
  TO_INSTALL.STATIC_LIBS += $(COCOA2D)
endif

DIR.COCOA2D = plugins/video/canvas/macosx/cocoa
OUT.COCOA2D = $(OUT)/$(DIR.COCOA2D)
INF.COCOA2D = $(SRCDIR)/$(DIR.COCOA2D)/cocoa2d.csplugin
INC.COCOA2D = $(wildcard $(addprefix $(SRCDIR)/, \
  $(DIR.COCOA2D)/*.h $(INC.COMMON.DRV2D)))
SRC.COCOA2D = $(wildcard $(addprefix $(SRCDIR)/, \
  $(foreach s,*.cpp *.m *.mm,$(DIR.COCOA2D)/$s) $(SRC.COMMON.DRV2D)))
OBJ.COCOA2D = $(addprefix $(OUT.COCOA2D)/, \
  $(notdir $(subst .cpp,$O,$(subst .mm,$O,$(SRC.COCOA2D:.m=$O)))))
DEP.COCOA2D = CSUTIL

OUTDIRS += $(OUT.COCOA2D)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cocoa2d cocoa2dclean cocoa2dcleandep

cocoa2d: $(OUTDIRS) $(COCOA2D)

$(OUT.COCOA2D)/%$O: $(SRCDIR)/$(DIR.COCOA2D)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.COCOA2D)/%$O: $(SRCDIR)/$(DIR.COCOA2D)/%.mm
	$(DO.COMPILE.MM)

$(OUT.COCOA2D)/%$O: $(SRCDIR)/$(DIR.COCOA2D)/%.m
	$(DO.COMPILE.M)

$(OUT.COCOA2D)/%$O: $(SRCDIR)/plugins/video/canvas/common/%.cpp
	$(DO.COMPILE.CPP)

$(COCOA2D): $(OBJ.COCOA2D) $(LIB.COCOA2D)
	$(DO.PLUGIN)

clean: cocoa2dclean
cocoa2dclean:
	-$(RMDIR) $(COCOA2D) $(OBJ.COCOA2D) $(OUTDLL)/$(notdir $(INF.COCOA2D))

cleandep: cocoa2dcleandep
cocoa2dcleandep:
	-$(RM) $(OUT.COCOA2D)/cocoa2d.dep

ifdef DO_DEPEND
dep: $(OUT.COCOA2D) $(OUT.COCOA2D)/cocoa2d.dep
$(OUT.COCOA2D)/cocoa2d.dep: $(SRC.COCOA2D)
	$(DO.DEPEND)
else
-include $(OUT.COCOA2D)/cocoa2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
