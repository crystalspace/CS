# This is a subinclude file used to define the rules needed
# to build the SVGALIB 2D driver -- svgalib2d

# Driver description
DESCRIPTION.svgalib2d = Crystal Space SVGAlib 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make svgalib2d    Make the $(DESCRIPTION.svgalib2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: svgalib2d svgalib2dclean
all plugins drivers drivers2d: svgalib2d

svgalib2d:
	$(MAKE_TARGET) MAKE_DLL=yes
svgalib2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LIB.SVGA2D.SYSTEM += -lvga -lvgagl

ifeq ($(USE_PLUGINS),yes)
  SVGA2D = $(OUTDLL)svga2d$(DLL)
  LIB.SVGA2D = $(foreach d,$(DEP.SVGA2D),$($d.LIB))
  LIB.SVGA2D.SPECIAL += $(LIB.SVGA2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(SVGA2D)
else
  SVGA2D = $(OUT)$(LIB_PREFIX)svga2d$(LIB)
  DEP.EXE += $(SVGA2D)
  LIBS.EXE += $(LIB.SVGA2D.SYSTEM)
  SCF.STATIC += svga2d
  TO_INSTALL.STATIC_LIBS += $(SVGA2D)
endif

INC.SVGA2D = $(wildcard plugins/video/canvas/svgalib/*.h   $(INC.COMMON.DRV2D))
SRC.SVGA2D = $(wildcard plugins/video/canvas/svgalib/*.cpp $(SRC.COMMON.DRV2D))
OBJ.SVGA2D = $(addprefix $(OUT),$(notdir $(SRC.SVGA2D:.cpp=$O)))
DEP.SVGA2D = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: svgalib2d svgalib2dclean

svgalib2d: $(OUTDIRS) $(SVGA2D)

$(OUT)%$O: plugins/video/canvas/svgalib/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SVGA2D)
 
$(SVGA2D): $(OBJ.SVGA2D) $(LIB.SVGA2D)
	$(DO.PLUGIN) $(LIB.SVGA2D.SPECIAL)

clean: svgalib2dclean
svgalib2dclean:
	$(RM) $(SVGA2D) $(OBJ.SVGA2D)

ifdef DO_DEPEND
dep: $(OUTOS)svgalib2d.dep
$(OUTOS)svgalib2d.dep: $(SRC.SVGA2D)
	$(DO.DEP)
else
-include $(OUTOS)svgalib2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
