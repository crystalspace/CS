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
	$(MAKE_TARGET) MAKE_DLL=yes DO_COCOA2D=yes
cocoa2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

MACOSX.SOURCE_COCOA2D_PATHS = plugins/video/canvas/macosx/cocoa
MACOSX.HEADER_COCOA2D_PATHS = \
  $(addprefix $(CFLAGS.I),$(MACOSX.SOURCE_COCOA2D_PATHS))

# Only add header search paths if actually building this plug-in or if
# USE_PLUGINS=no, in which case this module might be built as the dependency
# of some other module (rather than being built explicitly by the `cocoa2d'
# target).
ifeq ($(USE_PLUGINS),no)
  CFLAGS.INCLUDE += $(MACOSX.HEADER_COCOA2D_PATHS)
else
ifeq ($(DO_COCOA2D),yes)
  CFLAGS.INCLUDE += $(MACOSX.HEADER_COCOA2D_PATHS)
endif
endif

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(MACOSX.SOURCE_COCOA2D_PATHS)
vpath %.m   $(MACOSX.SOURCE_COCOA2D_PATHS)

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

INC.COCOA2D = $(wildcard $(INC.COMMON.DRV2D) \
  $(addsuffix /*.h,$(MACOSX.SOURCE_COCOA2D_PATHS)))
SRC.COCOA2D = $(wildcard $(SRC.COMMON.DRV2D) \
  $(addsuffix /*.cpp,$(MACOSX.SOURCE_COCOA2D_PATHS)) \
  $(addsuffix /*.m,$(MACOSX.SOURCE_COCOA2D_PATHS)))
OBJ.COCOA2D = $(addprefix $(OUT)/, \
  $(notdir $(subst .cpp,$O,$(SRC.COCOA2D:.m=$O))))
DEP.COCOA2D = CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cocoa2d cocoa2dclean

cocoa2d: $(OUTDIRS) $(COCOA2D)

$(COCOA2D): $(OBJ.COCOA2D) $(LIB.COCOA2D)
	$(DO.PLUGIN)

clean: cocoa2dclean
cocoa2dclean:
	$(RM) $(COCOA2D) $(OBJ.COCOA2D)

ifdef DO_DEPEND
dep: $(OUTOS)/cocoa2d.dep
$(OUTOS)/cocoa2d.dep: $(SRC.COCOA2D)
	$(DO.DEP1) $(MACOSX.HEADER_COCOA2D_PATHS) $(DO.DEP2)
else
-include $(OUTOS)/cocoa2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
