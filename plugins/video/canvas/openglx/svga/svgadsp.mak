# This is a subinclude file used to define the rules needed
# to build the SVGA Displaydriver for GLX 2D driver -- oglsvga

# Driver description
DESCRIPTION.oglsvga = Crystal Space SVGA GL/X 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make oglsvga     Make the $(DESCRIPTION.oglsvga)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: oglsvga oglsvgaclean
all plugins glxdisp: oglsvga

oglsvga:
	$(MAKE_TARGET) MAKE_DLL=yes

oglsvgaclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/openglx/svga

ifeq ($(USE_PLUGINS),yes)
  OGLSVGA = $(OUTDLL)oglsvga$(DLL)
  LIB.OGLSVGA = $(foreach d,$(DEP.OGLSVGA),$($d.LIB))
else
  OGLSVGA = $(OUT)$(LIB_PREFIX)oglsvga$(LIB)
  DEP.EXE += $(OGLSVGA)
  LIBS.EXE += $(CSUTIL.LIB) $(CSSYS.LIB)
  SCF.STATIC += oglsvga
endif

INC.OGLSVGA = $(wildcard plugins/video/canvas/openglx/svga/*.h)
SRC.OGLSVGA = $(wildcard plugins/video/canvas/openglx/svga/*.cpp)
OBJ.OGLSVGA = $(addprefix $(OUT),$(notdir $(SRC.OGLSVGA:.cpp=$O)))
DEP.OGLSVGA = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: oglsvga oglsvgaclean

oglsvga: $(OUTDIRS) $(OGLSVGA)

$(OGLSVGA): $(OBJ.OGLSVGA) $(LIB.OGLSVGA)
	$(DO.PLUGIN) $(LIBS.OGLSVGA)

clean: oglsvgaclean
oglsvgaclean:
	$(RM) $(OGLSVGA) $(OBJ.OGLSVGA) $(OUTOS)oglsvga.dep
 
ifdef DO_DEPEND
dep: $(OUTOS)oglsvga.dep
$(OUTOS)oglsvga.dep: $(SRC.OGLSVGA)
	$(DO.DEP)
else
-include $(OUTOS)oglsvga.dep
endif

endif # ifeq ($(MAKESECTION),targets)
