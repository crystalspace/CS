# Application description
DESCRIPTION.tbconvert = Crystal Space Big Terrain Conversion Tool

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make tbconvert    Make the $(DESCRIPTION.tbconvert)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: tbconvert tbconvertclean

all apps: tbconvert
tbconvert:
	$(MAKE_TARGET)
tbconvertclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/tools/tbconvert

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

.PHONY: build.tbconvert tbconvertclean

all: $(TBCONVERT.EXE)
build.tbconvert: $(OUTDIRS) $(TBCONVERT.EXE)
clean: tbconvertclean

$(TBCONVERT.EXE): $(DEP.EXE) $(OBJ.TBCONVERT) $(LIB.TBCONVERT)
	$(DO.LINK.EXE)

tbconvertclean:
	-$(RM) $(TBCONVERT.EXE) $(OBJ.TBCONVERT)

ifdef DO_DEPEND
dep: $(OUTOS)tbconvert.dep
$(OUTOS)tbconvert.dep: $(SRC.TBCONVERT)
	$(DO.DEP)
else
-include $(OUTOS)tbconvert.dep
endif

endif # ifeq ($(MAKESECTION),targets)
