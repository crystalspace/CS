#==============================================================================
# This is a MacOS X 2D graphics driver makefile that uses CoreGraphics to
# do most of it's work
# Copyright (C) 2001 by Matt Reda <mreda@mac.com>
#==============================================================================

DESCRIPTION.cgdriver2d = Crystal Space $(NEXT.DESCRIPTION) CoreGraphics 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make cgdriver2d   Make the $(DESCRIPTION.cgdriver2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cgdriver2d cgdriver2dclean
all softcanvas plugins drivers drivers2d: cgdriver2d

cgdriver2d:
	$(MAKE_TARGET) MAKE_DLL=yes DO_CGDRIVER2D=yes
cgdriver2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

NEXT.SOURCE_CG2D_PATHS = plugins/video/canvas/macosx/coregraphics plugins/video/canvas/macosx/common
NEXT.HEADER_CG2D_PATHS = $(addprefix $(CFLAGS.I),$(NEXT.SOURCE_CG2D_PATHS))

# Only add header search paths if actually building this plug-in or if
# USE_PLUGINS=no, in which case this module might be built as the dependency
# of some other module (rather than being built explicitly by the `cgdriver2d'
# target).
ifeq ($(USE_PLUGINS),no)
  CFLAGS.INCLUDE += $(NEXT.HEADER_CG2D_PATHS)
else
ifeq ($(DO_CGDRIVER2D),yes)
  CFLAGS.INCLUDE += $(NEXT.HEADER_CG2D_PATHS)
endif
endif

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(NEXT.SOURCE_CG2D_PATHS)
vpath %.m   $(NEXT.SOURCE_CG2D_PATHS)

ifeq ($(USE_PLUGINS),yes)
  CGDRIVER2D = $(OUTDLL)cgdriver2d$(DLL)
  LIB.CGDRIVER2D = $(foreach d,$(DEP.CGDRIVER2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CGDRIVER2D)
else
  CGDRIVER2D = $(OUT)$(LIB_PREFIX)cgdriver2d$(LIB)
  DEP.EXE += $(CGDRIVER2D)
  SCF.STATIC += cgdriver2d
  TO_INSTALL.STATIC_LIBS += $(CGDRIVER2D)
endif

INC.CGDRIVER2D = $(wildcard $(INC.COMMON.DRV2D) \
  $(addsuffix /*.h,$(NEXT.SOURCE_CG2D_PATHS)))
SRC.CGDRIVER2D = $(wildcard $(SRC.COMMON.DRV2D) \
  $(addsuffix /*.cpp,$(NEXT.SOURCE_CG2D_PATHS)) \
  $(addsuffix /*.m,$(NEXT.SOURCE_CG2D_PATHS)))
OBJ.CGDRIVER2D = $(addprefix $(OUT), \
  $(notdir $(subst .cpp,$O,$(SRC.CGDRIVER2D:.m=$O))))
DEP.CGDRIVER2D = CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cgdriver2d cgdriver2dclean

cgdriver2d: $(OUTDIRS) $(CGDRIVER2D)

$(CGDRIVER2D): $(OBJ.CGDRIVER2D) $(LIB.CGDRIVER2D)
	$(DO.PLUGIN)

clean: cgdriver2dclean
cgdriver2dclean:
	$(RM) $(CGDRIVER2D) $(OBJ.CGDRIVER2D)

ifdef DO_DEPEND
dep: $(OUTOS)cgdriver2d.dep
$(OUTOS)cgdriver2d.dep: $(SRC.CGDRIVER2D)
	$(DO.DEP1) $(NEXT.HEADER_CG2D_PATHS) $(DO.DEP2)
else
-include $(OUTOS)cgdriver2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
