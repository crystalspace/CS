# This is a subinclude file used to define the rules needed
# to build the SVGALIB 2D driver -- svgalib2d

# Driver description
DESCRIPTION.svgalib2d = Crystal Space SVGALib 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make svgalib2d    Make the $(DESCRIPTION.svgalib2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: svgalib2d

ifeq ($(USE_DLL),yes)
all drivers drivers2d: svgalib2d
endif

svgalib2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Link with SVGALib librarys
LIBS._SVGA2D+=-lvga -lvgagl

# The 2D SVGAlib driver
ifeq ($(USE_DLL),yes)
  SVGA2D=$(OUTDLL)svga2d$(DLL)
  LIBS.SVGA2D+=$(LIBS._SVGA2D)
  DEP.SVGA2D=$(CSCOM.LIB) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  SVGA2D=$(OUT)$(LIB_PREFIX)svga2d$(LIB)
  DEP.EXE+=$(SVGA2D)
  LIBS.EXE+=$(LIBS._SVGA2D)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SVGA2D
endif
DESCRIPTION.$(SVGA2D) = $(DESCRIPTION.svgalib2d)
SRC.SVGA2D = $(wildcard libs/cs2d/svgalib/*.cpp $(SRC.COMMON.DRV2D))
OBJ.SVGA2D = $(addprefix $(OUT),$(notdir $(SRC.SVGA2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: svgalib2d svgalib2dclean svgalib2dcleanlib

# Chain rules
clean: svgalib2dclean
cleanlib: svgalib2dcleanlib

svgalib2d: $(OUTDIRS) $(SVGA2D)

$(OUT)%$O: libs/cs2d/svgalib/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SVGA2D)
 
$(SVGA2D): $(OBJ.SVGA2D) $(DEP.SVGA2D)
	$(DO.LIBRARY) $(LIBS.SVGA2D)

svgalib2dclean:
	$(RM) $(SVGA2D)

svgalib2dcleanlib:
	$(RM) $(OBJ.SVGA2D) $(SVGA2D)

ifdef DO_DEPEND
$(OUTOS)svgalib2d.dep: $(SRC.SVGA2D)
	$(DO.DEP)
endif

-include $(OUTOS)svgalib2d.dep

endif # ifeq ($(MAKESECTION),targets)
