# Application description
DESCRIPTION.simple2 = Crystal Space tutorial part two, sprite

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simple2      Make the $(DESCRIPTION.simple2)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simple2 simple2clean

all apps: simple2
simple2:
	$(MAKE_APP)
simple2clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPLE2.EXE = simple2$(EXE)
DIR.SIMPLE2 = apps/tutorial/simple2
OUT.SIMPLE2 = $(OUT)/$(DIR.SIMPLE2)
INC.SIMPLE2 = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLE2)/*.h ))
SRC.SIMPLE2 = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLE2)/*.cpp ))
OBJ.SIMPLE2 = $(addprefix $(OUT.SIMPLE2)/,$(notdir $(SRC.SIMPLE2:.cpp=$O)))
DEP.SIMPLE2 = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPLE2 = $(foreach d,$(DEP.SIMPLE2),$($d.LIB))

OUTDIRS += $(OUT.SIMPLE2)

#TO_INSTALL.EXE += $(SIMPLE2.EXE)

MSVC.DSP += SIMPLE2
DSP.SIMPLE2.NAME = simple2
DSP.SIMPLE2.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simple2 simple2clean simple2cleandep

build.simple2: $(OUTDIRS) $(SIMPLE2.EXE)
clean: simple2clean

$(OUT.SIMPLE2)/%$O: $(SRCDIR)/$(DIR.SIMPLE2)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLE2.EXE): $(DEP.EXE) $(OBJ.SIMPLE2) $(LIB.SIMPLE2)
	$(DO.LINK.EXE)

simple2clean:
	-$(RM) simple2.txt
	-$(RMDIR) $(SIMPLE2.EXE) $(OBJ.SIMPLE2)

cleandep: simple2cleandep
simple2cleandep:
	-$(RM) $(OUT.SIMPLE2)/simple2.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLE2) $(OUT.SIMPLE2)/simple2.dep
$(OUT.SIMPLE2)/simple2.dep: $(SRC.SIMPLE2)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLE2)/simple2.dep
endif

endif # ifeq ($(MAKESECTION),targets)
