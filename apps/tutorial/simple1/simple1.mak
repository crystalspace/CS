# Application description
DESCRIPTION.simple1 = Crystal Space tutorial part one

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Application-specific help commands
APPHELP += \
  $(NEWLINE)echo $"  make simple1      Make the $(DESCRIPTION.simple1)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simple1 simple1clean

all apps: simple1
simple1:
	$(MAKE_APP)
simple1clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

SIMPLE1.EXE = simple1$(EXE)
DIR.SIMPLE1 = apps/tutorial/simple1
OUT.SIMPLE1 = $(OUT)/$(DIR.SIMPLE1)
INC.SIMPLE1 = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLE1)/*.h ))
SRC.SIMPLE1 = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SIMPLE1)/*.cpp ))
OBJ.SIMPLE1 = $(addprefix $(OUT.SIMPLE1)/,$(notdir $(SRC.SIMPLE1:.cpp=$O)))
DEP.SIMPLE1 = CSTOOL CSGFX CSUTIL CSGEOM CSUTIL
LIB.SIMPLE1 = $(foreach d,$(DEP.SIMPLE1),$($d.LIB))

OUTDIRS += $(OUT.SIMPLE1)

#TO_INSTALL.EXE += $(SIMPLE1.EXE)

MSVC.DSP += SIMPLE1
DSP.SIMPLE1.NAME = simple1
DSP.SIMPLE1.TYPE = appgui

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: build.simple1 simple1clean simple1cleandep

build.simple1: $(OUTDIRS) $(SIMPLE1.EXE)
clean: simple1clean

$(OUT.SIMPLE1)/%$O: $(SRCDIR)/$(DIR.SIMPLE1)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLE1.EXE): $(DEP.EXE) $(OBJ.SIMPLE1) $(LIB.SIMPLE1)
	$(DO.LINK.EXE)

simple1clean:
	-$(RM) simple1.txt
	-$(RMDIR) $(SIMPLE1.EXE) $(OBJ.SIMPLE1)

cleandep: simple1cleandep
simple1cleandep:
	-$(RM) $(OUT.SIMPLE1)/simple1.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLE1) $(OUT.SIMPLE1)/simple1.dep
$(OUT.SIMPLE1)/simple1.dep: $(SRC.SIMPLE1)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLE1)/simple1.dep
endif

endif # ifeq ($(MAKESECTION),targets)
