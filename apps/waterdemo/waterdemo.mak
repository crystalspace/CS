# Application description
DESCRIPTION.r3dtest = Displaying rendering of a watersurface in new renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make waterdemo      Make the $(DESCRIPTION.waterdemo)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: waterdemo waterdemoclean

all apps: waterdemo
waterdemo:
	$(MAKE_APP)
waterdemoclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

WATERDEMO.EXE = waterdemo$(EXE)
DIR.WATERDEMO = apps/tests/waterdemo
OUT.WATERDEMO = $(OUT)/$(DIR.WATERDEMO)
INC.WATERDEMO = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.WATERDEMO)/*.h ))
SRC.WATERDEMO = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.WATERDEMO)/*.cpp ))
OBJ.WATERDEMO = $(addprefix $(OUT.WATERDEMO)/,$(notdir $(SRC.WATERDEMO:.cpp=$O)))
DEP.WATERDEMO = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.WATERDEMO = $(foreach d,$(DEP.WATERDEMO),$($d.LIB))

OUTDIRS += $(OUT.WATERDEMO)

#TO_INSTALL.EXE += $(WATERDEMO.EXE)

MSVC.DSP += WATERDEMO
DSP.WATERDEMO.NAME = waterdemo
DSP.WATERDEMO.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.waterdemo waterdemoclean waterdemocleandep

build.waterdemo: $(OUTDIRS) $(WATERDEMO.EXE)
clean: waterdemoclean

$(OUT.WATERDEMO)/%$O: $(SRCDIR)/$(DIR.WATERDEMO)/%.cpp
	$(DO.COMPILE.CPP)

$(WATERDEMO.EXE): $(DEP.EXE) $(OBJ.WATERDEMO) $(LIB.WATERDEMO)
	$(DO.LINK.EXE)

waterdemoclean:
	-$(RM) waterdemo.txt
	-$(RMDIR) $(WATERDEMO.EXE) $(OBJ.WATERDEMO)

cleandep: waterdemocleandep
waterdemocleandep:
	-$(RM) $(OUT.WATERDEMO)/waterdemo.dep

ifdef DO_DEPEND
dep: $(OUT.WATERDEMO) $(OUT.WATERDEMO)/waterdemo.dep
$(OUT.WATERDEMO)/waterdemo.dep: $(SRC.WATERDEMO)
	$(DO.DEPEND)
else
-include $(OUT.WATERDEMO)/waterdemo.dep
endif

endif # ifeq ($(MAKESECTION),targets)
