# Application description
DESCRIPTION.demsky = Crystal Space sky demo

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += $(NEWLINE)echo $"  make demsky       Make the $(DESCRIPTION.demsky)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: demsky demskyclean

all apps: demsky
demsky:
	$(MAKE_TARGET)
demskyclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp apps/demosky apps/support

DEMOSKY.EXE=demosky$(EXE)
INC.DEMOSKY = $(wildcard apps/demosky/*.h)
SRC.DEMOSKY = $(wildcard apps/demosky/*.cpp)
OBJ.DEMOSKY = $(addprefix $(OUT),$(notdir $(SRC.DEMOSKY:.cpp=$O)))
DEP.DEMOSKY = \
  CSPARSER CSFX CSENGINE CSTERR CSFX CSGFXLDR CSUTIL CSSYS CSGEOM CSOBJECT CSUTIL
LIB.DEMOSKY = $(foreach d,$(DEP.DEMOSKY),$($d.LIB))

#TO_INSTALL.EXE += $(DEMOSKY.EXE)

MSVC.DSP += DEMOSKY
DSP.DEMOSKY.NAME = demosky
DSP.DEMOSKY.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: demsky demskyclean

all: $(DEMOSKY.EXE)
demsky: $(OUTDIRS) $(DEMOSKY.EXE)
clean: demskyclean

$(DEMOSKY.EXE): $(DEP.EXE) $(OBJ.DEMOSKY) $(LIB.DEMOSKY)
	$(DO.LINK.EXE)

demskyclean:
	-$(RM) $(DEMOSKY.EXE) $(OBJ.DEMOSKY)

ifdef DO_DEPEND
dep: $(OUTOS)demosky.dep
$(OUTOS)demosky.dep: $(SRC.DEMOSKY)
	$(DO.DEP)
else
-include $(OUTOS)demosky.dep
endif

endif # ifeq ($(MAKESECTION),targets)
