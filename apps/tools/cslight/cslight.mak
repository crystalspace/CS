# Application description
DESCRIPTION.cslight = Crystal Space Lighting Calculator

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make cslight      Make the $(DESCRIPTION.cslight)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cslight cslightclean

all apps: cslight
cslight:
	$(MAKE_APP)
cslightclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSLIGHT.EXE = cslight$(EXE)
DIR.CSLIGHT = apps/tools/cslight
OUT.CSLIGHT = $(OUT)/$(DIR.CSLIGHT)
INC.CSLIGHT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSLIGHT)/*.h ))
SRC.CSLIGHT = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSLIGHT)/*.cpp ))
OBJ.CSLIGHT = $(addprefix $(OUT.CSLIGHT)/,$(notdir $(SRC.CSLIGHT:.cpp=$O)))
DEP.CSLIGHT = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.CSLIGHT = $(foreach d,$(DEP.CSLIGHT),$($d.LIB))

OUTDIRS += $(OUT.CSLIGHT)

TO_INSTALL.EXE += $(CSLIGHT.EXE)

MSVC.DSP += CSLIGHT
DSP.CSLIGHT.NAME = cslight
DSP.CSLIGHT.TYPE = appgui

$(CSLIGHT.EXE).WINRSRC = $(SRCDIR)/apps/tools/cslight/cslight.rc

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.cslight cslightclean cslightcleandep

all: $(CSLIGHT.EXE)
build.cslight: $(OUTDIRS) $(CSLIGHT.EXE)
clean: cslightclean

$(OUT.CSLIGHT)/%$O: $(SRCDIR)/$(DIR.CSLIGHT)/%.cpp
	$(DO.COMPILE.CPP)

$(CSLIGHT.EXE): $(DEP.EXE) $(OBJ.CSLIGHT) $(LIB.CSLIGHT)
	$(DO.LINK.EXE)

cslightclean:
	-$(RM) cslight.txt
	-$(RMDIR) $(CSLIGHT.EXE) $(OBJ.CSLIGHT)

cleandep: cslightcleandep
cslightcleandep:
	-$(RM) $(OUT.CSLIGHT)/cslight.dep

ifdef DO_DEPEND
dep: $(OUT.CSLIGHT) $(OUT.CSLIGHT)/cslight.dep
$(OUT.CSLIGHT)/cslight.dep: $(SRC.CSLIGHT)
	$(DO.DEPEND)
else
-include $(OUT.CSLIGHT)/cslight.dep
endif

endif # ifeq ($(MAKESECTION),targets)
