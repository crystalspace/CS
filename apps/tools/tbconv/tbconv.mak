# Application description
DESCRIPTION.tbconv = Crystal Space Big Terrain Conversion Tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make tbconv       Make the $(DESCRIPTION.tbconv)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tbconv tbconvclean

all apps: tbconv
tbconv:
	$(MAKE_TARGET)
tbconvclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/tbconv

TBCONVERT.EXE = tbconvert$(EXE)
INC.TBCONVERT = $(wildcard apps/tools/tbconv/*.h)
SRC.TBCONVERT = $(wildcard apps/tools/tbconv/*.cpp)
OBJ.TBCONVERT = $(addprefix $(OUT)/,$(notdir $(SRC.TBCONVERT:.cpp=$O)))
DEP.TBCONVERT = CSTOOL CSGEOM CSTOOL CSGFX CSSYS CSUTIL CSSYS
LIB.TBCONVERT = $(foreach d,$(DEP.TBCONVERT),$($d.LIB))

TO_INSTALL.EXE    += $(TBCONVERT.EXE)

MSVC.DSP += TBCONVERT
DSP.TBCONVERT.NAME = tbconvert
DSP.TBCONVERT.TYPE = appcon
#DSP.TBCONVERT.RESOURCES = libs/cssys/win32/rsrc/cs1.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: tbconv tbconvclean

all: $(TBCONVERT.EXE)
tbconv: $(OUTDIRS) $(TBCONVERT.EXE)
clean: tbconvclean

$(TBCONVERT.EXE): $(DEP.EXE) $(OBJ.TBCONVERT) $(LIB.TBCONVERT)
	$(DO.LINK.EXE)

tbconvclean:
	-$(RM) $(TBCONVERT.EXE) $(OBJ.TBCONVERT)

ifdef DO_DEPEND
dep: $(OUTOS)tbconvert.dep
$(OUTOS)tbconvert.dep: $(SRC.TBCONVERT)
	$(DO.DEP)
else
-include $(OUTOS)tbconvert.dep
endif

endif # ifeq ($(MAKESECTION),targets)
